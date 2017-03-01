/***********************************************************************************************
// 文件名:     ctx_msgqueue.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 内容描述:   上下文的消息队列
// 版本信息:   1.0V
************************************************************************************************/
#ifndef SCBASIC_CTX_MSGQUEUE_H
#define SCBASIC_CTX_MSGQUEUE_H

#include <basic.h>
#include "../scbaisc_head.h"

#define CTXMESSAGE_TYPE_NOTHING					0		//其实是出错的包
#define CTXMESSAGE_TYPE_RUNFUNC					1		//执行函数		
#define CTXMESSAGE_TYPE_RUNCOROUTINE			2		//唤醒协程
#define CTXMESSAGE_TYPE_ONTIMER					3		//ontimer
#define CTXMESSAGE_TYPE_SOCKETMSG				4		//socketmsg

//执行函数的定义
class CCoroutineCtx;
struct ctx_message;
class CCorutinePlusThreadData;
typedef void(*pRunFuncCtxMessageCallback)(CCoroutineCtx*, ctx_message*, CCorutinePlusThreadData* pData);

#pragma	pack(1)
struct _SCBASIC_DLL_API ctx_message
{
	uint32_t		m_nSourceCtxID;
	intptr_t		m_session;
	uint32_t		m_nType;
	void*			m_pFunc;
	void*			m_data;
	size_t			sz;
	ctx_message(){
		memset(this, 0, sizeof(ctx_message));
	}
	ctx_message(uint32_t nCtxID){
		memset(this, 0, sizeof(ctx_message));
		m_nSourceCtxID = nCtxID;
	}
	//! 执行函数
	ctx_message(uint32_t nCtxID, pRunFuncCtxMessageCallback pFunc, intptr_t nSession = 0){
		memset(this, 0, sizeof(ctx_message));
		m_nType = CTXMESSAGE_TYPE_RUNFUNC;
		m_nSourceCtxID = nCtxID;
		m_pFunc = pFunc;
		m_session = nSession;
	}
	//! 执行协程
	ctx_message(uint32_t nCtxID, CCorutinePlus* pCorutine, intptr_t nSession = 0){
		memset(this, 0, sizeof(ctx_message));
		m_nType = CTXMESSAGE_TYPE_RUNCOROUTINE;
		m_nSourceCtxID = nCtxID;
		m_pFunc = pCorutine;
		m_session = nSession;
	}
	//! ontimer
	ctx_message(uint32_t nCtxID, Net_PtrInt nOnTimerKey, intptr_t nSession = 0){
		memset(this, 0, sizeof(ctx_message));
		m_nType = CTXMESSAGE_TYPE_ONTIMER;
		m_nSourceCtxID = nCtxID;
		m_pFunc = (void*)nOnTimerKey;
		m_session = nSession;
	}
	ctx_message(ctx_message&& msg){
		*this = msg;
		msg.sz = 0;
	}
	ctx_message& operator = (ctx_message&& msg){
		*this = msg;
		msg.sz = 0;
		return *this;
	}
	void ExportFromSmartBuffer(basiclib::CBasicSmartBuffer& smBuf){
		basiclib::SmartBufferExportOutData data;
		smBuf.ExportOutData(data);
		m_data = data.m_pExport;
		sz = data.m_nLength;
		data.m_pExport = nullptr;
		data.m_nLength = 0;
	}
	void CloneData(const char* pData, int nLength){
		if (nLength != 0){
			sz = nLength;
			m_data = basiclib::BasicAllocate(sz);
			memcpy(m_data, pData, sz);
		}
	}
	void ReleaseData(){
		if (sz > 0 && m_data){
			basiclib::BasicDeallocate(m_data);
		}
	}
	//! 可读的字符串
	void FormatMsgQueueToString(const std::function<void(const char* pData, int nLength)>& func){
		char szBuf[128] = { 0 };
#pragma warning (push)
#pragma warning (disable: 4293)
		func(szBuf, 
			sprintf(szBuf, "C(%d) S(%x%x) T(%d) SZ(%d) F(%x%x) D(%x%x)", m_nSourceCtxID, 
			(uint32_t)(m_session >> 32), (uint32_t)m_session, m_nType, sz, 
			(uint32_t)((intptr_t)m_pFunc >> 32), (uint32_t)m_pFunc, 
			(uint32_t)((intptr_t)m_data >> 32), (uint32_t)m_data));
#pragma warning (pop)
	}
};
#pragma	pack()

class CMQMgr;
//多生产者单消费者
class CCtxMessageQueue : public basiclib::CLockFreeMessageQueue<ctx_message, 32>
{
public:
	CCtxMessageQueue();
	virtual ~CCtxMessageQueue();

	//! 初始化函数
	void InitCtxMsgQueue(uint32_t nCtxID, CMQMgr* pMgr);
	
	//! 插入认为一定成功，返回代表是否通知,这个token必须是自己的
	bool MQPush(moodycamel::ProducerToken& token, ctx_message& message);
	bool MQPush(ctx_message& message);
	//! 单个消费者（单线程）,返回是否已经全部取完
	bool MQPop(moodycamel::ConsumerToken& token, ctx_message& message);
	bool MQPop(ctx_message& message);

	void PushToMQMgr();
	uint32_t GetCtxID(){ return m_nCtxID; }
protected:
	uint32_t									m_nCtxID;
	std::atomic<uint32_t>						m_nAddTimes;
	CMQMgr*										m_pMgr;

	friend class CMQMgr;
	friend class CCoroutineCtx;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMQMgr : public basiclib::CLockFreeMessageQueue<CCtxMessageQueue*, 1024>
{
public:
	CMQMgr();
	virtual ~CMQMgr();

	void GlobalMQPush(CCtxMessageQueue* queue);
	void GlobalMQPush(moodycamel::ProducerToken& token, CCtxMessageQueue* queue);
	CCtxMessageQueue* GlobalMQPop(moodycamel::ConsumerToken& token);

	//wait timeout ms
	void WaitForGlobalMQ(unsigned int nTimeout);
protected:
	basiclib::CEvent*		m_pEvent;
	std::atomic<bool>		m_bWaitThread;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif