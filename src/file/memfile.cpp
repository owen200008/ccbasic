#include "../inc/basic.h"
#include "filebase.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MF_HEAD			sizeof(long) * 3		//数据头的长度

#define MF_LEN			0						//数据长度
#define MF_POS			1						//当前位置
#define MF_MAP			2						//MAPFILE 句柄
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START

CMemFileBase::CMemFileBase()
{
	ClearMember();
}

CMemFileBase::~CMemFileBase()
{
	Close();
}

long CMemFileBase::Open(const char* lpszFileName, DWORD nOpenFlags)
{
	ASSERT(m_pFileObj == NULL);
	if (lpszFileName != NULL && lpszFileName[0] != '\0')
	{
		CDiskFile diskFile;
		if (diskFile.Open(lpszFileName, PF_OPEN_ONLY | PF_READ_ONLY) == BASIC_FILE_OK)
		{
			long lLength = diskFile.GetLength();
			if (lLength > 0)
			{
				char* pBuffer = _AllocMemory(lLength);
				if (pBuffer == NULL)
				{
					diskFile.Close();
					return BASIC_FILE_NO_MEMORY;
				}
				diskFile.Read(pBuffer, lLength);
			}
			diskFile.Close();
		}
	}
	if (!IsOpen())
	{
		_AllocMemory(0);
	}
	return BASIC_FILE_OK;
}

long CMemFileBase::OpenMemFile(void* pBuffer, long lCount, long lLength)
{
	ASSERT(m_pFileObj == NULL);
	if (lLength == -1)
	{
		return BASIC_FILE_OK;
	}
	else if (lLength == -2)	//传进来的是对象
	{
		CFileBase* pFileBase = (CFileBase*)pBuffer;
		lCount = pFileBase->GetLength();
		if (lCount > 0)
		{
			pBuffer = _AllocMemory(lCount);
			if (pBuffer != NULL)
			{
				pFileBase->Seek(0, FILE_BEGIN);
				pFileBase->Read(pBuffer, lCount);
			}
			else
			{
				return BASIC_FILE_NO_MEMORY;
			}
		}
		else
		{
			_AllocMemory(0);
		}
	}
	else
	{
		long lSize = lCount > lLength ? lCount : lLength;

		char* pDataBuffer = _AllocMemory(lSize);
		if (pDataBuffer != NULL)
		{
			if (pBuffer != NULL && lCount > 0)
			{
				memcpy(pDataBuffer, pBuffer, lCount);
			}
		}
		else
		{
			return BASIC_FILE_NO_MEMORY;
		}
	}

	return BASIC_FILE_OK;
}

long CMemFileBase::Close()
{
	FreeMemory();
	return BASIC_FILE_OK;
}

bool CMemFileBase::IsOpen() const
{
	return m_pFileObj != NULL;
}

long CMemFileBase::GetStatus(TLFileStatus& rStatus) const
{
	rStatus.m_size = GetLength();
	return BASIC_FILE_OK;
}

long CMemFileBase::Read(void* lpBuf, long lCount)
{
	char* pBuffer = (char*)GetCurDataBuffer(lCount);
	if (pBuffer != NULL)
	{
		memcpy(lpBuf, pBuffer, lCount);
		Seek(lCount, BASIC_FILE_CURRENT);
		return lCount;
	}
	return 0;
}

//内部使用的 memmove 函数，解决地址重叠的问题
static char* _basic_memmove(char* dst, const char* src, size_t count)
{
	char* ret = dst;
	if (dst <= src || dst >= (src + count))
	{
		/*
		* Non-Overlapping Buffers
		* copy from lower addresses to higher addresses
		*/
		while (count--)
		{
			*dst = *src;
			dst = dst + 1;
			src = src + 1;
		}
	}
	else
	{
		/*
		* Overlapping Buffers
		* copy from higher addresses to lower addresses
		*/
		dst = dst + count - 1;
		src = src + count - 1;

		while (count--)
		{
			*dst = *src;
			dst = dst - 1;
			src = src - 1;
		}
	}
	return(ret);
}


