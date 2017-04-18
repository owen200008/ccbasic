#include "ctx_threadstate.h"
#include "../log/ctx_log.h"
#include "../ctx_threadpool.h"

CreateTemplateSrc(CCtx_ThreadState)

CCtx_ThreadState::CCtx_ThreadState() : CCoroutineCtx(GlobalGetClassName(CCtx_ThreadState))
{
}

CCtx_ThreadState::~CCtx_ThreadState()
{

}

int CCtx_ThreadState::InitCtx(CMQMgr* pMQMgr, const std::function<const char*(InitGetParamType, const char* pKey, const char* pDefault)>& func)
{
    int nRet = CCoroutineCtx::InitCtx(pMQMgr, func);
	if (nRet == 0){
		//60s
		AddOnTimer(6000, OnTimerShowThreadState);
	}
	return nRet;
}
//业务类, 全部使用静态函数
DispatchReturn CCtx_ThreadState::OnTimerShowThreadState(CCoroutineCtx* pCtx, CCorutinePlusThreadData* pData)
{
    CCFrameSCBasicLogEventV(pData, "ThreadID(%08d): CorutinePool(%d/%d Stack(%d))",
        pData->m_dwThreadID,
        pData->m_pool.GetCreateCorutineTimes(), pData->m_pool.GetVTCorutineSize(), pData->m_pool.GetStackCreateTimes());
	return DispatchReturn_Success;
}
