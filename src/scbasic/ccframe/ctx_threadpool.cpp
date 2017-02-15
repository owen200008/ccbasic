#include "ctx_threadpool.h"
#include <basic.h>

////////////////////////////////////////////////////////////////////////////////////////////
CCorutinePlusThreadData::CCorutinePlusThreadData(CCtx_ThreadPool* pThreadPool)
{
	m_pThreadPool = pThreadPool;
	m_globalCToken.InitQueue(pThreadPool->m_globalMQMgrModule);
}

CCorutinePlusThreadData::~CCorutinePlusThreadData()
{
	if (m_pParam){
		m_pThreadPool->m_pReleaseFunc(m_pParam);
	}
}

void CCorutinePlusThreadData::InitThreadData(basiclib::CBasicThreadTLS* pTLS)
{
	m_dwThreadID = basiclib::BasicGetCurrentThreadId();
	pTLS->SetValue(this);
	m_pParam = m_pThreadPool->m_pCreateFunc(this);
}

void CCorutinePlusThreadData::ExecThreadOnWork()
{
	InitThreadData(&m_pThreadPool->m_threadIndexTLS);

	m_pool.InitCorutine(1024);
	CMQMgr* pMgrQueue = &m_pThreadPool->m_globalMQMgrModule;
	CMQMgr* pThreadMQ = &m_threadMgrQueue;
	uint32_t& usPacketNumber = m_pThreadPool->m_nDPacketNumPerTime;
	bool &bRunning = m_pThreadPool->m_bRunning;

	CCtxMessageQueue* pQ = nullptr;
	CCtxMessageQueue* pThreadQ = nullptr;

	basiclib::BasicLogEventV("启动ExecThreadOnWork %d ThreadID(%d)", basiclib::BasicGetCurrentThreadId(), m_dwThreadID);
	while (bRunning){
		pQ = DispathCtxMsg(pMgrQueue, pQ, usPacketNumber);
		if (pQ == nullptr) {
			pThreadQ = DispathCtxMsg(pThreadMQ, pThreadQ, usPacketNumber);
			if (pThreadQ == nullptr)
				pMgrQueue->WaitForGlobalMQ(100);
		}
	}
	basiclib::BasicLogEventV("退出ExecThreadOnWork %d ThreadID(%d)", basiclib::BasicGetCurrentThreadId(), m_dwThreadID);
}

