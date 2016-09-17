#include "../inc/basic.h"

#include "sendbuffer.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
CBasicSendBuffer::CBasicSendBuffer()
{
	memset(this, 0, sizeof(CBasicSendBuffer));
}

CBasicSendBuffer::~CBasicSendBuffer()
{

}

long CBasicSendBuffer::ReadBuffer(char* pBuffer, long lLength)
{
	long lLeft = m_lLength - m_lIndex;
	if(lLeft > 0)
	{
		long lCopy = lLeft > lLength ? lLength : lLeft;
		if(pBuffer != NULL && lLength > 0)
		{
			memcpy(pBuffer, &m_pszBuffer[m_lIndex], lCopy);
			m_lIndex += lCopy;
			return lCopy;
		}
	}
	return 0;
}

long CBasicSendBuffer::CopyTo(CBasicSendBuffer*& pNewSendBuffer, long& lCopy)
{
	long lRet = 0;
	long lLeft = m_lLength - m_lIndex;
	if(lLeft > 0)
	{
		if(m_lIndex == 0 && lLeft <= lCopy)
		{
			pNewSendBuffer = this;
			lCopy -= lLeft;
			lRet |= SB_COPY_POP;
		}
		else
		{
			long lLength = 0;
			if(lLeft > lCopy)
			{
				lLength = lCopy;
			}
			else
			{
				lLength = lLeft;
				lRet = SB_COPY_POP | SB_COPY_DELETE;
			}
			lCopy -= lLength;
			
			pNewSendBuffer = AllocSendBuffer(&m_pszBuffer[m_lIndex], lLength);
			
			m_lIndex += lLength;
		}
	}
	else
	{
		lRet = SB_COPY_POP | SB_COPY_DELETE;
	}
	return lRet;
}

BOOL CBasicSendBuffer::IsCanIgore(long lMaxLivingTime)
{
	if(lMaxLivingTime <= 0)
	{
		return FALSE;
	}
	if(GetSurvivalTime() > lMaxLivingTime)
	{
		return TRUE;
	}
	return FALSE;
}

long CBasicSendBuffer::GetSurvivalTime()
{
	return (long)(time(NULL) - m_tTime);
}

void CBasicSendBuffer::InitBuffer(const char* pBuffer, long lLength)
{
	m_lLength = lLength;
	m_lIndex  = 0;
	m_tTime   = time(NULL);

	memcpy(m_pszBuffer, pBuffer, lLength);
}

CBasicSendBuffer* CBasicSendBuffer::AllocSendBuffer(const char* pBuffer, long lLength)
{
	long lSize = sizeof(CBasicSendBuffer) - 1 + lLength;
	CBasicSendBuffer* pSendBuffer = (CBasicSendBuffer*)BasicAllocate(lSize);
	pSendBuffer->InitBuffer(pBuffer, lLength);
	return pSendBuffer;
}

void CBasicSendBuffer::DeleteSendBuffer(CBasicSendBuffer* pSendBuffer)
{
	BasicDeallocate((void*)pSendBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicSendBufferQueue::CBasicSendBufferQueue()
{
	m_lSendLimit = 0;
}

CBasicSendBufferQueue::~CBasicSendBufferQueue()
{
	EmptySendBuffer();
}

long CBasicSendBufferQueue::AddSendBuffer(const char* pBuffer, long lLength)
{
	CSingleLock lock(&m_cs);
	lock.Lock();

	ExamSendBuffer();

	send_buffer_queue& Queue = m_lSendLimit > 0 ? m_wQueue : m_sQueue;

	if(m_lMaxPacket > 0 && (long)Queue.size() > m_lMaxPacket)
	{
		return BASIC_NET_OUTOF_MAXPACKET;
	}
	CBasicSendBuffer* pSendBuffer = CBasicSendBuffer::AllocSendBuffer(pBuffer, lLength);
	if(pSendBuffer == NULL)
	{
		return BASIC_NET_NO_MEMORY;
	}
	Queue.push_back(pSendBuffer);
	return lLength;
}

long CBasicSendBufferQueue::ReadBuffer(char* pBuffer, long lLength)
{
	long lCopy = 0;
	CSingleLock lock(&m_cs);
	lock.Lock();
	while(!m_sQueue.empty())
	{
		CBasicSendBuffer* pSendBuffer = m_sQueue.front();
		long lThisCopy = pSendBuffer->ReadBuffer(pBuffer, lLength);
		if(pSendBuffer->IsEmpty())
		{
			m_sQueue.pop_front();
			CBasicSendBuffer::DeleteSendBuffer(pSendBuffer);
		}
		if(lThisCopy <= 0)
		{
			break;
		}
		lCopy += lThisCopy;

		pBuffer += lThisCopy;
		lLength -= lThisCopy;
		if(lLength <= 0)
		{
			break;
		}
	}
	return lCopy;
}

long CBasicSendBufferQueue::CopyWaitQueue()
{
	if(m_lSendLimit <= 0)
	{
		return 0;
	}

	long lCopy = m_lSendLimit;
	CSingleLock lock(&m_cs);
	lock.Lock();
	while(!m_wQueue.empty())
	{
		CBasicSendBuffer* pSendBuffer = m_wQueue.front();
		CBasicSendBuffer* pNewSendBuffer = NULL;
		long lRet = pSendBuffer->CopyTo(pNewSendBuffer, lCopy);
		if(lRet & SB_COPY_POP)
		{
			m_wQueue.pop_front();
		}
		if(lRet & SB_COPY_DELETE)
		{
			CBasicSendBuffer::DeleteSendBuffer(pSendBuffer);
		}
		if(pNewSendBuffer != NULL)
		{
			m_sQueue.push_back(pNewSendBuffer);
		}
		if(lCopy <= 0)
		{
			break;
		}
	}
	return m_lSendLimit - lCopy;
}

long CBasicSendBufferQueue::EmptySendBuffer()
{
	CSingleLock lock(&m_cs);
	lock.Lock();
	_EmptySendBuffer(m_sQueue);
	_EmptySendBuffer(m_wQueue);
	return BASIC_NET_OK;
}

void CBasicSendBufferQueue::_EmptySendBuffer(send_buffer_queue& Queue)
{
	while(!Queue.empty())
	{
		CBasicSendBuffer* pSendBuffer = Queue.front();
		Queue.pop_front();
		CBasicSendBuffer::DeleteSendBuffer(pSendBuffer);
	}
}


void CBasicSendBufferQueue::ExamSendBuffer()
{
	if(m_lMaxLivingTime <= 0)
	{
		return;
	}
	_ExamSendBuffer(m_sQueue);
	_ExamSendBuffer(m_wQueue);
}

void CBasicSendBufferQueue::_ExamSendBuffer(send_buffer_queue& Queue)
{
	send_buffer_queue::iterator iter = Queue.begin();
	send_buffer_queue::iterator iEnd = Queue.end();
	for(;iter != iEnd;)
	{
		send_buffer_queue::value_type pSendBuffer = (*iter);
		if(pSendBuffer->IsCanIgore(m_lMaxLivingTime))
		{
			iter = Queue.erase(iter);
			CBasicSendBuffer::DeleteSendBuffer(pSendBuffer);
		}
		else
		{
			iter++;
		}
	}
}

BOOL CBasicSendBufferQueue::IsEmpty()
{
	CSingleLock lock(&m_cs);
	lock.Lock();
	return m_sQueue.empty() && m_wQueue.empty();
}

long CBasicSendBufferQueue::GetSendBufferMemory()
{
	CSingleLock lock(&m_cs);
	lock.Lock();
	long lTotalSize = 0;
	_GetSendBufferMemory(m_sQueue, lTotalSize);
	_GetSendBufferMemory(m_wQueue, lTotalSize);
	return lTotalSize;
}

void CBasicSendBufferQueue::_GetSendBufferMemory(send_buffer_queue& Queue, long& lTotalSize)
{
	send_buffer_queue::iterator pSendBuffer = Queue.begin();
	send_buffer_queue::iterator iEnd = Queue.end();
	for(;pSendBuffer != iEnd; pSendBuffer++)
	{
		lTotalSize += (*pSendBuffer)->GetLength();
	}
}

long CBasicSendBufferQueue::GetSendBufferCount()
{
	CSingleLock lock(&m_cs);
	lock.Lock();
	return m_sQueue.size() + m_wQueue.size();
}

Net_Int CBasicSendBufferQueue::m_lMaxLivingTime = 0;
Net_Int CBasicSendBufferQueue::m_lMaxPacket = 0;

__NS_BASIC_END
