#ifndef SKYNETPLUS_COROUTINE_CTX_THREADSTATE_H
#define SKYNETPLUS_COROUTINE_CTX_THREADSTATE_H

#include "../dllmodule.h"

class _SCBASIC_DLL_API CCtx_ThreadState : public CCoroutineCtx
{
public:
    CCtx_ThreadState();
	virtual ~CCtx_ThreadState();

	CreateTemplateHeader(CCtx_ThreadState);
	
    virtual int InitCtx(CMQMgr* pMQMgr, const std::function<const char*(InitGetParamType, const char* pKey, const char* pDefault)>& func);

	////////////////////////////////////////////////////////////////////////////////////////
	//业务类, 全部使用静态函数
	static DispatchReturn OnTimerShowThreadState(CCoroutineCtx* pCtx, CCorutinePlusThreadData* pData);
};



#endif