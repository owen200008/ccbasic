#include "../inc/basic.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#ifdef __BASICWINDOWS
extern "C"
{
    extern void coctx_swap(coctx_t *, coctx_t*);
}
#define EIP 4
#define ESP 5
void coctx_make(coctx_t *ctx, coctx_pfn_t pfn, const void* s1)
{
	memset(ctx->regs, 0, sizeof(ctx->regs));
    int *sp = (int*)(ctx->ss_sp + ctx->ss_size);
    sp = (int*)((unsigned long)sp & -16L);
    sp -= 2;
    sp[1] = (int)s1;
    ctx->regs[ESP] = (char*)sp;
    ctx->regs[EIP] = (char*)pfn;
}
#else
extern "C"
{
    extern void coctx_swap(coctx_t *, coctx_t*) asm("coctx_swap");
}
#endif

#if defined(__i386__)
#define EIP 4
#define ESP 5
void coctx_make(coctx_t *ctx, coctx_pfn_t pfn, const void* s1)
{
	memset(ctx->regs, 0, sizeof(ctx->regs));
    int *sp = (int*)(ctx->ss_sp + ctx->ss_size);
    sp = (int*)((unsigned long)sp & -16L);
    sp -= 2;
    sp[1] = (int)s1;
    ctx->regs[ESP] = (char*)sp;
    ctx->regs[EIP] = (char*)pfn;
}
#elif defined(__x86_64__)
#define RDI 9
#define RIP 10
#define RSP 11
void coctx_make(coctx_t *ctx, coctx_pfn_t pfn, const void *s1)
{
	memset(ctx->regs, 0, sizeof(ctx->regs));
    char* sp = ctx->ss_sp + ctx->ss_size;
    sp = (char*)((unsigned long)sp & -16LL);
    ctx->regs[RSP] = sp - 8;
    ctx->regs[RIP] = (char*)pfn;
    ctx->regs[RDI] = (char*)s1;
}
#endif

///////////////////////////////////////////////////////////////////////////
void MakeContextFunc(uint32_t low32, uint32_t hi32)
{
	uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
	CCorutinePlus* pThis = (CCorutinePlus*)ptr;
	pThis->StartFunc();
}

void MakeContextFuncLibco(CCorutinePlus* pThis)
{
	pThis->StartFuncLibco();
}

CCorutinePlus::CCorutinePlus()
{
	m_state = CoroutineState_Ready;
	m_nCap = 0;
	m_nSize = 0;
	m_stack = nullptr;
	m_pRunPool = nullptr;
    m_pUseRunPoolStack = nullptr;
	memset(m_pResumeParam, 0, sizeof(void*) * RESUME_MAXPARAM);
	m_bSysHook = true;
    m_ctx.ss_size = STACK_SIZE;
}

CCorutinePlus::~CCorutinePlus()
{
	if(m_stack)
	{
		basiclib::BasicDeallocate(m_stack);
		m_nSize = 0;
		m_nCap = 0;
		m_stack = nullptr;
	}
}

void CCorutinePlus::CheckStackSize(int nDefaultStackSize)
{
	if(nDefaultStackSize > m_nCap)
	{
		if(m_stack)
		{
			basiclib::BasicDeallocate(m_stack);
		}
		m_nCap = nDefaultStackSize;
		m_stack = (char*)basiclib::BasicAllocate(m_nCap);
	}
}

void CCorutinePlus::ReInit(coroutine_func func)
{
	m_func = func;
	m_state = CoroutineState_Ready;
}

void CCorutinePlus::YieldCorutine()
{
    m_pRunPool->YieldFunc(this);
}

void CCorutinePlus::StartFunc()
{
	m_func(this);
}
void CCorutinePlus::StartFuncLibco()
{
	StartFunc();

    m_pRunPool->FinishFunc(this);
}

void CCorutinePlus::SaveStack()
{
    int nLength = 0;
    char* pTop = m_pUseRunPoolStack + STACK_SIZE;
#ifdef __BASICWINDOWS
    __asm{
        mov         eax, dword ptr[pTop]
        sub         eax, esp
        mov         dword ptr[nLength], eax
    }
#else
#if defined(__i386__) 
	__asm __volatile(
	"mov %1,%%eax\nsub %%esp,%%eax\nmov %%eax, %0"
	: "=r"(nLength)
	: "r" (pTop)
	: "memory");
#elif defined(__x86_64__)
	__asm __volatile(
	"mov %1, %%rax\n\tsub %%rsp,%%rax\n\tmov %%eax,%0"
	: "=r"(nLength)
	: "r"(pTop)
	);
#endif
#endif
    CheckStackSize(nLength);
    m_nSize = nLength;
    memcpy(m_stack, pTop - nLength, m_nSize);
}

void CCorutinePlus::ResumeStack()
{
    m_ctx.ss_sp = m_pUseRunPoolStack;
    if (m_state == CoroutineState_Ready)
        coctx_make(&m_ctx, (coctx_pfn_t)MakeContextFuncLibco, this);
    else if (m_state == CoroutineState_Suspend)
        memcpy(m_ctx.ss_sp + STACK_SIZE - m_nSize, m_stack, m_nSize);
}

void CCorutinePlus::Resume(CCorutinePlusPool* pPool)
{
	m_pRunPool = pPool;
    m_pRunPool->ResumeFunc(this);
	m_pRunPool = nullptr;
}

