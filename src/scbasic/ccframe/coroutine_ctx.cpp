#include "coroutine_ctx.h"
#include "ctx_handle.h"
#include "ctx_threadpool.h"

uint32_t CCoroutineCtx::m_nTotalCtx = 0;
uint32_t& CCoroutineCtx::GetTotalCreateCtx()
{ 
	return m_nTotalCtx; 
}
//////////////////////////////////////////////////////////////////////////////
CCoroutineCtx::CCoroutineCtx(const char* pName)
{
	m_sessionID_Index = 0;
	m_ctxID = 0;
	if (pName)
	{
		m_strCtxName = pName;
	}
	basiclib::BasicInterlockedIncrement((LONG*)&m_nTotalCtx);
}

CCoroutineCtx::~CCoroutineCtx()
{
	ASSERT(m_bRelease == true);
	basiclib::BasicInterlockedDecrement((LONG*)&m_nTotalCtx);
}

//初始化
int CCoroutineCtx::InitCtx(CMQMgr* pMQMgr)
{
	if (m_ctxID != 0)
		return 1;
	if (0 == CSingletonCoroutineCtxHandle::Instance().Register(this)){
		return 1;
	}
	m_ctxMsgQueue.InitCtxMsgQueue(m_ctxID, pMQMgr);
	m_Ctoken.InitQueue(m_ctxMsgQueue);
	return 0;
}

void CCoroutineCtx::ReleaseCtx()
{
	m_bRelease = true;
	if (m_ctxID != 0){
		m_ctxMsgQueue.PushToMQMgr();
		return;
	}
	ASSERT(0);
}

//分配一个新的sessionid
int32_t CCoroutineCtx::GetNewSessionID()
{
	return basiclib::BasicInterlockedIncrement((LONG*)&m_sessionID_Index);
}

bool CCoroutineCtx::PushMessageByID(uint32_t nDstCtxID, ctx_message& msg)
{
	CRefCoroutineCtx pCtx = CSingletonCoroutineCtxHandle::Instance().GetContextByHandleID(nDstCtxID);
	if (pCtx == nullptr)
		return false;
	return pCtx->PushMessage(msg);
}
bool CCoroutineCtx::PushMessage(ctx_message& msg)
{
	m_ctxMsgQueue.MQPush(msg);
	return true;
}

//! 分配任务
DispatchReturn CCoroutineCtx::DispatchMsg(ctx_message& msg, CCorutinePlusThreadData* pData)
{
	DispatchReturn nRet = DispatchReturn_Success;
	switch (msg.m_nType)
	{
		case CTXMESSAGE_TYPE_RUNFUNC:
		{
			pRunFuncCtxMessageCallback pFunc = (pRunFuncCtxMessageCallback)msg.m_pFunc;
			pFunc(this, &msg, pData);
		}
		break;
		case CTXMESSAGE_TYPE_RUNCOROUTINE:
		{
			CCorutinePlus* pCorutine = (CCorutinePlus*)msg.m_pFunc;
			pCorutine->Resume(&pData->m_pool, msg, pData);
		}
		break;
		case CTXMESSAGE_TYPE_ONTIMER:
		{
			pCallbackOnTimerFunc pFunc = (pCallbackOnTimerFunc)msg.m_pFunc;
			nRet = (*pFunc)(this, pData);
		}
		break;
	}
	return nRet;
}

void CCoroutineCtxOnTimer(Net_PtrInt nKey, Net_PtrInt pParam1)
{
	uint32_t nSourceCtxID = pParam1;
	ctx_message msg(nSourceCtxID, nKey);
	int nRet = CCoroutineCtx::PushMessageByID(nSourceCtxID, msg);
	if (nRet == -1){
		//失败删除定时器
		CCtx_ThreadPool::GetThreadPool()->GetOnTimerModule().DelTimer(nKey);
	}
}

//! 加入timer
bool CCoroutineCtx::AddOnTimer(int nTimes, pCallbackOnTimerFunc pCallback)
{
	return CCtx_ThreadPool::GetThreadPool()->m_ontimerModule.AddOnTimer((Net_PtrInt)pCallback, CCoroutineCtxOnTimer, nTimes, m_ctxID);
}
bool CCoroutineCtx::AddOnTimeOut(int nTimes, pCallbackOnTimerFunc pCallback)
{
	return CCtx_ThreadPool::GetThreadPool()->m_ontimerModule.AddTimeOut((Net_PtrInt)pCallback, CCoroutineCtxOnTimer, nTimes, m_ctxID);
}
void CCoroutineCtx::DelTimer(pCallbackOnTimerFunc pCallback)
{
	return CCtx_ThreadPool::GetThreadPool()->m_ontimerModule.DelTimer((Net_PtrInt)pCallback);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
