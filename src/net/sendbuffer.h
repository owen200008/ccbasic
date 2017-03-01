/***********************************************************************************************
// 文件名:     sendbuffer.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012-2-22 22:55:08
// 内容描述:   定义发送数据缓冲和发送队列
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_SENDBUFFER_H
#define BASIC_SENDBUFFER_H

#include <time.h>
#include <deque>
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////
//声明
class CBasicSendBuffer;

////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
///////////////////////////////////////////////////////////////////////////////
//发送的buffer
struct SendDataToSendThread
{
	int32_t		m_cbData;
	int32_t		m_nIndex;
	char*		m_pData;
	SendDataToSendThread(){
		m_cbData = 0;
		m_nIndex = 0;
		m_pData = nullptr;
	}
	SendDataToSendThread(const char* pBuffer, int32_t cbData){
		m_cbData = cbData;
		m_nIndex = 0;
		m_pData = (char*)basiclib::BasicAllocate(cbData);
		memcpy(m_pData, pBuffer, cbData);
	}
	void Reset(const char* pBuffer, int32_t cbData){
		ResetDataLength(cbData);
		memcpy(m_pData, pBuffer, cbData);
	}
	void ReleaseData(){
		if (m_pData)
			basiclib::BasicDeallocate(m_pData);
		m_pData = nullptr;
		m_cbData = 0;
	}
	int32_t ReadBuffer(char* pBuffer, int32_t nLength){
		int32_t lLeft = m_cbData - m_nIndex;
		if (lLeft > 0)
		{
			long lCopy = lLeft > nLength ? nLength : lLeft;
			if (pBuffer != NULL && nLength > 0)
			{
				memcpy(pBuffer, m_pData + m_nIndex, lCopy);
				m_nIndex += lCopy;
				return lCopy;
			}
		}
		return 0;
	}
	BOOL IsEmpty(){
		return m_nIndex >= m_cbData && m_pData != nullptr;
	}
	char* ResetDataLength(int32_t cbData){
		if (m_pData){
			basiclib::BasicDeallocate(m_pData);
		}
		m_cbData = cbData;
		m_nIndex = 0;
		m_pData = (char*)basiclib::BasicAllocate(cbData);
		return m_pData;
	}
};

class CMsgSendBufferQueue : public CMessageQueueLock<SendDataToSendThread>
{
public:
	CMsgSendBufferQueue();
	virtual ~CMsgSendBufferQueue();

	uint32_t ReadBuffer(char* pBuffer, uint32_t nLength);
	BOOL IsEmpty(){
		return GetMQLength() == 0;
	}
};

class CLockFreeMsgSendBufferQueue : public CLockFreeMessageQueue<SendDataToSendThread>
{
public:
	CLockFreeMsgSendBufferQueue();
	virtual ~CLockFreeMsgSendBufferQueue();

	//单线程调用
	uint32_t ReadBuffer(char* pBuffer, int32_t nLength);
	BOOL IsEmpty();

	void MQPush(SendDataToSendThread* pData);
	bool MQPop(SendDataToSendThread& pData);
protected:
	SendDataToSendThread	m_readData;
	bool					m_bRevertData;

	ConcurrentQueue::consumer_token_t	m_ctoken;
};

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////

#endif 
