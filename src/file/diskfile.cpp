#include "../inc/basic.h"
#include "filebase.h"
#include "file_linux.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SF_RW_MAX_SIZE		(10 * 1024 * 1024)		//一次写入(读出)的最大长度，超出这个长度要分批处理。
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
extern long GetFileErrorID();

CDiskFile::CDiskFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

CDiskFile::~CDiskFile()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		Close();
	}
}


long CDiskFile::Open(const char* lpszFileName, DWORD nOpenFlags)
{
	// shouldn't open an already open file (it will leak)
	ASSERT(m_hFile == INVALID_HANDLE_VALUE);
	if (lpszFileName == NULL || lpszFileName[0] == '\0')
	{
		return BASIC_FILE_BAD_PATH;
	}

	DWORD dwAccess = 0;
	if (nOpenFlags & PF_READ_ONLY)
	{
		dwAccess = GENERIC_READ;
	}
	else
	{
		dwAccess = GENERIC_READ | GENERIC_WRITE;
	}

	SECURITY_ATTRIBUTES sa;
#ifdef __BASICWINDOWS
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;
#endif
	// map creation flags
	DWORD dwCreateFlag = 0;
	if ((nOpenFlags & PF_OPEN_ONLY) || (nOpenFlags & PF_READ_ONLY))
	{
		dwCreateFlag = OPEN_EXISTING;
	}
	else if (nOpenFlags & PF_OPEN_TRUN)
	{
		dwCreateFlag = CREATE_ALWAYS;
	}
	else if (nOpenFlags & PF_CREATE_ONLY)
	{
		dwCreateFlag = CREATE_NEW;
	}
	else
	{
		dwCreateFlag = OPEN_ALWAYS;
	}

	// attempt file creation
	HANDLE hFile = ::CreateFileA(lpszFileName, dwAccess, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL); //I know this
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return GetFileErrorID();
	}
	m_hFile = hFile;

	return BASIC_FILE_OK;
}

long CDiskFile::OpenMemFile(void* pBuffer, long lCount, long lLength)
{
	return BASIC_FILE_NOT_SUPPORT;
}

long CDiskFile::Close()
{
	long lRet = BASIC_FILE_OK;
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		if (!::CloseHandle(m_hFile))		//I know this
		{
			lRet = GetFileErrorID();
		}
		m_hFile = INVALID_HANDLE_VALUE;
	}
	return lRet;
}

bool CDiskFile::IsOpen() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}


extern void FillFileStatusTime(TLFileStatus& rStatus, const FILETIME& ftCreate, const FILETIME& ftAccess, const FILETIME& ftModify);
long CDiskFile::GetStatus(TLFileStatus& rStatus) const
{
	long lLength = GetLength();
	if (lLength < 0)
	{
		return lLength;
	}

	rStatus.m_size = lLength;

	// get time current file size
	FILETIME ftCreate, ftAccess, ftModify;
	if (!::GetFileTime(m_hFile, &ftCreate, &ftAccess, &ftModify))   //I know this
	{
		return GetFileErrorID();
	}

	FillFileStatusTime(rStatus, ftCreate, ftAccess, ftModify);

	return BASIC_FILE_OK;
}

long CDiskFile::GetPosition() const
{
	return _seek(0, BASIC_FILE_CURRENT);
}

long CDiskFile::Seek(long lOff, DWORD nFrom)
{
	return _seek(lOff, nFrom);
}

long CDiskFile::_seek(long lOff, DWORD nFrom) const
{
	assert(m_hFile != INVALID_HANDLE_VALUE);
	long lPos = (long)::SetFilePointer(m_hFile, lOff, NULL, nFrom); //I know this
	if (lPos < 0)
	{
		if (::GetLastError() != NO_ERROR) //I know this
		{
			return GetFileErrorID();
		}
		else
		{
			return BASIC_FILE_TOO_LARGE;
		}
	}

	return lPos;
}

long CDiskFile::SetLength(long lNewLen, char cFill)
{
	assert(m_hFile != INVALID_HANDLE_VALUE);

	long lOldLen = Seek(0, BASIC_FILE_END);
	if (lOldLen < 0)
	{
		return lOldLen;
	}
	if (lNewLen > lOldLen && cFill != 0)
	{
		Write(NULL, lNewLen - lOldLen, 1, cFill);
	}
	else
	{
		Seek(lNewLen, BASIC_FILE_BEGIN);
		if (!::SetEndOfFile(m_hFile))  //I know this
		{
			return GetFileErrorID();
		}
	}
	Flush();
	return lNewLen;
}

