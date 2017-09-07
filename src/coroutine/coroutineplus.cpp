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
    int *sp = (int*)(ctx->ss_sp + STACK_SIZE);
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
    int *sp = (int*)(ctx->ss_sp + STACK_SIZE);
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
    char* sp = ctx->ss_sp + STACK_SIZE;
    sp = (char*)((unsigned long)sp & -16LL);
    ctx->regs[RSP] = sp - 8;
    ctx->regs[RIP] = (char*)pfn;
    ctx->regs[RDI] = (char*)s1;
}
#endif

///////////////////////////////////////////////////////////////////////////
void MakeContextFuncLibco(CCorutinePlusBase* pThis){
	pThis->StartFuncLibco();
}

CCorutinePlusBase::CCorutinePlusBase(){
	m_state = CoroutineState_Ready;
	m_pRunPool = nullptr;
    m_pCreatePool = nullptr;
	m_pResumeParam = nullptr;
	m_func = nullptr;
}

CCorutinePlusBase::~CCorutinePlusBase()
{
}

void CCorutinePlusBase::Create(coroutine_func pFunc){
	m_state = CoroutineState_Ready;
	m_func = pFunc;
}

void CCorutinePlusBase::YieldCorutine(){
#ifdef _DEBUG
	char sStackSize = 0;
	int nLength = m_ctx.ss_sp + STACK_SIZE - &sStackSize;
	//预留10K的空间
	ASSERT(nLength < STACK_SIZE - 10 * 1024);
	static int nTotalUseStackSize = 0;
	if (nLength > nTotalUseStackSize) {
		basiclib::BasicLogEventV(basiclib::DebugLevel_Info, "CCorutinePlusBase::YieldCorutine MaxStack Size %dK", (int)(nLength / 1024));
		nTotalUseStackSize = nLength;
	}
#endif
    m_pRunPool->YieldFunc(this);
}

void CCorutinePlusBase::StartFuncLibco(){
	m_func(this);
    m_pRunPool->FinishFunc(this);
}

CoroutineState CCorutinePlusBase::Resume(CCorutinePlusPoolBase* pPool){
    m_tmResumeTime = time(NULL);
	m_pRunPool = pPool;
    m_pRunPool->ResumeFunc(this);
	m_pRunPool = nullptr;
    return m_state;
}

//! 判断是否是死循环或者没有唤醒
bool CCorutinePlusBase::IsCoroutineError(time_t tmNow){
    if (m_state == CoroutineState_Suspend || m_state == CoroutineState_Running)
        //超过5s都没有响应,说明是有问题或者死循环
        return tmNow - m_tmResumeTime > 5;
    return false;
}

///////////////////////////////////////////////////////////////////////////
CCorutinePlusPoolBalance     CCorutinePlusPoolBase::m_balance;
CCorutinePlusPoolMgr         CCorutinePlusPoolBase::m_poolMgr;
CCorutinePlusPoolBase::CCorutinePlusPoolBase(){
	m_usRunCorutineStack = 0;
	for (int i = 0; i < CorutinePlus_Max_Stack; i++){
		m_pStackRunCorutine[i] = nullptr;
	}
	m_pStackRunCorutine[m_usRunCorutineStack++] = &m_selfPlus;
    m_selfPlus.m_pRunPool = this;
    
    m_nRealVTCorutineSize = 0;
    m_nCorutineSize = 0;
    m_nCreateTimes = 0;
    m_nLimitSize = 1024;

    m_usShareStackSize = 0;
    m_nCreateTimesShareStack = 0;
    m_usMaxCreateShareStackSize = 1024;

    m_bInit = false;
}

CCorutinePlusPoolBase::~CCorutinePlusPoolBase()
{
	for(auto& c : m_vtCorutinePlus){
		delete c;
	}
	m_vtCorutinePlus.clear();
	char* pBuf[1024] = { 0 };
	int nIndex = 0;
	for (auto& c : m_vtStacks) {
		pBuf[nIndex++] = c;
		//返还堆栈
		if (nIndex == 1024) {
			m_balance.ReleaseExtraStack(pBuf, nIndex);
			nIndex = 0;
		}
	}
	if (nIndex > 0) {
		m_balance.ReleaseExtraStack(pBuf, nIndex);
	}
}

