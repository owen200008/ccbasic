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

	virtual int InitCtx(CMQMgr* pMQMgr);

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
template<class... _Types>
void CCFrameSCBasicLogEventV(const char* pszLog, _Types&&... _Args){
	CCFrameSCBasicLogEventV(nullptr, pszLog, std::forward<_Types>(_Args)...);
}
template<class... _Types>
void CCFrameSCBasicLogEventErrorV(const char* pszLog, _Types&&... _Args){
	CCFrameSCBasicLogEventErrorV(nullptr, pszLog, std::forward<_Types>(_Args)...);
}


#endif