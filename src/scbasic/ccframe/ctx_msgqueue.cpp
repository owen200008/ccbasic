#include "ctx_msgqueue.h"
//////////////////////////////////////////////////////////////////////////////////////////////
CCtxMessageQueue::CCtxMessageQueue() : m_nAddTimes(0)
{
	m_nCtxID = 0;
	m_pMgr = nullptr;
}

CCtxMessageQueue::~CCtxMessageQueue()
{
}

//! ³õÊ¼»¯º¯Êý
void CCtxMessageQueue::InitCtxMsgQueue(uint32_t nCtxID, CMQMgr* pMgr)
{
	m_nCtxID = nCtxID;
	m_pMgr = pMgr;
}

bool CCtxMessageQueue::MQPush(ctx_message& message)
{
	moodycamel::ProducerToken token(*this);
	return MQPush(token, message);
}

bool CCtxMessageQueue::MQPush(moodycamel::ProducerToken& token, ctx_message& message)
{
	enqueue(token, std::move(message));
	uint32_t nRet = m_nAddTimes.fetch_add(1);
	if (nRet == 0){
		m_pMgr->GlobalMQPush(this);
		return true;
	}
	return false;
}

bool CCtxMessageQueue::MQPop(ctx_message& message)
{
	moodycamel::ConsumerToken token(*this);
	return MQPop(token, message);
}

bool CCtxMessageQueue::MQPop(moodycamel::ConsumerToken& token, ctx_message& message)
{
	uint32_t nDequeueAddTimes = m_nAddTimes;
	bool bRet = try_dequeue(token, message);
	if (!bRet){
		LONG lSet = 0;
		if (!m_nAddTimes.compare_exchange_weak(nDequeueAddTimes, 0)){
			return MQPop(token, message);
		}
	}
	return bRet;
}

void CCtxMessageQueue::PushToMQMgr()
{
	uint32_t nRet = m_nAddTimes.fetch_add(1);
	if (nRet == 1){
		m_pMgr->GlobalMQPush(this);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
CMQMgr::CMQMgr() : m_bWaitThread(true)
{
	m_pEvent = new basiclib::CEvent(FALSE, TRUE);
}

CMQMgr::~CMQMgr()
{
	if (m_pEvent){
		delete m_pEvent;
		m_pEvent = nullptr;
	}
}

void CMQMgr::GlobalMQPush(CCtxMessageQueue* queue)
{
	moodycamel::ProducerToken token(*this);
	GlobalMQPush(token, queue);
}
void CMQMgr::GlobalMQPush(moodycamel::ProducerToken& token, CCtxMessageQueue* queue)
{
	bool bRet = enqueue(token, queue);
	if (bRet){
		if (m_bWaitThread.exchange(false)){
			m_pEvent->SetEvent();
		}
	}
}

void CMQMgr::WaitForGlobalMQ(unsigned int nTimeout)
{
	m_bWaitThread.store(true);
	basiclib::BasicWaitForSingleObject(*m_pEvent, nTimeout);
}

CCtxMessageQueue* CMQMgr::GlobalMQPop(moodycamel::ConsumerToken& token)
{
	CCtxMessageQueue* pRet = nullptr;
	try_dequeue(token, pRet);
	return pRet;
}