bool CCorutinePlusPoolBase::InitCorutine(int nDefaultSize, int nLimitCorutineCount, uint16_t nShareStackSize, uint16_t nMaxShareStackSize){
    if (m_bInit){
        return true;
    }
    if (nDefaultSize <= 0 || nLimitCorutineCount < nDefaultSize || nShareStackSize == 0 || nMaxShareStackSize < nShareStackSize)
    {
        return false;
    }
    m_bInit = true;
    m_nLimitSize = nLimitCorutineCount;
    m_usMaxCreateShareStackSize = nMaxShareStackSize;
    {
        //创建协程
        m_vtCorutinePlus.reserve(nDefaultSize * 2);
        for (int i = 0; i < nDefaultSize; i++){
            m_vtCorutinePlus.push_back(CreateCorutine());
        }
        m_nCorutineSize = m_vtCorutinePlus.size();
        m_nRealVTCorutineSize = m_nCorutineSize;
    }
    {
        //创建堆栈
        m_vtStacks.reserve(nShareStackSize * 2 + 1);
		for (int i = 0; i < nShareStackSize; i++) {
			m_vtStacks.push_back(CreateShareStack());
		}
        m_usShareStackSize = (uint16_t)m_vtStacks.size();
        m_usRealShareStackSize = m_usShareStackSize;
    }
    return m_bInit;
}

void CCorutinePlusBase::InitStackAndPool(CCorutinePlusPoolBase* pPool, char* pStack){
    m_pCreatePool = pPool;
    m_ctx.ss_sp = pStack;
	ASSERT(pStack != nullptr);
}

CCorutinePlusBase* CCorutinePlusPoolBase::GetCorutine(bool bGlobal){
	CCorutinePlusBase* pRet = nullptr;
    if (m_nCorutineSize == 0){
        //首先从全局拿一下
        if (bGlobal){
            //默认获取256
            uint16_t nHalf = m_nRealVTCorutineSize;
			CCorutinePlusBase* pBuf[256] = { 0 };
            if (nHalf > 256)
                nHalf = 256;
            int nGetCount = m_balance.GetCorutineMore(pBuf, nHalf);
            for (int i = 0; i < nGetCount; i++){
                m_vtCorutinePlus[m_nCorutineSize++] = pBuf[i];
            }
            return GetCorutine(false);
        }
        else{
            pRet = CreateCorutine();
        }
	}
    else{
        pRet = m_vtCorutinePlus[--m_nCorutineSize];
    }
    //! 获取堆栈
	char* pStackData = GetShareStack();
#ifdef _DEBUG
	if (nullptr == pStackData) {
		basiclib::BasicLogEvent(basiclib::DebugLevel_Error, "StackData Get Error!");
	}
#endif
    pRet->InitStackAndPool(this, pStackData);
	return pRet;
}

void CCorutinePlusPoolBase::ReleaseCorutine(CCorutinePlusBase* pPTR){
    if (m_nCorutineSize >= m_nRealVTCorutineSize){
        m_vtCorutinePlus.push_back(pPTR);
        m_nRealVTCorutineSize++;
    }
    else{
        m_vtCorutinePlus[m_nCorutineSize] = pPTR;
    }
    m_nCorutineSize++;
    //! 返还堆栈
    ReleaseShareStack(pPTR->m_ctx.ss_sp);

    if (m_nCorutineSize >= m_nLimitSize){
        //放到全局去
		CCorutinePlusBase* pBuf[1024] = { 0 };
        //默认返还一半
        uint16_t nHalf = m_nCorutineSize / 2;
        if (nHalf > 1024)
            nHalf = 1024;
        for (int i = 0; i < nHalf; i++){
            pBuf[i] = m_vtCorutinePlus[--m_nCorutineSize];
        }
        //返还堆栈
        m_balance.ReleaseCorutineMore(pBuf, nHalf);
    }
}

CCorutinePlusBase* CCorutinePlusPoolBase::CreateCorutine(){
	CCorutinePlusBase* pRet = ConstructCorutine();
	m_nCreateTimes++;
    m_poolMgr.CreateCorutine(pRet);
	return pRet;
}

