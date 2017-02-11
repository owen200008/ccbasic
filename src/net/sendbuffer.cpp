#include "../inc/basic.h"
#include "sendbuffer.h"
#include "net.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START

//////////////////////////////////////////////////////////////////////////////////////////////////
CMsgSendBufferQueue::CMsgSendBufferQueue()
{

}

CMsgSendBufferQueue::~CMsgSendBufferQueue()
{

}

uint32_t CMsgSendBufferQueue::ReadBuffer(char* pBuffer, uint32_t nLength)
{
#define MAX_COMBIN_COPY		128
	SendDataToSendThread m_copyRelease[MAX_COMBIN_COPY];
	int nReleaseIndex = 0;

	uint32_t lCopy = 0;
	basiclib::CSpinLockFunc lock(&m_lock, TRUE);
	while (m_head != m_tail)
	{
		SendDataToSendThread* pSendData = &(m_queue[m_head]);
		int32_t nThisCopy = pSendData->ReadBuffer(pBuffer, nLength);
		//读完之后再拷贝
		m_copyRelease[nReleaseIndex++] = *pSendData;

		if (pSendData->IsEmpty()){
			m_head++;

			if (m_head >= m_cap){
				m_head = 0;
			}
			int length = m_tail - m_head;
			if (length < 0){
				length += m_cap;
			}
			while (length > m_overload_threshold)
			{
				if (m_overloadCallback != nullptr)
					m_overloadCallback(m_overload_threshold);
				m_overload_threshold *= 2;
			}
		}
		if (nThisCopy <= 0){
			break;
		}
		lCopy += nThisCopy;
		pBuffer += nThisCopy;
		nLength -= nThisCopy;
		if (nLength <= 0)
		{
			break;
		}
		if (nReleaseIndex >= MAX_COMBIN_COPY)
		{
			break;
		}
	}
	if (m_head == m_tail)
	{
		// reset overload_threshold when queue is empty
		m_overload_threshold = m_defaultoverload;
	}
	lock.UnLock();
	for (int i = 0; i < MAX_COMBIN_COPY; i++)
	{
		if (m_copyRelease[i].IsEmpty())
		{
			m_copyRelease[i].ReleaseData();
		}
		else
		{
			break;
		}
	}
	return lCopy;
}

/////////////////////////////////////////////////////////////////////////
CLockFreeMsgSendBufferQueue::CLockFreeMsgSendBufferQueue()
{
	m_bRevertData = false;
}

CLockFreeMsgSendBufferQueue::~CLockFreeMsgSendBufferQueue()
{

}

uint32_t CLockFreeMsgSendBufferQueue::ReadBuffer(char* pBuffer, int32_t nLength)
{
	uint32_t lCopy = 0;
	if (m_bRevertData){
		int32_t nThisCopy = m_readData.ReadBuffer(pBuffer, nLength);
		if (!m_readData.IsEmpty()){
			return nThisCopy;
		}
		lCopy += nThisCopy;
		pBuffer += nThisCopy;
		nLength -= nThisCopy;
		m_bRevertData = false;
		m_readData.ReleaseData();
	}
	while (nLength > 0){
		if (!try_dequeue(m_readData)){
			break;
		}
		int32_t nThisCopy = m_readData.ReadBuffer(pBuffer, nLength);
		lCopy += nThisCopy;
		pBuffer += nThisCopy;
		nLength -= nThisCopy;
		if (!m_readData.IsEmpty()){
			m_bRevertData = true;
			break;
		}
		m_readData.ReleaseData();
	}
	return lCopy;
}

__NS_BASIC_END
