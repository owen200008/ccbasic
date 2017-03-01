/***********************************************************************************************
// 文件名:     ctx_msgqueue.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 内容描述:   用于协程和异步的协程池
// 版本信息:   1.0V
************************************************************************************************/

#ifndef SCBASIC_CTX_THREADPOOL_H
#define SCBASIC_CTX_THREADPOOL_H

#include "ctx_handle.h"
#include "dllmodule.h"
#include "log/ctx_log.h"

//call in the thread create
class basiclib::CBasicThreadTLS;
class CCtx_ThreadPool;
class _SCBASIC_DLL_API CCorutinePlusThreadData : public basiclib::CBasicObject
{
public:
	CCorutinePlusThreadData(CCtx_ThreadPool* pThreadPool);
	virtual ~CCorutinePlusThreadData();

	void InitThreadData(basiclib::CBasicThreadTLS* pTLS);

	//执行
	void ExecThreadOnWork();

	//! 封装执行一个协程

protected:
	CCtxMessageQueue* DispathCtxMsg(CMQMgr* pMgrQueue, CCtxMessageQueue* pQ, uint32_t& usPacketNumber);
public:
	CCtx_ThreadPool*									m_pThreadPool;
	//全局消息消费者token
	moodycamel::ConsumerToken							m_globalCToken;

	DWORD												m_dwThreadID;
	CCorutinePlusPool									m_pool;
	CMQMgr												m_threadMgrQueue;
	void*												m_pParam;
};
CCorutinePlusThreadData* GetCorutinePlusThreadData(basiclib::CBasicThreadTLS* pTLS);

typedef fastdelegate::FastDelegate0<void*> OnCreateUDData;
class _SCBASIC_DLL_API CCtx_ThreadPool : public basiclib::CBasicObject
{
public:
	static CCtx_ThreadPool* GetThreadPool();
	static CCorutinePlusThreadData* GetSelfThreadData();
	static void CreateThreadPool(CCtx_ThreadPool* pPool);
public:
	//必须实现的虚函数
	virtual const char* GetCtxInitString(InitGetParamType nType, const char* pParam, const char* pDefault) = 0;
public:
	CCtx_ThreadPool();
	virtual ~CCtx_ThreadPool();

	//! 初始化
	virtual bool Init(const std::function<void*(CCorutinePlusThreadData*)>& pCreateFunc, 
		const std::function<void*(void*)>& pReleaseParamFunc);
	//! 等待退出
	void Wait();
public:
	basiclib::CBasicThreadTLS& GetTLS(){ return m_threadIndexTLS; }
	CBasicOnTimer& GetOnTimerModule();
	CMQMgr& GetGlobalMQMgr(){ return m_globalMQMgrModule; }
	CDllRegisterCtxTemplateMgr& GetCtxTemplateRegister(){ return m_mgtDllCtx; }
protected:
	friend THREAD_RETURN ThreadOnWork(void*);
	
	friend THREAD_RETURN ThreadOnMonitor(void*);
	void ExecThreadOnMonitor();
protected:
	bool														m_bRunning;
	basiclib::CBasicThreadTLS									m_threadIndexTLS;
	CBasicOnTimer												m_ontimerModule;			//执行ontimer callback
	basiclib::basic_vector<HANDLE>								m_vtHandle;
	HANDLE														m_hMonitor;

	CMQMgr														m_globalMQMgrModule;
	uint32_t													m_nDPacketNumPerTime;

	std::function<void*(CCorutinePlusThreadData*)>				m_pCreateFunc;
	std::function<void*(void*)>									m_pReleaseFunc;
	//ctx管理
	CDllRegisterCtxTemplateMgr									m_mgtDllCtx;
protected:
	//不会释放的ctx
	CCoroutineCtx_Log*											m_pLog;

	friend class CCoroutineCtx;
	friend class CCorutinePlusThreadData;
};

#endif