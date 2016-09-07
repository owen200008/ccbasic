#include "../inc/basic.h"

#ifdef __BASICWINDOWS
#include <shlwapi.h>
#include <time.h>
#else
#include <dirent.h>

#include "../../def/file/file_linux.h"
#ifndef LPWIN32_FIND_DATAA
#define LPWIN32_FIND_DATAA LPWIN32_FIND_DATA
#endif

#ifndef WIN32_FIND_DATAA
#define WIN32_FIND_DATAA WIN32_FIND_DATA
#endif

#ifndef FindFirstFileA
#define FindFirstFileA FindFirstFile
#endif


#ifndef FindNextFileA
#define FindNextFileA FindNextFile
#endif
#endif //#ifdef _WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
extern long GetFileErrorID();
extern time_t _FileTimeToTime(const FILETIME& fileTime);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicFileFind::CBasicFileFind()
{
	m_pFoundInfo = NULL;
	m_pNextInfo = NULL;
	m_hContext = NULL;
}

CBasicFileFind::~CBasicFileFind()
{
	Close();
}

void CBasicFileFind::Close()
{
	if (m_pFoundInfo != NULL)
	{
		BasicDeallocate(m_pFoundInfo);
		m_pFoundInfo = NULL;
	}

	if (m_pNextInfo != NULL)
	{
		BasicDeallocate(m_pNextInfo);
		m_pNextInfo = NULL;
	}

	if (m_hContext != NULL)
	{
		if (m_hContext != INVALID_HANDLE_VALUE)
		{
			CloseContext();
		}
		m_hContext = NULL;
	}
}

void CBasicFileFind::CloseContext()
{
	::FindClose(m_hContext);		//I know this
}

ULONGLONG CBasicFileFind::GetLength() const
{
	ULARGE_INTEGER nFileSize;

	if (m_pFoundInfo != NULL)
	{
		LPWIN32_FIND_DATAA pFindData = (LPWIN32_FIND_DATAA)m_pFoundInfo;
#ifdef __BASICWINDOWS
		nFileSize.LowPart = pFindData->nFileSizeLow;
		nFileSize.HighPart = pFindData->nFileSizeHigh;
#else
		ULONGLONG uTmp = pFindData->nFileSizeHigh;
		nFileSize.QuadPart = uTmp << 32;
		nFileSize.QuadPart += pFindData->nFileSizeLow;
#endif
	}
	else
	{
		nFileSize.QuadPart = 0;
	}

	return nFileSize.QuadPart;
}

CBasicString CBasicFileFind::GetFilePath() const
{
	return m_strRoot + GetFileName();
}

CBasicString CBasicFileFind::GetFileTitle() const
{
	CBasicString strFullName = GetFileName();

	char szTitle[MAX_PATH];
	Basic_GetFileTitle(strFullName.c_str(), szTitle, MAX_PATH);

	return szTitle;
}

CBasicString CBasicFileFind::GetFileName() const
{
	CBasicString ret;

	if (m_pFoundInfo != NULL)
	{
		ret = ((LPWIN32_FIND_DATAA)m_pFoundInfo)->cFileName;
	}
	return ret;
}

CBasicString CBasicFileFind::GetFileURL() const
{
	CBasicString strResult("file://");	//This is not prompted string
	strResult += GetFilePath();
	return strResult;
}

CBasicString CBasicFileFind::GetRoot() const
{
	return m_strRoot;
}
time_t CBasicFileFind::GetLastAccessTime() const
{
	if (m_pFoundInfo != NULL)
	{
		return _FileTimeToTime(((LPWIN32_FIND_DATAA)m_pFoundInfo)->ftLastAccessTime);
	}
	return 0;
}

time_t CBasicFileFind::GetLastWriteTime() const
{
	if (m_pFoundInfo != NULL)
	{
		return _FileTimeToTime(((LPWIN32_FIND_DATAA)m_pFoundInfo)->ftLastWriteTime);
	}
	return 0;
}

time_t CBasicFileFind::GetCreationTime() const
{
	if (m_pFoundInfo != NULL)
	{
		return _FileTimeToTime(((LPWIN32_FIND_DATAA)m_pFoundInfo)->ftCreationTime);
	}
	return 0;
}

