// mem.cpp : 定义控制台应用程序的入口点。
//

#include "../inc/basic.h"
#include "../exception/call_stack.hpp"

__NS_BASIC_START

uint32_t GetHandleID_Default()
{
	return 0;
}
static gethandleid_func g_func = GetHandleID_Default;
static LONG _used_memory = 0;
static LONG _memory_malloc = 0;
static LONG _memory_free = 0;
typedef struct _mem_data
{
	uint32_t handle;
	uint32_t allocated;
	_mem_data()
	{
		handle = 0;
		allocated = 0;
	}
} mem_data;
#define SLOT_SIZE 0x100000
static mem_data mem_stats[SLOT_SIZE];

void BindGetHandleIDFunc(gethandleid_func func)
{
	g_func = func;
}

static uint32_t* get_allocated_field(uint32_t handle)
{
	int h = (int)(handle & (SLOT_SIZE - 1));
	mem_data *data = &mem_stats[h];
	uint32_t old_handle = data->handle;
	uint32_t old_alloc = data->allocated;
	if (old_handle == 0 || old_alloc <= 0)
	{
		BasicInterlockedCompareExchange((LONG*)&data->handle, old_handle, handle);
		if (old_alloc < 0)
		{
			BasicInterlockedCompareExchange((LONG*)&data->allocated, old_alloc, 0);
		}
	}
	if (data->handle != handle)
	{
		return nullptr;
	}
	return &data->allocated;
}

void UpdateMallocState_Alloc(uint32_t handleid, size_t size)
{
	BasicInterlockedExchangeAdd(&_used_memory, size);
	BasicInterlockedIncrement(&_memory_malloc);
	uint32_t* allocated = get_allocated_field(handleid);
	if (allocated)
	{
		BasicInterlockedExchangeAdd((LONG*)allocated, size);
	}
}
void UpdateMallocState_Free(uint32_t handleid, size_t size)
{
	BasicInterlockedExchangeSub(&_used_memory, size);
	BasicInterlockedIncrement(&_memory_free);
	uint32_t* allocated = get_allocated_field(handleid);
	if (allocated)
	{
		BasicInterlockedExchangeSub((LONG*)allocated, size);
	}
}

struct HeadFillFix
{
	union
	{
		uint32_t	m_value;
		struct
		{
			uint32_t m_bBegin : 1;
			uint32_t m_size : 30;
			uint32_t m_bEnd : 1;
		};
	};
	uint32_t	m_handleID;
};
#define MallocFixSize	sizeof(HeadFillFix)

inline void* Fill_prefix(char* ptr, size_t size)
{
	uint32_t handleid = g_func();
	HeadFillFix* pFix = (HeadFillFix*)ptr;
#ifdef _DEBUG
	pFix->m_bBegin = 1;
	pFix->m_size = size;
	pFix->m_bEnd = 1;
#else
	pFix->m_size = size;
#endif
	pFix->m_handleID = handleid;
	ptr += MallocFixSize;
	UpdateMallocState_Alloc(handleid, size);
	return ptr;
}
inline void* Clean_prefix(char* ptr)
{
	ptr -= MallocFixSize;
	HeadFillFix* pFix = (HeadFillFix*)ptr;
#ifdef _DEBUG
	if(pFix->m_bBegin != 1 || pFix->m_bEnd != 1)
	{
		//调用堆栈
		stacktrace::call_stack st;
		BasicLogEventErrorV("Free prefix error");
		BasicLogEventErrorV(st.to_string().c_str());
		exit(0);
	}
#endif
	UpdateMallocState_Free(pFix->m_handleID, pFix->m_size);
	return ptr;
}

#ifdef _USE_SYS_MALLOC
void* BasicAllocate(size_t size)
{
	void* p = malloc(size + MallocFixSize);
	if(p == NULL)
		return NULL;
	return Fill_prefix((char*)p, size);
}

void* BasicReallocate(void* p, size_t size)
{
	if (p == nullptr)
		return BasicAllocate(size);
	p = Clean_prefix((char*)p);
	if (p == nullptr)
		return BasicAllocate(size);

	p = realloc(p, size + MallocFixSize);
	if (p == NULL)
		return NULL;
	return Fill_prefix((char*)p, size);
}

void BasicDeallocate(void* p, size_t size)
{
	if(p == NULL)
		return;
	p = Clean_prefix((char*)p);
	HeadFillFix* pFix = (HeadFillFix*)p;
	if (size != 0 && size != pFix->m_size)
	{
#ifdef _DEBUG
		//调用堆栈
		stacktrace::call_stack st;
		BasicLogEventErrorV("BasicDeallocate size error");
		BasicLogEventErrorV(st.to_string().c_str());
		exit(0);
#else
		BasicLogEventErrorV("BasicDeallocate size error");
#endif
	}
	free(p);
}

#else	//_USE_SYS_MALLOC
extern "C"{
	#include "jemalloc.h"
}
void* BasicAllocate(size_t size)
{	
	void* p = je_malloc(size + MallocFixSize);
	if (p == NULL)
		return NULL;
	return Fill_prefix((char*)p, size);
}

void* BasicReallocate(void* p, size_t size)
{
	if (p == nullptr)
		return BasicAllocate(size);
	p = Clean_prefix((char*)p);
	if (p == nullptr)
		return BasicAllocate(size);

	p = je_realloc(p, size + MallocFixSize);
	if (p == NULL)
		return NULL;
	return Fill_prefix((char*)p, size);
}

void BasicDeallocate(void* p, size_t size)
{
	if(p == NULL)
		return;
	p = Clean_prefix((char*)p);
	HeadFillFix* pFix = (HeadFillFix*)p;
	if (size != 0 && size != pFix->m_size)
	{
#ifdef _DEBUG
		//调用堆栈
		stacktrace::call_stack st;
		BasicLogEventErrorV("BasicDeallocate size error");
		BasicLogEventErrorV(st.to_string().c_str());
		exit(0);
#else
		BasicLogEventErrorV("BasicDeallocate size error");
#endif
	}
	je_free(p);
}

#endif //_USE_SYS_MALLOC

///取内存分配信息
void BasicGetOperationInfo(
			size_t& nAllocateCount, 
			size_t& nDeallocateCount,
			size_t& nUseMemory
						 )
{
	nAllocateCount = _memory_malloc;
	nDeallocateCount = _memory_free;
	nUseMemory = _used_memory;
}

void BasicShowCurrentMemInfo()
{
	CBasicSmartBuffer smBuf;
	smBuf.SetDataLength(10240);
	smBuf.SetDataLength(0);
	char szBuf[128] = { 0 };
	unsigned int total = 0;
	for (int i = 0; i < SLOT_SIZE; i++)
	{
		mem_data* data = &mem_stats[i];
		if (data->handle != 0 && data->allocated != 0) 
		{
			total += data->allocated;
			sprintf(szBuf, "0x % x -> %zdkb", data->handle, data->allocated >> 10);
			smBuf.AppendString(szBuf);
		}
	}
	sprintf(szBuf, "+total: %zdkb", total);
	smBuf.AppendString(szBuf);

	BasicLogEvent(smBuf.GetDataBuffer());
}

long BasicGetHandleIDMemInfo(uint32_t nHandleID)
{
	uint32_t* pSize = get_allocated_field(nHandleID);
	if (pSize)
	{
		return *pSize;
	}
	return -1;
}

char* BasicStrdup(const char* p)
{
	int sz = strlen(p);
	char* ret = (char*)BasicAllocate(sz+1);
	memcpy(ret, p, sz+1);
	return ret;
}

__NS_BASIC_END