long CDiskFile::GetLength() const
{
	DWORD dwHigh = 0;
	long lLength = (long)::GetFileSize(m_hFile, &dwHigh);  //I know this
	if (dwHigh != 0)
	{
		return BASIC_FILE_TOO_LARGE;
	}
	if (lLength < 0)
	{
		if (::GetLastError() != NO_ERROR)  //I know this
		{
			return GetFileErrorID();
		}
		else
		{
			return BASIC_FILE_TOO_LARGE;
		}
	}
	return lLength;
}

long CDiskFile::Read(void* lpBuf, long lCount)
{
	assert(m_hFile != INVALID_HANDLE_VALUE);
	assert(lpBuf != NULL);

	if (lCount <= 0)
	{
		return 0;   // avoid Win32 "null-read"
	}

	DWORD dwRead = 0;
	if (!::ReadFile(m_hFile, lpBuf, lCount, &dwRead, NULL))  //I know this
	{
		return GetFileErrorID();
	}

	return (long)dwRead;
}

long CDiskFile::Write(const void* lpBuf, long lCount, long lRepeat, char cFill)
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

	char* lpTemp = (char*)lpBuf;
	long lLeft = 0;
	if (lpTemp == NULL)
	{
		if (lCount > SF_RW_MAX_SIZE)	//如果大于这个长度，分开写
		{
			lCount *= lRepeat;
			lRepeat = lCount / SF_RW_MAX_SIZE;
			lLeft = lCount % SF_RW_MAX_SIZE;
			lCount = SF_RW_MAX_SIZE;
		}
		lpTemp = (char*)BasicAllocate(lCount);
		memset(lpTemp, cFill, lCount);
	}

	long lWritten = 0;
	for (int i = 0; i < lRepeat; i++)
	{
		lWritten = WriteToFile(lpTemp, lCount);
		if (lWritten <= 0)
		{
			break;
		}
	}
	if (lLeft > 0 && lWritten > 0)
	{
		lWritten = WriteToFile(lpTemp, lLeft);
	}
	if (lpBuf == NULL)
	{
		BasicDeallocate(lpTemp);
	}
	if (lWritten <= 0)
	{
		return lWritten;
	}
	return lTotal;
}

long CDiskFile::WriteToFile(const void* lpBuf, long lCount)
{
	assert(m_hFile != INVALID_HANDLE_VALUE);
	assert(lpBuf != NULL);

	if (lCount <= 0)
	{
		return 0;
	}

	DWORD nWritten = 0;
	if (!::WriteFile(m_hFile, lpBuf, lCount, &nWritten, NULL))  //I know this
	{
		return GetFileErrorID();
	}

	if ((long)nWritten != lCount)
	{
		return BASIC_FILE_DISK_FULL;
	}
	return lCount;
}

long CDiskFile::Flush()
{
	assert(m_hFile != INVALID_HANDLE_VALUE);

	if (!::FlushFileBuffers(m_hFile))   //I know this
	{
		return GetFileErrorID();
	}
	return BASIC_FILE_OK;
}

CFileBase* CDiskFile::Duplicate() const
{
	assert(m_hFile != INVALID_HANDLE_VALUE);

	CDiskFile* pFile = NULL;
#ifdef __BASICWINDOWS
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (::DuplicateHandle(::GetCurrentProcess(), (HANDLE)m_hFile, ::GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS) && hFile != INVALID_HANDLE_VALUE) //I know this
	{
		pFile = new CDiskFile();
		pFile->m_hFile = hFile;
		return pFile;
	}
#endif
	return pFile;
}

void* CDiskFile::GetDataBuffer()
{
	return NULL;
}

void* CDiskFile::GetCurDataBuffer(long& lCount)
{
	return NULL;
}

long CDiskFile::CopyTo(const char* lpszThisFileName, const char* lpszFileName)
{
	if (STR_INVALID(lpszFileName))
	{
		return BASIC_FILE_BAD_PATH;
	}
	return Basic_CopyFile(lpszThisFileName, lpszFileName, false);
}

long CDiskFile::CopyFrom(const char* lpszThisFileName, const char* lpszFileName)
{
	if (STR_INVALID(lpszFileName))
	{
		return BASIC_FILE_BAD_PATH;
	}
	long lRet = Basic_CopyFile(lpszFileName, lpszThisFileName, false);
	Seek(0, BASIC_FILE_BEGIN);
	return lRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////

