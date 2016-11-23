#include "../inc/basic.h"

__NS_BASIC_START

BOOL BasicStaticBuffer::IsEmpty() const
{
	return m_pBuffer == NULL || m_nLength == 0;
}

void* BasicStaticBuffer::Alloc(unsigned short len)
{
	if (len <= m_nAlloc)
	{
		m_nLength = len;
	}
	else
	{
		Release();
		AllocBuffer(len);
		m_nLength = len;
	}
	return m_pBuffer;
}

#define GROW_SIZE		1024
void* BasicStaticBuffer::AllocBuffer(unsigned short len)
{
	long lNewSize = m_nLength + len; //((m_nLength + len - 1) / GROW_SIZE + 1) * GROW_SIZE;
	if(lNewSize > m_nAlloc)
	{
		char* pTemp = (char*)BasicAllocate(lNewSize);
		memset(pTemp, 0, lNewSize);
		if(m_pBuffer != NULL)
		{
			memcpy(pTemp, m_pBuffer, m_nLength);
			BasicDeallocate(m_pBuffer);
		}
		m_nAlloc = (unsigned short)lNewSize;
		m_pBuffer = pTemp; 
	}
	return (char*)m_pBuffer + m_nLength;

}

BOOL BasicStaticBuffer::Assign(unsigned short len, unsigned char c)
{
	Release();
	void* pBuffer = Alloc(len);
	if (pBuffer != NULL)
	{
		memset(pBuffer, c, len);
		m_nLength = len;
		return TRUE;
	}
	return FALSE;
}

void BasicStaticBuffer::Release()
{
	if (m_pBuffer)
	{
		BasicDeallocate(m_pBuffer);
		m_pBuffer = NULL;
	}
	m_nLength = 0;
	m_nAlloc  = 0;
}

void* BasicStaticBuffer::AppendBuffer(const void* buf, unsigned short len)
{
	void* pBuffer = AllocBuffer(len);
	if(pBuffer != NULL && buf != NULL)
	{
		memcpy(pBuffer, buf, len);
		m_nLength += len;
	}
	return m_pBuffer;
}

void BasicStaticBuffer::Clone(const BasicStaticBuffer& buffer)
{
	if(this != &buffer)
	{
		Release();
		AppendBuffer(buffer.m_pBuffer, buffer.m_nLength);
	}
}

void* BasicStaticBuffer::AppendString(const char* str, unsigned short len)
{
	if(str != NULL && str[0] != '\0')
	{
		if (len == (unsigned short)-1)
		{
			len = __tcslen(str);
		}
		unsigned short nLength = (len + 1)* sizeof(char);
		AppendBuffer(str, nLength);
		m_nLength -= sizeof(char);
	}
	return m_pBuffer;
}

void* BasicStaticBuffer::GetBuffer() const
{
	return m_pBuffer;
}

size_t BasicStaticBuffer::GetLength() const
{
	return m_nLength;
}

const char* BasicStaticBuffer::GetString() const
{
	return (NULL == m_pBuffer) ? Null_String_S : (const char*)m_pBuffer;
}

void* BasicStaticBuffer::SetLength(unsigned short len)
{
	if (len <= m_nLength)
	{
		m_nLength = len;
	}
	else
	{
		unsigned short lAdd = len - m_nLength;
		AllocBuffer(lAdd);
		m_nLength = len;
	}

	return m_pBuffer;
}

CBasicStaticBuffer::CBasicStaticBuffer()
{
	memset(this, 0, sizeof(CBasicStaticBuffer));

}

CBasicStaticBuffer::~CBasicStaticBuffer(void)
{
	Release();
}

void CBasicStaticBuffer::Release()
{
	if (m_pBuffer)
	{
		BasicDeallocate(m_pBuffer);
		m_pBuffer = NULL;
	}
	m_lLength = 0;
}
BOOL CBasicStaticBuffer::IsEmpty() const
{
	return (NULL == m_pBuffer) || (0 == m_lLength);
}

void* CBasicStaticBuffer::Alloc(size_t len)
{
	if (NULL == m_pBuffer || len > m_lLength)
	{
		Release();
		m_pBuffer = BasicAllocate(len);
	}
	if (m_pBuffer)
		m_lLength = len;
	return m_pBuffer;
}

void* CBasicStaticBuffer::SetLength(size_t len)
{
	if (m_lLength < len)
	{
		void* pBuffer = BasicAllocate(len);
		if (!IsEmpty())
		{
			memcpy(pBuffer, m_pBuffer, m_lLength);
		}
		Release();
		m_pBuffer = pBuffer;
	}
	m_lLength = len;
	return m_pBuffer;
}

void* CBasicStaticBuffer::AppendBuffer(const void* buf, size_t len)
{
	if (NULL == buf || 0 == len)
		return m_pBuffer;

	size_t offset = m_lLength;
	void* pBuffer = SetLength(m_lLength + len);
	if (pBuffer != NULL)
	{
		memcpy((char*)pBuffer + offset, buf, len);
	}
	return m_pBuffer;
}

void CBasicStaticBuffer::Clone(const CBasicStaticBuffer& buffer)
{
	if (this != &buffer)
	{
		Release();
		AppendBuffer(buffer.m_pBuffer, buffer.m_lLength);
	}
}

void* CBasicStaticBuffer::AppendString(const char* str, size_t len)
{
	if (str != NULL && str[0] != '\0')
	{
		if (len == NullLen)
		{
			len = strlen(str);
		}
		long lLength = (len + 1)* sizeof(char);
		AppendBuffer(str, lLength);
		m_lLength -= sizeof(char);
	}
	return m_pBuffer;
}

BOOL CBasicStaticBuffer::Assign(size_t len, unsigned char c)
{
	if (Alloc(len))
	{
		memset(m_pBuffer, c, len);
		return TRUE;
	}
	return FALSE;
}

void* CBasicStaticBuffer::GetBuffer() const
{
	return m_pBuffer;
}

size_t CBasicStaticBuffer::GetLength() const
{
	return m_lLength;
}

const char* CBasicStaticBuffer::GetString() const
{
	return (const char*)m_pBuffer;
}

BOOL CBasicStaticBuffer::FromFile(const char* lpszFile)
{
	FILE* fp = fopen(lpszFile, "rb");
	if (fp)
	{
		Release();
		fseek(fp, 0, SEEK_END);
		long nFileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* pReadBuffer = (char*)Alloc(nFileLen + 1);
		if (pReadBuffer != NULL)
		{
			int nReadLen = fread(pReadBuffer, sizeof(char), nFileLen, fp);
			if (nReadLen == nFileLen)
			{
				pReadBuffer[nFileLen] = 0;
				SetLength(nFileLen);
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CBasicStaticBuffer::Attach(void* buffer, size_t length)
{
	Release();
	m_pBuffer = buffer;
	m_lLength = length;
}
void CBasicStaticBuffer::Detach()
{
	m_pBuffer = NULL;
	m_lLength = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END
