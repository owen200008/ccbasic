#include "../inc/basic.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START


#define GROW_SIZE		1024

#define GROW_MODE_NORMAL		0			//默认的分配策略
#define GROW_MODE_FAST			-1			//快速增长的分配策略


inline long _GetGrowLength(long lOldLength, long lLength, long lGrowMode)
{
	if (lGrowMode == GROW_MODE_NORMAL)
	{
		return ((lOldLength + lLength - 1) / GROW_SIZE + 1) * GROW_SIZE;
	}
	else if (lGrowMode == GROW_MODE_FAST)
	{
		long lStep = MAX(lOldLength + lLength, 2 * lOldLength);
		return ((lStep - 1) / GROW_SIZE + 1) * GROW_SIZE;
	}
	return lLength;
}

CBasicSmartBuffer::CBasicSmartBuffer()
{
	EmptyBuffer();
}

CBasicSmartBuffer::CBasicSmartBuffer(const basiclib::CBasicSmartBuffer &buffer)
{
	EmptyBuffer();
	Free();
	AppendData(buffer.m_pszBuffer, buffer.m_cbBuffer);
}

CBasicSmartBuffer::~CBasicSmartBuffer()
{
	Free();
}

void CBasicSmartBuffer::EmptyBuffer()
{
	m_bSelfBuf = true;
	m_pszBuffer = NULL;
	m_cbAlloc = 0;
	m_cbBuffer = 0;
}

void CBasicSmartBuffer::Free()
{
	if (m_bSelfBuf)
	{
		if (m_pszBuffer)
		{
			BasicDeallocate(m_pszBuffer, m_cbAlloc);
		}
	}
	EmptyBuffer();
}

bool CBasicSmartBuffer::ExportOutData(SmartBufferExportOutData& data)
{
	if (m_bSelfBuf)
	{
		data.ReleaseData();
		data.m_pExport = m_pszBuffer;
		data.m_nLength = m_cbBuffer;
		return true;
	}
	return false;
}

void CBasicSmartBuffer::SetDataLength(long lLength)
{
	if (lLength < 0)
	{
		//这里以后可能作为特殊参数处理
	}
	else
	{
		long lAddLength = lLength - m_cbBuffer;
		if (lAddLength > 0)
		{
			if (AllocBuffer(lAddLength) == NULL)
			{
				return;
			}
		}
		m_cbBuffer = lLength;
	}
}


char* CBasicSmartBuffer::AppendData(const char* pszData, long lLength)
{
	char* pBuffer = AllocBuffer(lLength);
	if (pBuffer != NULL && pszData != NULL)
	{
		memcpy(pBuffer, pszData, lLength);
		m_cbBuffer += lLength;
	}
	return pBuffer;
}

//用快速增长分配策略
char* CBasicSmartBuffer::AppendDataEx(const char* pszData, long lLength)
{
	char* pBuffer = AllocBuffer(lLength, GROW_MODE_FAST);
	if (pBuffer != NULL && pszData != NULL)
	{
		memcpy(pBuffer, pszData, lLength);
		m_cbBuffer += lLength;
	}
	return pBuffer;
}

char* CBasicSmartBuffer::CommitData(long lLength)
{
	char* pBuffer = AllocBuffer(lLength, GROW_MODE_FAST);
	if (pBuffer != NULL)
	{
		m_cbBuffer += lLength;
	}
	return pBuffer;
}

void CBasicSmartBuffer::AppendString(const char* lpszText)
{
	if (lpszText != NULL && lpszText[0] != '\0')
	{
		long lLength = strlen(lpszText) * sizeof(char);
		AllocBuffer(lLength + sizeof(char), GROW_MODE_FAST);	//多分配一个字符
		AppendData((const char*)lpszText, lLength);
	}
}

void CBasicSmartBuffer::AppendData(long lVal)
{
	char szBuf[64];
	sprintf(szBuf, "%d", lVal);
	AppendString(szBuf);
}
void CBasicSmartBuffer::AppendData(double fVal)
{
	char szBuf[64];
	sprintf(szBuf, "%f", fVal);
	AppendString(szBuf);
}

char* CBasicSmartBuffer::AllocBuffer(long lLength, long lGrowLength)
{
	if (m_cbBuffer + lLength > m_cbAlloc)
	{
		long lNewSize = _GetGrowLength(m_cbBuffer, lLength, lGrowLength);

		char* pTemp = (char*)BasicAllocate(lNewSize);
		if (pTemp == NULL)
		{
			return NULL;
		}
		memset(pTemp, 0, lNewSize);
		if (m_pszBuffer != NULL)
		{
			memcpy(pTemp, m_pszBuffer, m_cbBuffer);
			if (m_bSelfBuf)
			{
				BasicDeallocate(m_pszBuffer, m_cbAlloc);
			}
		}
		m_cbAlloc = lNewSize;
		m_pszBuffer = pTemp;
		m_bSelfBuf = true;
	}
	return &m_pszBuffer[m_cbBuffer];
}


BOOL CBasicSmartBuffer::InitFormFile(const char* lpszFile)
{
	BOOL bRet = FALSE;
	FILE* fp = fopen(lpszFile, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long lSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		SetDataLength(lSize);
		long lRet = fread(GetDataBuffer(), sizeof(char), lSize, fp);
		if (lRet == lSize)
		{
			bRet = TRUE;
		}
		else
		{
			SetDataLength(0);
		}
		fclose(fp);
	}
	return bRet;
}

CBasicSmartBuffer& CBasicSmartBuffer::operator = (const CBasicSmartBuffer& buffer)
{
	Free();
	AppendData(buffer.m_pszBuffer, buffer.m_cbBuffer);
	return *this;
}

char* CBasicSmartBuffer::GetDataBuffer(long& lLength) const
{
	lLength = m_cbBuffer;
	return m_pszBuffer;
}

bool operator == (CBasicSmartBuffer& b1, CBasicSmartBuffer& b2)
{
	if (b1.GetDataLength() != b2.GetDataLength())
		return false;

	return memcmp(b1.GetDataBuffer(), b2.GetDataBuffer(), b1.GetDataLength()) == 0;
}
//读取数据
void CBasicSmartBuffer::ReadData(void* pBuffer, int nLength)
{
	if (m_cbBuffer < nLength || nLength <= 0)
	{
		ASSERT(FALSE);
		return;
	}
	if (pBuffer)
		memcpy(pBuffer, m_pszBuffer, nLength);
	m_cbBuffer -= nLength;
	memmove(m_pszBuffer, m_pszBuffer + nLength, m_cbBuffer);
}
//binddatatosmartbuffer
void CBasicSmartBuffer::BindOutData(char* pData, int nLength)
{
	Free();
	m_bSelfBuf = false;
	m_pszBuffer = pData;
	m_cbAlloc = nLength;
	m_cbBuffer = m_cbAlloc;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END