long CMemFileBase::Write(const void* lpBuf, long lCount, long lRepeat, char cFill)
{
	assert(lRepeat > 0);
	if (lCount <= 0)
	{
		return 0;
	}

	long lTotal = lCount * lRepeat;
	if (lTotal <= 0)
	{
		return BASIC_FILE_TOO_LARGE;
	}

	long lPosition = GetPosition();
	long lLength = GetLength();

	long lNewLen = lPosition + lTotal;
	if (lNewLen > lLength)
	{
		long lRet = SetLength(lNewLen, cFill);
		if (lRet != BASIC_FILE_OK)
		{
			return lRet;
		}
	}

	long lWrite = 0;
	char* pBuffer = _GetDataBuffer();
	if (lpBuf != NULL)
	{
		for (int i = 0; i < lRepeat; i++)
		{
			_basic_memmove(&pBuffer[lPosition], (const char*)lpBuf, lCount);
			lPosition += lCount;
			lWrite += lCount;
		}
	}
	else
	{
		lWrite = lTotal;
		memset(&pBuffer[lPosition], cFill, lWrite);
		lPosition += lWrite;
	}
	_SetDataHead(MF_POS, lPosition);
	return lWrite;
}

long CMemFileBase::Flush()
{
	return BASIC_FILE_OK;
}

CFileBase* CMemFileBase::Duplicate() const
{
	CMemFileBase* pFile = new CMemFileBase();		//Does not require this buffer to release
	long lLength = GetLength();
	if (lLength > 0)
	{
		char* pBuffer = _GetDataBuffer();
		pFile->OpenMemFile(pBuffer, lLength, 0);
		pFile->Seek(GetPosition(), BASIC_FILE_BEGIN);
	}
	return pFile;
}

void* CMemFileBase::GetDataBuffer()
{
	return _GetDataBuffer();
}

void* CMemFileBase::GetCurDataBuffer(long& lCount)
{
	if (lCount > 0)
	{
		long lPosition = GetPosition();
		if (lPosition >= 0)
		{
			long lDataLength = GetLength() - lPosition;
			if (lDataLength > 0)
			{
				if (lCount > lDataLength)
				{
					lCount = lDataLength;
				}
				char* pBuffer = _GetDataBuffer();
				return &pBuffer[lPosition];
			}
		}
	}
	return NULL;
}


long CMemFileBase::CopyTo(const char* lpszThisFileName, const char* lpszFileName)
{
	if (STR_INVALID(lpszFileName))
	{
		lpszFileName = lpszThisFileName;
		if (STR_INVALID(lpszFileName))
		{
			return BASIC_FILE_BAD_PATH;
		}
	}
	CDiskFile diskFile;
	long lRet = diskFile.Open(lpszFileName, PF_DISK_FILE);
	if (lRet != BASIC_FILE_OK)
	{
		return lRet;
	}
	char* pBuffer = _GetDataBuffer();
	if (pBuffer != NULL)
	{
		long lLength = GetLength();
		long lWrite = diskFile.Write(pBuffer, lLength, 1, 0);
		if (lWrite < 0)
		{
			lRet = lWrite;
		}
		else
		{
			diskFile.SetLength(lLength, 0);
		}
	}
	return lRet;
}

