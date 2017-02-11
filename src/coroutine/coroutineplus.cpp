#include "../inc/basic.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

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
	memset(m_pResumeParam, 0, sizeof(void*) * RESUME_MAXPARAM);
	m_bSysHook = true;
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
	char* pTop = m_pRunPool->m_stack + STACK_SIZE;
	char dummy = 0;
	int nLength = pTop - &dummy;
	CheckStackSize(nLength);
	m_nSize = nLength;
	memcpy(m_stack, &dummy, m_nSize);

	m_state = CoroutineState_Suspend;

	m_pRunPool->m_usRunCorutineStack--;
#ifdef USE_UCONTEXT
	swapcontext(&m_ctx, &m_pRunPool->m_pStackRunCorutine[m_pRunPool->m_usRunCorutineStack - 1]->m_ctx);
#else
	coctx_swapcontext(&m_ctx, &m_pRunPool->m_pStackRunCorutine[m_pRunPool->m_usRunCorutineStack - 1]->m_ctx);
#endif
}

void CCorutinePlus::StartFunc()
{
	m_func(this);
	m_state = CoroutineState_Dead;
}
void CCorutinePlus::StartFuncLibco()
{
	StartFunc();

	m_pRunPool->m_usRunCorutineStack--;
	//exit no need to save stack
#ifndef USE_UCONTEXT
	coctx_swapcontext(&m_ctx, &m_pRunPool->m_pStackRunCorutine[m_pRunPool->m_usRunCorutineStack - 1]->m_ctx);
	printf("error, much resume...\n");
#endif
}

void CCorutinePlus::Resume(CCorutinePlusPool* pPool)
{
	m_pRunPool = pPool;
	switch(m_state)
	{
	case CoroutineState_Ready:
		{
			CCorutinePlus* pRunCorutine = pPool->GetCurrentCorutinePlus();
#ifdef USE_UCONTEXT
			getcontext(&m_ctx);
			m_ctx.uc_stack.ss_sp = pPool->m_stack;
			m_ctx.uc_stack.ss_size = STACK_SIZE;
			m_ctx.uc_link = &pRunCorutine->m_ctx;
			m_state = CoroutineState_Running;
			uintptr_t ptr = (uintptr_t)this;
			makecontext(&m_ctx, (void (*)(void))MakeContextFunc, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
			swapcontext(&pRunCorutine->m_ctx, &m_ctx);
#else
			m_ctx.ss_sp = pPool->m_stack;
			m_ctx.ss_size = STACK_SIZE;
			m_state = CoroutineState_Running;
			coctx_make(&m_ctx, (coctx_pfn_t)MakeContextFuncLibco, this);

			m_pRunPool->m_pStackRunCorutine[m_pRunPool->m_usRunCorutineStack++] = this;
			coctx_swapcontext(&pRunCorutine->m_ctx, &m_ctx);
#endif
			if(m_state == CoroutineState_Dead)
			{
				pPool->ReleaseCorutine(this);
			}
		}
		break;
	case CoroutineState_Suspend:
		{
			CCorutinePlus* pRunCorutine = pPool->GetCurrentCorutinePlus();
#ifdef USE_UCONTEXT
			m_ctx.uc_stack.ss_sp = pPool->m_stack;
			m_ctx.uc_stack.ss_size = STACK_SIZE;
			m_ctx.uc_link = &pRunCorutine->m_ctx;
			
			memcpy(pPool->m_stack + STACK_SIZE - m_nSize, m_stack, m_nSize);
			m_state = CoroutineState_Running;
			swapcontext(&pRunCorutine->m_ctx, &m_ctx);
#else
			m_ctx.ss_sp = pPool->m_stack;
			m_ctx.ss_size = STACK_SIZE;

			memcpy(pPool->m_stack + STACK_SIZE - m_nSize, m_stack,m_nSize);
			m_state = CoroutineState_Running;

			m_pRunPool->m_pStackRunCorutine[m_pRunPool->m_usRunCorutineStack++] = this;
			coctx_swapcontext(&pRunCorutine->m_ctx, &m_ctx);
#endif
			if(m_state == CoroutineState_Dead)
			{
				 pPool->ReleaseCorutine(this);
			}
		}
		break;
	case CoroutineState_Dead:
		{
			//do nothing
		}
		break;
	default:
		assert(0);
		break;
	}
	m_pRunPool = nullptr;
}

///////////////////////////////////////////////////////////////////////////
CCorutinePlusPool::CCorutinePlusPool()
{
	m_usCreateTimes = 0;
	m_usRunCorutineStack = 0;
	for (int i = 0; i < CorutinePlus_Max_Stack; i++){
		m_pStackRunCorutine[i] = nullptr;
	}
	m_pStackRunCorutine[m_usRunCorutineStack++] = &m_selfPlus;
}

CCorutinePlusPool::~CCorutinePlusPool()
{
	for(auto& c : m_vtCorutinePlus)
	{
		delete c;
	}
	m_vtCorutinePlus.clear();
}

bool CCorutinePlusPool::InitCorutine(int nDefaultSize, int nDefaultStackSize)
{
	if(nDefaultSize <= 0 || nDefaultStackSize < 1024)
	{
		return false;
	}
	m_nDefaultStackSize = nDefaultStackSize;
	m_vtCorutinePlus.reserve(nDefaultSize * 2);
	for(int i = 0;i < nDefaultSize;i++)
	{
		CreateCorutine(true);
	}
	return true;
}

CCorutinePlus* CCorutinePlusPool::GetCorutine()
{
	int nSize = m_vtCorutinePlus.size();
	if(nSize == 0)
	{
		return CreateCorutine(false);
	}
	CCorutinePlus* pRet = m_vtCorutinePlus[nSize - 1];
	m_vtCorutinePlus.pop_back();
	return pRet;
}

void CCorutinePlusPool::ReleaseCorutine(CCorutinePlus* pPTR)
{
	m_vtCorutinePlus.push_back(pPTR);
}

CCorutinePlus* CCorutinePlusPool::CreateCorutine(bool bPush)
{
	CCorutinePlus* pRet = new CCorutinePlus();	
	pRet->CheckStackSize(m_nDefaultStackSize);
	m_usCreateTimes++;
	if(bPush)
		m_vtCorutinePlus.push_back(pRet);
	return pRet;
}
////////////////////////////////////////////////////////////////////////////////////////////
CCorutinePlusThreadData::CCorutinePlusThreadData(basiclib::CBasicThreadTLS* pTLS, void* pParam)
{
	pTLS->SetValue(this);
	m_pParam = pParam;
}

CCorutinePlusThreadData::~CCorutinePlusThreadData()
{

}

CCorutinePlusThreadData* GetCorutinePlusThreadData(basiclib::CBasicThreadTLS* pTLS)
{
	return (CCorutinePlusThreadData*)pTLS->GetValue();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