CCtxMessageQueue* CCorutinePlusThreadData::DispathCtxMsg(CMQMgr* pMgrQueue, CCtxMessageQueue* pQ, uint32_t& usPacketNumber)
{
	if (pQ == NULL) {
		pQ = pMgrQueue->GlobalMQPop(m_globalCToken);
		if (pQ == NULL)
			return NULL;
	}
	uint32_t nCtxID = pQ->GetCtxID();
	CRefCoroutineCtx pCtx = CSingletonCoroutineCtxHandle::Instance().GetContextByHandleID(nCtxID);
	if (pCtx == nullptr){
		//发生异常
		basiclib::BasicLogEventErrorV("分发队列(DispathCtxMsg)找不到ctxid!!!");
		return pMgrQueue->GlobalMQPop(m_globalCToken);
	}
	if (pCtx->IsReleaseCtx()){
		//清空队列
		moodycamel::ConsumerToken token(*pQ);
		ctx_message msg;
		while (pQ->MQPop(token, msg)){
			msg.FormatMsgQueueToString([&](const char* pData, int nLength)->void{

			});
		}
		CSingletonCoroutineCtxHandle::Instance().UnRegister(pCtx.GetResFunc());
		return pMgrQueue->GlobalMQPop(m_globalCToken);
	}

	DispatchReturn nDispathRet = DispatchReturn_Success;
	for (int i = 0; i < nDealPacketNumber; i++){
		ctx_message msg;
		if (!pQ->MQPop(msg)){
			return pMgrQueue->GlobalMQPop();
		}
		//分配任务
		//nDispathRet = pCtx->DispatchMsg(msg, );
		if (nDispathRet < DispatchReturn_Success){
			basiclib::BasicLogEventErrorV("DispatchMsg出现错误(%d) Ctx(%d) Name(%s) SignName(%s)", nDispathRet, nCtxID, pCtx->GetCtxName().c_str(), pCtx->GetCtxSignName().c_str());
		}
		else if (nDispathRet == DispatchReturn_LockCtx){
			ASSERT(msg.sz == 0);
			break;
		}
	}
	CCtxMessageQueue* pNextQ = pMgrQueue->GlobalMQPop();
	if (pNextQ) {
		//锁定就不需要放到全局队列
		if (nDispathRet != DispatchReturn_LockCtx)
			pMgrQueue->GlobalMQPush(pQ);
		pQ = pNextQ;
	}
	return pQ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CCorutinePlusThreadData* GetCorutinePlusThreadData(basiclib::CBasicThreadTLS* pTLS)
{
	return (CCorutinePlusThreadData*)pTLS->GetValue();
}


///////////////////////////////////////////////////////////////////////////////////////////////
CCtx_ThreadPool::CCtx_ThreadPool()
{
	m_nDPacketNumPerTime = 1;
	m_bRunning = false;
}

CCtx_ThreadPool::~CCtx_ThreadPool()
{

}

bool CCtx_ThreadPool::Init(int nWorkThreadCount,
	const std::function<void*(CCorutinePlusThreadData*)>& pCreateFunc,
	const std::function<void*(void*)>& pReleaseParamFunc)
{
	m_bRunning = true;

	m_pCreateFunc = pCreateFunc;
	m_pReleaseFunc = pReleaseParamFunc;
	//创建indextls
	m_threadIndexTLS.CreateTLS();

	m_ontimerModule.InitTimer();

	DWORD dwThreadID = 0;
	for (int i = 0; i < nWorkThreadCount; i++){
		m_vtHandle.push_back(basiclib::BasicCreateThread(ThreadOnWork, new CCorutinePlusThreadData(this), &dwThreadID));
	}
	m_hMonitor = basiclib::BasicCreateThread(ThreadOnMonitor, this, &dwThreadID);
	return true;
}

//! 等待退出
void CCtx_ThreadPool::Wait()
{
	basiclib::BasicWaitThread(m_hMonitor);
	for (auto& workHandle : m_vtHandle){
		basiclib::BasicWaitThread(workHandle);
	}
	m_ontimerModule.CloseTimer();
	m_ontimerModule.WaitThreadExit();
}

THREAD_RETURN ThreadOnWork(void* pArgv)
{
	//随机因子
	srand(time(NULL) + basiclib::BasicGetTickTime() + basiclib::BasicGetCurrentThreadId());
	CCorutinePlusThreadData* pThreadData = (CCorutinePlusThreadData*)pArgv;
	pThreadData->ExecThreadOnWork();
	return 0;
}


THREAD_RETURN ThreadOnMonitor(void* pArgv)
{
	//随机因子
	srand(time(NULL) + basiclib::BasicGetTickTime() + basiclib::BasicGetCurrentThreadId());
	CCtx_ThreadPool* pPool = (CCtx_ThreadPool*)pArgv;
	pPool->ExecThreadOnMonitor();
	return 0;
}
void CCtx_ThreadPool::ExecThreadOnMonitor()
{
	uint32_t& nTotalCtx = CCoroutineCtx::GetTotalCreateCtx();
	basiclib::BasicLogEventV("启动ExecThreadOnMonitor %d", basiclib::BasicGetCurrentThreadId());
	while (nTotalCtx != 0){
		basiclib::BasicSleep(100);
	}
	m_bRunning = false;
	basiclib::BasicLogEventV("退出ExecThreadOnMonitor %d", basiclib::BasicGetCurrentThreadId());
}