long CMemFileBase::CopyFrom(const char* lpszThisFileName, const char* lpszFileName)
{
	if (STR_INVALID(lpszFileName))
	{
		lpszFileName = lpszThisFileName;
		if (STR_INVALID(lpszFileName))
		{
			return BASIC_FILE_BAD_PATH;
		}
	}
	Close();
	return Open(lpszFileName, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
inline char* mf_getdatabuffer(char* p)			//I know that the length of this buffer
{
	return p == NULL ? NULL : (p + MF_HEAD);
}
inline long mf_getdatahead(char* p, int n)
{
	return p == NULL ? 0 : ((long*)p)[n];
}

void inline mf_setdatahead(char* p, int n, long l)
{
	((long*)p)[n] = l;
}


long CMemFileBase::GetPosition() const
{
	return _GetDataHead(MF_POS);
}

long CMemFileBase::Seek(long lOff, DWORD nFrom)
{
	long lPosition = 0;
	switch (nFrom)
	{
	case BASIC_FILE_BEGIN:
	{
							 lPosition = lOff;
							 break;
	}
	case BASIC_FILE_CURRENT:
	{
							   lPosition = GetPosition() + lOff;
							   break;
	}
	case BASIC_FILE_END:
	{
						   lPosition = GetLength() + lOff;
						   break;
	}
	default:
	{
			   return GetPosition();
	}
	}
	_SetDataHead(MF_POS, lPosition);
	return lPosition;
}

long CMemFileBase::GetLength() const
{
	return _GetDataHead(MF_LEN);
}

long CMemFileBase::SetLength(long lNewLen, char cFill)
{
	long lOldLength = GetLength();
	char* pBuffer = _AllocMemory(lNewLen);
	if (pBuffer == NULL)
	{
		return BASIC_FILE_NO_MEMORY;
	}
	if (cFill != 0)
	{
		long lNewLength = GetLength();
		if (lNewLength > lOldLength)
		{
			memset(&pBuffer[lOldLength], cFill, lNewLength - lOldLength);
		}
	}
	return BASIC_FILE_OK;
}

char* CMemFileBase::_GetDataBuffer() const
{
	return mf_getdatabuffer(m_pFileObj);
}

long CMemFileBase::_GetDataHead(int nIndex) const
{
	return mf_getdatahead(m_pFileObj, nIndex);
}

void CMemFileBase::_SetDataHead(int nIndex, long lValue)
{
	mf_setdatahead(m_pFileObj, nIndex, lValue);
}

#define MF_MEM_MAX_SIZE		(40 * 1024 * 1024)		//纯内存文件的最大长度，超出这个长度，需要使用 MAPFILE
char* CMemFileBase::_AllocMemory(long lLength)
{
	long lOldLength = _GetDataHead(MF_LEN);
	if (lOldLength == 0 || lOldLength != lLength)
	{
		char* pBuffer = NULL;
		long lNewLength = lLength + MF_HEAD;
#ifdef __BASICWINDOWS
		if (lNewLength > MF_MEM_MAX_SIZE)
		{
			HANDLE hMapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, (LPSECURITY_ATTRIBUTES)NULL, PAGE_READWRITE, 0, lNewLength, NULL); //I know this
			if (hMapHandle != NULL)
			{
				pBuffer = (char*)MapViewOfFile(hMapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);		//I know this
				if (pBuffer != NULL)
				{
					memset(pBuffer, 0, lNewLength);
					mf_setdatahead(pBuffer, MF_MAP, (long)hMapHandle);
				}
			}
		}
		if (pBuffer == NULL)
#endif
		{
			pBuffer = (char*)BasicAllocate(lNewLength);
			if (pBuffer != NULL)
			{
				memset(pBuffer, 0, lNewLength);
			}
		}
		if (pBuffer != NULL)
		{
			mf_setdatahead(pBuffer, MF_LEN, lLength);
			if (m_pFileObj != NULL)
			{
				long lPosition = _GetDataHead(MF_POS);
				if (lPosition > lLength)
				{
					lPosition = lLength;
				}
				mf_setdatahead(pBuffer, MF_POS, lPosition);

				char* pOldBuffer = _GetDataBuffer();
				char* pNewBuffer = mf_getdatabuffer(pBuffer);
				long lCopy = lOldLength > lLength ? lLength : lOldLength;
				memcpy(pNewBuffer, pOldBuffer, lCopy);

				FreeMemory();
			}
		}
		m_pFileObj = pBuffer;
	}
	return _GetDataBuffer();
}

void CMemFileBase::FreeMemory()
{
	if (m_pFileObj != NULL)
	{
#ifdef __BASICWINDOWS
		HANDLE hMapHandle = (HANDLE)_GetDataHead(MF_MAP);
		if (hMapHandle != NULL)
		{
			UnmapViewOfFile(m_pFileObj);		//I know this
			CloseHandle(hMapHandle);			//I know this
		}
		else
#endif
		{
			BasicDeallocate(m_pFileObj);
		}
		m_pFileObj = NULL;
	}
}

void CMemFileBase::ClearMember()
{
	m_pFileObj = NULL;
}

__NS_BASIC_END