BOOL CBasicFileFind::IsDots() const
{
	BOOL bResult = FALSE;
	if (m_pFoundInfo != NULL && IsDirectory())
	{
		LPWIN32_FIND_DATAA pFindData = (LPWIN32_FIND_DATAA)m_pFoundInfo;
		if (pFindData->cFileName[0] == '.')
		{
			if (pFindData->cFileName[1] == '\0' || (pFindData->cFileName[1] == '.' && pFindData->cFileName[2] == '\0'))
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

BOOL CBasicFileFind::IsReadOnly() const
{
	return MatchesMask(FILE_ATTRIBUTE_READONLY);
}

BOOL CBasicFileFind::IsDirectory() const
{
	return MatchesMask(FILE_ATTRIBUTE_DIRECTORY);
}

BOOL CBasicFileFind::IsCompressed() const
{
	return MatchesMask(FILE_ATTRIBUTE_COMPRESSED);
}

BOOL CBasicFileFind::IsSystem() const
{
	return MatchesMask(FILE_ATTRIBUTE_SYSTEM);
}

BOOL CBasicFileFind::IsHidden() const
{
	return MatchesMask(FILE_ATTRIBUTE_HIDDEN);
}

BOOL CBasicFileFind::IsTemporary() const
{
	return MatchesMask(FILE_ATTRIBUTE_TEMPORARY);
}

BOOL CBasicFileFind::IsNormal() const
{
	return MatchesMask(FILE_ATTRIBUTE_NORMAL);
}

BOOL CBasicFileFind::IsArchived() const
{
	return MatchesMask(FILE_ATTRIBUTE_ARCHIVE);
}

BOOL CBasicFileFind::MatchesMask(DWORD dwMask) const
{
	if (m_pFoundInfo != NULL)
	{
		return (!!(((LPWIN32_FIND_DATAA)m_pFoundInfo)->dwFileAttributes & dwMask));
	}
	return FALSE;
}
long CBasicFileFind::FindFile(const char* lpszFilePath, const char* lpszFileName)
{
	if (lpszFilePath == NULL)
	{
		return BASIC_FILE_BAD_PATH;
	}

	int nMaxLength = _countof(((WIN32_FIND_DATAA*)m_pNextInfo)->cFileName);

	int nPathLen = strlen(lpszFilePath);
	if (nPathLen >= nMaxLength)
	{
		return BASIC_FILE_BAD_PATH;
	}

	char szFullPath[MAX_PATH];
	long lRet = Basic_GetFileFullPath(szFullPath, lpszFilePath);
	if (lRet != BASIC_FILE_OK)
	{
		return lRet;
	}

	char* lpTemp = Basic_FindFileName(szFullPath);
	if (strlen(lpTemp) > 0)		//有文件名
	{
		char cTemp = *lpTemp;
		*lpTemp = '\0';
		m_strRoot = szFullPath;
		*lpTemp = cTemp;
	}
	else
	{
		m_strRoot = szFullPath;
		char szAllFile[] = "*.*";
		if (STR_INVALID(lpszFileName))
		{
			lpszFileName = szAllFile;
		}
		int nNameLen = strlen(lpszFileName);
		if ((nNameLen + nPathLen) >= nMaxLength)
		{
			return BASIC_FILE_BAD_PATH;
		}
		strcat(szFullPath, lpszFileName);
	}

	Close();

	m_pNextInfo = BasicAllocate(sizeof(WIN32_FIND_DATAA));
	memset(m_pNextInfo, 0, sizeof(WIN32_FIND_DATAA));

	WIN32_FIND_DATAA *pFindData = (WIN32_FIND_DATAA *)m_pNextInfo;

	strcpy(pFindData->cFileName, szFullPath);

	m_hContext = ::FindFirstFileA(szFullPath, (WIN32_FIND_DATAA*)m_pNextInfo);		//I know this

	if (m_hContext == INVALID_HANDLE_VALUE)
	{
		lRet = GetFileErrorID();
		Close();
		return lRet;
	}

	return BASIC_FILE_OK;
}

long CBasicFileFind::FindNextFile()
{
	if (m_hContext == NULL)
	{
		return BASIC_FILE_GENERIC_ERROR;
	}
	if (m_pFoundInfo == NULL)
	{
		m_pFoundInfo = BasicAllocate(sizeof(WIN32_FIND_DATAA));
		memset(m_pFoundInfo, 0, sizeof(WIN32_FIND_DATAA));
	}

	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	if (::FindNextFileA(m_hContext, (LPWIN32_FIND_DATAA)m_pNextInfo) == 0)		//I know this
	{
		return BASIC_FILE_NOT_FOUND;
	}
	return BASIC_FILE_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