void CCorutinePlusPoolBase::YieldFunc(CCorutinePlusBase* pCorutine){
    m_usRunCorutineStack--;
#ifdef _DEBUG
	if (m_usRunCorutineStack <= 0) {
		ASSERT(0);
	}
#endif
	CCorutinePlusBase* pChange = m_pStackRunCorutine[m_usRunCorutineStack - 1];

    pCorutine->m_state = CoroutineState_Suspend;
    //不需要恢复栈
    coctx_swap(&pCorutine->m_ctx, &pChange->m_ctx);
}

void CCorutinePlusPoolBase::FinishFunc(CCorutinePlusBase* pCorutine){
    pCorutine->m_state = CoroutineState_Death;
    m_usRunCorutineStack--;
    //返还协程
    ReleaseCorutine(pCorutine);

    //为了保证不要崩溃
    do{
        coctx_swap(&pCorutine->m_ctx, &m_pStackRunCorutine[m_usRunCorutineStack - 1]->m_ctx);
        //不应该进入这里
        ASSERT(0);
        basiclib::BasicLogEventError("error, finish must be no resume...");
    } while (true);
}

void CCorutinePlusPoolBase::ResumeFunc(CCorutinePlusBase* pNext){
#ifdef _DEBUG
    if (pNext->m_state != CoroutineState_Ready && pNext->m_state != CoroutineState_Suspend){
        ASSERT(0);
        return;
    }
#endif
	CCorutinePlusBase* pRunCorutine = m_pStackRunCorutine[m_usRunCorutineStack - 1];

    if (pNext->m_state == CoroutineState_Ready)
        coctx_make(&pNext->m_ctx, (coctx_pfn_t)MakeContextFuncLibco, pNext);
    m_pStackRunCorutine[m_usRunCorutineStack++] = pNext;
    pNext->m_state = CoroutineState_Running;
    coctx_swap(&pRunCorutine->m_ctx, &pNext->m_ctx);
}

void CCorutinePlusPoolBase::ReleaseShareStack(char* pStack){
    if (m_usShareStackSize >= m_usRealShareStackSize){
        m_vtStacks.push_back(pStack);
        m_usRealShareStackSize++;
    }
    else{
        m_vtStacks[m_usShareStackSize] = pStack;
    }
    m_usShareStackSize++;
    if (m_usShareStackSize > m_usMaxCreateShareStackSize){
        char* pBuf[512] = { 0 };
        //默认返还一半
        uint16_t nHalfShareStackSize = m_usShareStackSize / 2;
        if (nHalfShareStackSize > 512)
            nHalfShareStackSize = 512;
        for (int i = 0; i < nHalfShareStackSize; i++){
            pBuf[i] = m_vtStacks[--m_usShareStackSize];
        }
        //返还堆栈
        m_balance.ReleaseExtraStack(pBuf, nHalfShareStackSize);
    }
}

char* CCorutinePlusPoolBase::GetShareStack(bool bGlobal){
	char* pRet = nullptr;
    if (m_usShareStackSize == 0){
        //首先从全局拿一下
        if (bGlobal){
            //默认获取256
            uint16_t nHalf = m_usRealShareStackSize;
            char* pBuf[256] = { 0 };
            if (nHalf > 256)
                nHalf = 256;
            int nGetCount = m_balance.GetExtraStack(pBuf, nHalf);
            for (int i = 0; i < nGetCount; i++){
                m_vtStacks[m_usShareStackSize++] = pBuf[i];
            }
			pRet = GetShareStack(false);
        }
		else {
			pRet = CreateShareStack();
		}
    }
	else {
		pRet = m_vtStacks[--m_usShareStackSize];
	}
	return pRet;
}

