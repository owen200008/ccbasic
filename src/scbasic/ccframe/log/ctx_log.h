#ifndef SCBASIC_CCFRAME_LOG_H
#define SCBASIC_CCFRAME_LOG_H

#include "../dllmodule.h"

class CCorutinePlusThreadData;
class _SCBASIC_DLL_API CCoroutineCtx_Log : public CCoroutineCtx
{
public:
    CCoroutineCtx_Log();
	virtual ~CCoroutineCtx_Log();

	CreateTemplateHeader(CCoroutineCtx_Log);

    virtual int InitCtx(CMQMgr* pMQMgr, const std::function<const char*(InitGetParamType, const char* pKey, const char* pDefault)>& func);

	void LogEvent(CCorutinePlusThreadData* pThreadData, int nChannel, const char* pszLog);
	////////////////////////////////////////////////////////////////////////////////////////
	//业务类, 全部使用静态函数
	static DispatchReturn OnTimerBasicLog(CCoroutineCtx* pCtx, CCorutinePlusThreadData* pData);
protected:
	void LogEventCorutine(CCorutinePlusThreadData* pThreadData, int nChannel, basiclib::WriteLogDataBuffer& logData);
};

//!事件记录
 _SCBASIC_DLL_API void CCFrameSCBasicLogEventV(CCorutinePlusThreadData* pThreadData, const char* pszLog, ...);
 _SCBASIC_DLL_API void CCFrameSCBasicLogEventErrorV(CCorutinePlusThreadData* pThreadData, const char* pszLog, ...);
 _SCBASIC_DLL_API void CCFrameSCBasicLogEvent(CCorutinePlusThreadData* pThreadData, const char* pszLog);
 _SCBASIC_DLL_API void CCFrameSCBasicLogEventError(CCorutinePlusThreadData* pThreadData, const char* pszLog);
 _SCBASIC_DLL_API void CCFrameSCBasicLogEvent(const char* pszLog);
 _SCBASIC_DLL_API void CCFrameSCBasicLogEventError(const char* pszLog);


#endif