///////////////////////////////////////////////////////////////////////////
CCorutinePlusPool::CCorutinePlusPool()
{
	m_nCreateTimes = 0;
	m_usRunCorutineStack = 0;
	for (int i = 0; i < CorutinePlus_Max_Stack; i++){
		m_pStackRunCorutine[i] = nullptr;
	}
	m_pStackRunCorutine[m_usRunCorutineStack++] = &m_selfPlus;
    m_selfPlus.m_pRunPool = this;
    m_vtStacks.reserve(5);
    //first is null, use for system
    m_vtStacks.push_back(nullptr);
    m_usStackSize = m_vtStacks.size();
    m_nCorutineSize = 0;
    m_nRealVTCorutineSize = 0;
}

CCorutinePlusPool::~CCorutinePlusPool()
{
	for(auto& c : m_vtCorutinePlus){
		delete c;
	}
	m_vtCorutinePlus.clear();
    for (auto& c : m_vtStacks){
        basiclib::BasicDeallocate(c);
    }
    m_vtStacks.clear();
}

bool CCorutinePlusPool::InitCorutine(int nDefaultSize, int nDefaultStackSize, int nShareStackSize)
{
	if(nDefaultSize <= 0 || nDefaultStackSize < 1024)
	{
		return false;
	}
	m_nDefaultStackSize = nDefaultStackSize;
	m_vtCorutinePlus.reserve(nDefaultSize * 2);
	for(int i = 0;i < nDefaultSize;i++){
        m_vtCorutinePlus.push_back(CreateCorutine());
	}
    m_nCorutineSize = m_vtCorutinePlus.size();
    m_nRealVTCorutineSize = m_vtCorutinePlus.size();
    m_vtStacks.reserve(nShareStackSize * 2 + 1);
    for (int i = 0; i < nShareStackSize; i++){
        m_vtStacks.push_back((char*)basiclib::BasicAllocate(STACK_SIZE));
    }
    m_usStackSize = m_vtStacks.size();
	return true;
}

CCorutinePlus* CCorutinePlusPool::GetCorutine()
{
    if (m_nCorutineSize == 0)
	{
		return CreateCorutine();
	}
    CCorutinePlus* pRet = m_vtCorutinePlus[m_nCorutineSize - 1];
    m_nCorutineSize--;
	return pRet;
}

void CCorutinePlusPool::ReleaseCorutine(CCorutinePlus* pPTR)
{
    if (m_nCorutineSize >= m_nRealVTCorutineSize){
        m_vtCorutinePlus.push_back(pPTR);
        m_nRealVTCorutineSize++;
    }
    else{
        m_vtCorutinePlus[m_nCorutineSize] = pPTR;
    }
    m_nCorutineSize++;
}

CCorutinePlus* CCorutinePlusPool::CreateCorutine()
{
	CCorutinePlus* pRet = new CCorutinePlus();	
	pRet->CheckStackSize(m_nDefaultStackSize);
	m_nCreateTimes++;
	return pRet;
}

void CCorutinePlusPool::YieldFunc(CCorutinePlus* pCorutine)
{
    m_usRunCorutineStack--;
    CCorutinePlus* pChange = m_pStackRunCorutine[m_usRunCorutineStack - 1];

    //进入函数会多保存一个栈(SaveStack的栈),不影响使用,必须在coctx_swap之前实现
    pCorutine->SaveStack();
    pCorutine->m_state = CoroutineState_Suspend;
#ifdef _DEBUG
    pCorutine->m_pUseRunPoolStack = nullptr;
#endif
    //不需要恢复栈
    coctx_swap(&pCorutine->m_ctx, &pChange->m_ctx);
}

void CCorutinePlusPool::FinishFunc(CCorutinePlus* pCorutine)
{
    pCorutine->m_state = CoroutineState_Death;
    m_usRunCorutineStack--;
#ifdef _DEBUG
    pCorutine->m_pUseRunPoolStack = nullptr;
#endif
    ReleaseCorutine(pCorutine);
    coctx_swap(&pCorutine->m_ctx, &m_pStackRunCorutine[m_usRunCorutineStack - 1]->m_ctx);
    basiclib::BasicLogEventError("error, much resume...");
}

void CCorutinePlusPool::ResumeFunc(CCorutinePlus* pNext)
{
#ifdef _DEBUG
    if (pNext->m_state != CoroutineState_Ready && pNext->m_state != CoroutineState_Suspend){
        ASSERT(0);
        return;
    }
#endif
    CCorutinePlus* pRunCorutine = m_pStackRunCorutine[m_usRunCorutineStack - 1];

    pNext->m_pUseRunPoolStack = GetStack(m_usRunCorutineStack);
    pNext->ResumeStack();
    m_pStackRunCorutine[m_usRunCorutineStack++] = pNext;
    pNext->m_state = CoroutineState_Running;
    coctx_swap(&pRunCorutine->m_ctx, &pNext->m_ctx);
}
char* CCorutinePlusPool::GetStack(int nIndex)
{
    if (m_usStackSize > nIndex){
        return m_vtStacks[nIndex];
    }
    else if (nIndex == m_usStackSize){
        m_vtStacks.push_back((char*)basiclib::BasicAllocate(STACK_SIZE));
        m_usStackSize += 1;
        return m_vtStacks[nIndex];
    }
    else{
        ASSERT(0);
    }
    return nullptr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
