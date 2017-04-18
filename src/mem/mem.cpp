// mem.cpp : 定义控制台应用程序的入口点。
//

#include "../inc/basic.h"
__NS_BASIC_START

//这里不能用自旋锁，会恶化
basiclib::CMutex g_lockCheck;
std::map<void*, stacktrace::call_stack> g_mapCheck;
#ifdef _DEBUG
uint32_t g_nCheckMemMode = MemRunMemCheck_RunSizeCheck | MemRunMemCheck_RunTongJi;
#else
uint32_t g_nCheckMemMode = 0;
#endif
int	m_nCheckMin = 0;
int m_nCheckMax = 0;

//////////////////////////////////////////////////////////////////////////////
uint32_t GetHandleID_Default()
{
	return 0;
}
static gethandleid_func g_func = GetHandleID_Default;
static LONG _used_memory = 0;
static LONG _memory_malloc = 0;
static LONG _memory_free = 0;
static LONG _memory_malloc_size = 0;
static LONG _memory_free_size = 0;
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
		if (!basiclib::BasicInterlockedCompareExchange((LONG*)&data->handle, handle, old_handle)){
			return nullptr;
		}
		if (old_alloc < 0)
		{
			basiclib::BasicInterlockedCompareExchange((LONG*)&data->allocated, 0, old_handle);
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
	BasicInterlockedExchangeAdd(&_memory_malloc_size, size);
	BasicInterlockedIncrement(&_memory_malloc);
	uint32_t* allocated = get_allocated_field(handleid);
	if (allocated)
	{
		basiclib::BasicInterlockedExchangeAdd((LONG*)allocated, size);
	}
	
}
void UpdateMallocState_Free(uint32_t handleid, size_t size)
{
	BasicInterlockedExchangeSub(&_used_memory, size);
	BasicInterlockedExchangeAdd(&_memory_free_size, size);
	BasicInterlockedIncrement(&_memory_free);
	uint32_t* allocated = get_allocated_field(handleid);
	if (allocated)
	{
		basiclib::BasicInterlockedExchangeSub((LONG*)allocated, size);
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
	if (g_nCheckMemMode & MemRunMemCheck_RunTongJi)
	{
		UpdateMallocState_Alloc(handleid, size);
	}
	if (g_nCheckMemMode & MemRunMemCheck_RunCheckMem)
	{
		if (m_nCheckMin <= size && size <= m_nCheckMax){
			basiclib::CSingleLock lock(&g_lockCheck, TRUE);
			stacktrace::call_stack dtStack(0);
			g_mapCheck[ptr].SwapStack(dtStack);
		}
	}
	return ptr;
}
inline void* Clean_prefix(char* ptr)
{
	ptr -= MallocFixSize;
	HeadFillFix* pFix = (HeadFillFix*)ptr;
	if (g_nCheckMemMode & MemRunMemCheck_RunCheckMem)
	{
		if (pFix->m_size >= m_nCheckMin && pFix->m_size <= m_nCheckMax){
			basiclib::CSingleLock lock(&g_lockCheck, TRUE);
			g_mapCheck.erase(ptr + MallocFixSize);
			lock.Unlock();
		}
	}
	
#ifdef _DEBUG
	if(pFix->m_bBegin != 1 || pFix->m_bEnd != 1)
	{
		//调用堆栈
		stacktrace::call_stack st(0);
		BasicLogEventErrorV("Free prefix error");
		BasicLogEventErrorV(st.to_string().c_str());
		exit(0);
	}
#endif
	if (g_nCheckMemMode & MemRunMemCheck_RunTongJi)
	{
		UpdateMallocState_Free(pFix->m_handleID, pFix->m_size);
	}
	return ptr;
}


#ifdef _USE_SYS_MALLOC
#define Fast_allocate malloc
#define Fast_reallocate realloc
#define Fast_deallocate free
#else	//_USE_SYS_MALLOC
extern "C"{
	#include "jemalloc.h"
}
#define Fast_allocate je_malloc
#define Fast_reallocate je_realloc
#define Fast_deallocate je_free
#endif //_USE_SYS_MALLOC
void* CheckFunc_allocate(size_t size)
{
	void* p = Fast_allocate(size + MallocFixSize);
	if(p == NULL)
		return NULL;
	return Fill_prefix((char*)p, size);
}

void* CheckFunc_reallocate(void* p, size_t size)
{
	if (p == nullptr)
		return CheckFunc_allocate(size);
	p = Clean_prefix((char*)p);
	if (p == nullptr)
		return CheckFunc_allocate(size);

	p = Fast_reallocate(p, size + MallocFixSize);
	if (p == NULL)
		return NULL;
	return Fill_prefix((char*)p, size);
}

void CheckFunc_deallocate(void* p)
{
	if(p == NULL)
		return;
	p = Clean_prefix((char*)p);
	Fast_deallocate(p);
}

///取内存分配信息
void BasicGetOperationInfo(
			size_t& nAllocateCount, 
			size_t& nDeallocateCount,
			size_t& nUseMemory,
			size_t& nAllocateSize,
			size_t& nDeAllocateSize
						 )
{
	nAllocateCount = _memory_malloc;
	nDeallocateCount = _memory_free;
	nUseMemory = _used_memory;
	nAllocateSize = _memory_malloc_size;
	nDeAllocateSize = _memory_free_size;
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

    BasicLogEvent(DebugLevel_Info, smBuf.GetDataBuffer());
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

////////////////////////////////////////////////////////////////////////////////////////////
typedef void*(*pallocateFunc)(size_t size);
typedef void*(*pReallocateFunc)(void* p, size_t size);
typedef void(*pDeallocateFunc)(void* p);
#ifdef _DEBUG
pallocateFunc g_pallocateFunc = CheckFunc_allocate;
pReallocateFunc g_pReallocateFunc = CheckFunc_reallocate;
pDeallocateFunc g_pDeallocateFunc = CheckFunc_deallocate;
#else
pallocateFunc g_pallocateFunc = Fast_allocate;
pReallocateFunc g_pReallocateFunc = Fast_reallocate;
pDeallocateFunc g_pDeallocateFunc = Fast_deallocate;
#endif

void BasicSetMemRunMemCheck(uint32_t nMode, int nMin, int nMax)
{
	if (nMode == 0)
	{
		g_pallocateFunc = Fast_allocate;
		g_pReallocateFunc = Fast_reallocate;
		g_pDeallocateFunc = Fast_deallocate;
	}
	else
	{
		g_pallocateFunc = CheckFunc_allocate;
		g_pReallocateFunc = CheckFunc_reallocate;
		g_pDeallocateFunc = CheckFunc_deallocate;
	}
	g_nCheckMemMode = nMode;
	m_nCheckMin = nMin;
	m_nCheckMax = nMax;

}
void DumpRunMemCheck()
{
	g_nCheckMemMode = 0;
	basiclib::CSingleLock lock(&g_lockCheck, TRUE);
	for (auto& checkData : g_mapCheck){
        basiclib::BasicLogEvent(DebugLevel_Info, checkData.second.to_string().c_str());
	}
}
void* BasicAllocate(size_t size){
	return g_pallocateFunc(size);
}
void* BasicReallocate(void* p, size_t size){
	return g_pReallocateFunc(p, size);
}
void BasicDeallocate(void* p){
	return g_pDeallocateFunc(p);
}

__NS_BASIC_END
