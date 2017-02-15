/***********************************************************************************************
// 文件名:     coroutine_ctx.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 内容描述:   协程资源基类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef SCBASIC_COROUTINE_CTX_H
#define SCBASIC_COROUTINE_CTX_H

#include "ctx_msgqueue.h"

enum DispatchReturn
{
	DispatchReturn_Error = -1,
	DispatchReturn_Success = 0, //代表包处理完成不需要保留
	DispatchReturn_Release = 1,	//代表release
	DispatchReturn_LockCtx = 1,	//代表锁定ctx，包内部释放	
};

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
typedef const char*(*pInitGetParamFunc)(int, const char*, const char*);
typedef fastdelegate::FastDelegate0<void> HandleOnTimer;			//错误消息
typedef void(*pCallOnTimerFunc)();
class CCoroutineCtx;
typedef DispatchReturn(*pCallbackOnTimerFunc)(CCoroutineCtx* pCtx);
class _SCBASIC_DLL_API CCoroutineCtx : public basiclib::CBasicObject, public basiclib::EnableRefPtr<CCoroutineCtx>
{
public:
	CCoroutineCtx(const char* pName = nullptr);
	virtual ~CCoroutineCtx();

	/*  初始化0代表成功
		回调参数
		0 代表 从lua文件取
	*/
	virtual int InitCtx(pInitGetParamFunc pFunc, CMQMgr* pMQMgr);
	//! 不需要自己delete，只要调用release
	virtual void ReleaseCtx();
	/*	分配任务 <0代表出错*/
	virtual DispatchReturn DispatchMsg(ctx_message& msg, CCorutinePlusThreadData& data);
	//! 加入timer
	bool AddOnTimer(int nTimes, pCallbackOnTimerFunc pCallback);
	bool AddOnTimeOut(int nTimes, pCallbackOnTimerFunc pCallback);
	void DelTimer(pCallbackOnTimerFunc pCallback);
public:
	uint32_t GetCtxID(){ return m_ctxID; }
	basiclib::CBasicString& GetCtxName(){ return m_strCtxName; }
	basiclib::CBasicString& GetCtxSignName(){ return m_strSignName; }
	//分配一个新的sessionid
	int32_t GetNewSessionID();
public:
	static uint32_t& GetTotalCreateCtx();

	static bool PushMessageByID(ctx_message& msg);
	bool PushMessage(ctx_message& msg);
	///////////////////////////////////////////////////////////////////////////////////////////
	//定义线程安全的函数
public:
	bool IsReleaseCtx(){ return m_bRelease; }
protected:
	bool										m_bRelease;
	uint32_t									m_ctxID;
	int32_t										m_sessionID_Index;
	basiclib::CBasicString						m_strCtxName;
	basiclib::CBasicString						m_strSignName;
	CCtxMessageQueue							m_ctxMsgQueue;
	moodycamel::ConsumerToken					m_Ctoken;		//优化读取

	//统计总共创建的上下文
	static uint32_t				m_nTotalCtx;
	
	friend class CCoroutineCtxHandle;
};
typedef basiclib::CBasicRefPtr<CCoroutineCtx> CRefCoroutineCtx;
#pragma warning (pop)

#endif