char* CCorutinePlusPoolBase::CreateShareStack(){
	char* pRet = nullptr;
	if (m_vtCreateStack.MQPop(&pRet) == 0) {
		return pRet;
	}

	int nCreateSize = 0;
	pRet = m_balance.GetNewCreateStack(m_nCreateTimesShareStack, nCreateSize);
	char* pAddBegin = pRet + STACK_SIZE;
	for (int i = 1; i < nCreateSize; i++) {
		m_vtCreateStack.MQPush(&pAddBegin);
		pAddBegin += STACK_SIZE;
	}
	m_nCreateTimesShareStack++;
#ifdef _DEBUG
	if (pRet == nullptr) {
		return pRet;
	}
#endif
	return pRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CCorutinePlusPoolBalance::CCorutinePlusPoolBalance(){
}
CCorutinePlusPoolBalance::~CCorutinePlusPoolBalance(){
	m_queueCreateStack.Drop_Queue([](char**c)->void {
		basiclib::BasicDeallocate(*c);
	});
}
//! 获取多余的corutine
int CCorutinePlusPoolBalance::GetCorutineMore(CCorutinePlusBase* pPTR[], int nCount){
    int nGetCount = 0;
    m_queueExtraCorutine.SafeFuncCallback([&]()->void{
        for (nGetCount = 0; nGetCount < nCount; nGetCount++){
            if (m_queueExtraCorutine.SafeMQPop(&pPTR[nGetCount]) != 0)
                return;
        }
    });
    return nGetCount;
}
void CCorutinePlusPoolBalance::ReleaseCorutineMore(CCorutinePlusBase* pPTR[], int nCount)
{
    m_queueExtraCorutine.SafeFuncCallback([&]()->void{
        for (int i = 0; i < nCount; i++)
            m_queueExtraCorutine.SafeMQPush(&pPTR[i]);
    });
}

int CCorutinePlusPoolBalance::GetExtraStack(char* pBuf[], int nCount){
    int nGetCount = 0;
    m_queueExtraStack.SafeFuncCallback([&]()->void{
        for (nGetCount = 0; nGetCount < nCount; nGetCount++){
            if (m_queueExtraStack.SafeMQPop(&pBuf[nGetCount]) != 0)
                return;
        }
    });
    return nGetCount;
}

void CCorutinePlusPoolBalance::ReleaseExtraStack(char* pBuf[], int nCount){
    m_queueExtraStack.SafeFuncCallback([&]()->void{
        for (int i = 0; i < nCount;i++)
            m_queueExtraStack.SafeMQPush(&pBuf[i]);
    });
}
int CCorutinePlusPoolBalance::GetExtraCorutineCount(){
    return m_queueExtraCorutine.GetMQLength();
}
int CCorutinePlusPoolBalance::GetExtraShareStackCount(){
    return m_queueExtraStack.GetMQLength();
}
//! 获取创建的堆栈
char* CCorutinePlusPoolBalance::GetNewCreateStack(int nTimes, int& nCreateSize) {
	nCreateSize = 16 * (int)pow(2, nTimes + 1);
	if (nCreateSize < 16)
		nCreateSize = 16;
	else if (nCreateSize > 1024)
		nCreateSize = 1024;
	char* pRet = (char*)basiclib::BasicAllocate(nCreateSize * STACK_SIZE);
	m_queueCreateStack.MQPush(&pRet);
	return pRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CCorutinePlusPoolMgr::CCorutinePlusPoolMgr()
{

}

CCorutinePlusPoolMgr::~CCorutinePlusPoolMgr()
{

}
void CCorutinePlusPoolMgr::CreateCorutine(CCorutinePlusBase* pPTR){
    MQPush(&pPTR);
}
//! 检查所有的协程是否有问题
void CCorutinePlusPoolMgr::CheckAllCorutine(){
    int nLength = GetMQLength();
    if (nLength > 0){
		CCorutinePlusBase** pCheckCorutine = new CCorutinePlusBase*[nLength];
        {
            basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lock, TRUE);
            CopyAll(pCheckCorutine);
        }
        time_t tmNow = time(NULL);
        for (int i = 0; i < nLength; i++){
            if (pCheckCorutine[i]->IsCoroutineError(tmNow)){
                basiclib::BasicLogEventErrorV("协程检查出现异常,可能存在死循环或者逻辑不正确导致协程无法唤醒 状态(%d) 地址(%x)", pCheckCorutine[i]->GetCoroutineState(), pCheckCorutine[i]->m_func);
            }
        }
        delete[]pCheckCorutine;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//! 启动时候创建线程之前绑定param
CCorutinePlusThreadDataBase::CCorutinePlusThreadDataBase(){
    m_dwThreadID = basiclib::BasicGetCurrentThreadId();
    //默认调用一遍初始化
	m_pPool = CreatePool();
	m_pPool->InitCorutine();
}

CCorutinePlusThreadDataBase::~CCorutinePlusThreadDataBase(){
}

