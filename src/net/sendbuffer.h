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
#pragma	pack(1)
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
class CBasicSendBuffer;

////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
//函数 CopyTo 的返回值 
#define SB_COPY_POP			0x0001		//从队列里面弹出
#define SB_COPY_DELETE		0x0002		//删除数据

//no derived from CBasicobject
class CBasicSendBuffer			
{
public:
	CBasicSendBuffer();
	~CBasicSendBuffer();
public:
	long	ReadBuffer(char* pBuffer, long lLength);
	long	CopyTo(CBasicSendBuffer*& pNewSendBuffer, long& lCopy);		//返回值 SB_COPY_*
	
	BOOL	IsCanIgore(long lMaxLivingTime);
	BOOL	IsEmpty() { return m_lIndex >= m_lLength; }
	long	GetLength() { return m_lLength; }
public:
	static CBasicSendBuffer* AllocSendBuffer(const char* pBuffer, long lLength);
	static void DeleteSendBuffer(CBasicSendBuffer* pSendBuffer);
protected:
	long	GetSurvivalTime();
	void	InitBuffer(const char* pBuffer, long lLength);
protected:
	Net_Int	m_lLength;
	Net_Int	m_lIndex;
	time_t	m_tTime;
	char	m_pszBuffer[1];
};

typedef std::deque<CBasicSendBuffer*> send_buffer_queue;

//no derived from CBasicobject
class CBasicSendBufferQueue      
{
public:
	CBasicSendBufferQueue();
	~CBasicSendBufferQueue();
public:
	long AddSendBuffer(const char* pBuffer, long lLength);
	long ReadBuffer(char* pBuffer, long lLength);
	long CopyWaitQueue();			//由于带宽限制，把等待队列的数据复制到发送队列

	long EmptySendBuffer();
	void SetMaxLivingTime(long lMaxLivingTime) { m_lMaxLivingTime = lMaxLivingTime; }
	void SetMaxPacket(long lMaxPacket) { m_lMaxPacket = lMaxPacket; }
	void SetSendLimit(long lSendLimit) { m_lSendLimit = lSendLimit; }

	BOOL IsEmpty();
	long GetSendBufferMemory();
	long GetSendBufferCount();
protected:
	void _EmptySendBuffer(send_buffer_queue& Queue);
	void _GetSendBufferMemory(send_buffer_queue& Queue, long& lTotalSize);
protected:
	void ExamSendBuffer();
	void _ExamSendBuffer(send_buffer_queue& Queue);
protected:
	static Net_Int	m_lMaxLivingTime;		//数据包的最大生存时间（秒）
	static Net_Int	m_lMaxPacket;			//数据包的最大个数

	CCriticalSection	m_cs;			//队列的锁
	send_buffer_queue	m_sQueue;		//待发送的队列
	send_buffer_queue	m_wQueue;		//如果有带宽限制，保存的队列

	Net_Int				m_lSendLimit;	//发送的带宽限制  字节/秒
};

///////////////////////////////////////////////////////////////////////////////
//发送的buffer
#pragma	pack(1)
struct SendDataToSendThread
{
	Net_Int		m_cbData;
	char*		m_pData;
	Net_Int		m_nIndex;
	SendDataToSendThread(){
		m_cbData = 0;
		m_nIndex = 0;
		m_pData = nullptr;
	}
	SendDataToSendThread(const char* pBuffer, Net_Int cbData){
		m_cbData = cbData;
		m_nIndex = 0;
		m_pData = (char*)basiclib::BasicAllocate(cbData);
		memcpy(m_pData, pBuffer, cbData);
	}
	void ReleaseData(){
		if (m_pData)
			basiclib::BasicDeallocate(m_pData);
		m_pData = nullptr;
		m_cbData = 0;
	}
	Net_Int ReadBuffer(char* pBuffer, Net_Int nLength){
		Net_Int lLeft = m_cbData - m_nIndex;
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
};
#pragma	pack()

class CMsgSendBufferQueue : public CMessageQueueLock<SendDataToSendThread>
{
public:
	CMsgSendBufferQueue();
	virtual ~CMsgSendBufferQueue();

	Net_UInt ReadBuffer(char* pBuffer, Net_UInt nLength);
};

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack()
#endif 
