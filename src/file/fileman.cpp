#include "../inc/basic.h"

#include "filedefine.h"

#include <sys/stat.h>
#include <sys/types.h>


#ifdef __BASICWINDOWS
#include <io.h>
#include <direct.h>
#include <time.h>
#include <mbstring.h>
#pragma comment(lib, "shell32.lib")
#else
#include <sys/stat.h>
#include <sys/types.h>
#include "file_linux.h"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD ChangeFileAttributes(struct _stat& st)
{
	DWORD dwAttr = 0;
	if(st.st_mode & S_IFDIR)
	{
		dwAttr |= FILE_ATTRIBUTE_DIRECTORY;
}
	if (st.st_mode & S_IFCHR)
	{
		dwAttr |= FILE_ATTRIBUTE_DEVICE;
	}
	if (st.st_mode & S_IFREG)
	{
		dwAttr |= FILE_ATTRIBUTE_NORMAL;
	}
	return dwAttr;
}

__NS_BASIC_START
long GetFileErrorID()
{
	DWORD l_dwError = ::GetLastError();			//I know this
#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
	return (long)l_dwError;
#else
	switch(l_dwError)
	{
	case NO_ERROR:
		{
			return BASIC_FILE_OK;
			break;
		}
	case ERROR_FILE_NOT_FOUND:		//no need break
	case ERROR_INVALID_HANDLE:		//no need break
	case ERROR_NO_MORE_FILES:		//no need break
	case ERROR_DISK_CHANGE:
		{
			return BASIC_FILE_NOT_FOUND;
			break;
		}
	case ERROR_BAD_NETPATH:			//no need break
	case ERROR_BAD_NET_NAME:
		{
			return BASIC_FILE_BAD_NET_PATH;
			break;
		}

	case ERROR_PATH_NOT_FOUND:		//no need break
	case ERROR_INVALID_DRIVE:		//no need break
	case ERROR_NOT_SAME_DEVICE:		//no need break
	case ERROR_WRONG_DISK:			//no need break
	case ERROR_DUP_NAME:			//no need break
	case ERROR_DEV_NOT_EXIST:		//no need break
	case ERROR_SHARING_PAUSED:		//no need break
	case ERROR_ALREADY_ASSIGNED:	//no need break
	case ERROR_BUFFER_OVERFLOW:		//no need break
	case ERROR_INVALID_NAME:		//no need break
	case ERROR_INVALID_LEVEL:		//no need break
	case ERROR_DIR_NOT_ROOT:		//no need break
	case ERROR_NO_VOLUME_LABEL:		//no need break
	case ERROR_LABEL_TOO_LONG:		//no need break
	case ERROR_BAD_PATHNAME:		//no need break
	case ERROR_META_EXPANSION_TOO_LONG:	//no need break
	case ERROR_FILENAME_EXCED_RANGE:	//no need break
	case ERROR_DIRECTORY:
		{
			return BASIC_FILE_BAD_PATH;
			break;
		}

	case ERROR_TOO_MANY_OPEN_FILES:			//no need break
	case ERROR_SHARING_BUFFER_EXCEEDED:		//no need break
	case ERROR_TOO_MANY_NAMES:				//no need break
	case ERROR_NO_MORE_SEARCH_HANDLES:		
		{
			return BASIC_FILE_TOO_MANY_OPEN;
			break;
		}

	case ERROR_WRITE_PROTECT:				//no need break
	case ERROR_WRITE_FAULT:
		{
			return BASIC_FILE_WRITE_FAULT;
			break;
		}

	case ERROR_NETWORK_BUSY:			//no need break
	case ERROR_BAD_NET_RESP:			//no need break
	case ERROR_NETNAME_DELETED:			//no need break
	case ERROR_NETWORK_ACCESS_DENIED:	//no need break
	case ERROR_INVALID_PASSWORD:
		{
			return BASIC_FILE_NET_ACCESS_DENIED;
			break;
		}

	case ERROR_FILE_EXISTS:				//no need break
	case ERROR_ALREADY_EXISTS:
		{
			return BASIC_FILE_ALREADY_EXISTS;
			break;
		}

	case ERROR_ACCESS_DENIED:			//no need break
	case ERROR_INVALID_ACCESS:			//no need break
	case ERROR_REQ_NOT_ACCEP:			//no need break
	case ERROR_CANNOT_MAKE:				//no need break
	case ERROR_SWAPERROR:				//no need break
	case ERROR_BUSY:	
		{
			return BASIC_FILE_ACCESS_DENIED;
			break;
		}

	case ERROR_BAD_FORMAT:				//no need break
	case ERROR_NOT_DOS_DISK:			//no need break
	case ERROR_BAD_REM_ADAP:			//no need break
	case ERROR_BAD_DEV_TYPE:			//no need break
	case ERROR_INVALID_ORDINAL:			//no need break
	case ERROR_INVALID_EXE_SIGNATURE:	//no need break
	case ERROR_BAD_EXE_FORMAT:			//no need break
	case ERROR_INVALID_TARGET_HANDLE:
		{
			return BASIC_FILE_INVALID;
			break;
		}

	case ERROR_CURRENT_DIRECTORY:		//no need break
	case ERROR_DIR_NOT_EMPTY:
		{
			return BASIC_FILE_REMOVE_CUR_DIR;
			break;
		}

	case ERROR_NET_WRITE_FAULT:
		{
			return BASIC_FILE_NET_WRITE_FAULT;
			break;
		}

	case ERROR_BAD_UNIT:
	case ERROR_NOT_READY:				//no need break
	case ERROR_BAD_COMMAND:				//no need break
	case ERROR_CRC:						//no need break
	case ERROR_UNEXP_NET_ERR:			//no need break
	case ERROR_ADAP_HDW_ERR:			//no need break
	case ERROR_OPERATION_ABORTED:		//no need break
	case ERROR_IO_INCOMPLETE:			//no need break
	case ERROR_IO_PENDING:				//no need break
	case ERROR_INVALID_CATEGORY:
		{
			return BASIC_FILE_HARD_IO;
			break;
		}

	case ERROR_BAD_LENGTH:				//no need break
	case ERROR_SEEK:					//no need break
	case ERROR_SECTOR_NOT_FOUND:		//no need break
	case ERROR_READ_FAULT:				//no need break
	case ERROR_NEGATIVE_SEEK:			//no need break
	case ERROR_SEEK_ON_DEVICE:
		{
			return BASIC_FILE_BAD_SEEK;
			break;
		}

	case ERROR_SHARING_VIOLATION:
		{
			return BASIC_FILE_SHARING_VIOLATION;
			break;
		}

	case ERROR_LOCK_VIOLATION:				//no need break
	case ERROR_DRIVE_LOCKED:				//no need break
	case ERROR_LOCK_FAILED:					//no need break
		{
			return BASIC_FILE_LOCK_VIOLATION;
			break;
		}

	case ERROR_HANDLE_EOF:
		{
			return BASIC_FILE_END_OF_FILE;
			break;
		}

	case ERROR_HANDLE_DISK_FULL:		//no need break
	case ERROR_DISK_FULL:
		{
			return BASIC_FILE_DISK_FULL;
			break;
		}

	case ERROR_NO_SPOOL_SPACE:
		{
			return BASIC_FILE_DIR_FULL;
			break;
		}

	}
	return BASIC_FILE_GENERIC_ERROR;
#endif
}

#ifdef __BASICWINDOWS
void _GetRoot(const char* lpszPath, CBasicString& strRoot)
{
	char* lpszRoot = strRoot.GetBuffer(_MAX_PATH);
	memset(lpszRoot, 0, _MAX_PATH);
	strncpy(lpszRoot, lpszPath, _MAX_PATH);
	PathStripToRootA(lpszRoot);		//I know this
	strRoot.ReleaseBuffer();
}
#endif

long Basic_GetFileFullPath(char* lpszPathOut, const char* lpszFileIn)
{
#ifdef __BASICWINDOWS
	static char szCurPath[] = ".\\";
#else
	static char szCurPath[] = "./";
#endif
	if (STR_INVALID(lpszFileIn))
	{
		lpszFileIn = szCurPath;
	}

	char* lpszFilePart;
	DWORD dwRet = ::GetFullPathNameA(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart);		//I know this
	if (dwRet == 0)
	{
		return GetFileErrorID();
	}
	else if (dwRet >= _MAX_PATH)
	{
		return BASIC_FILE_BAD_PATH; // long path won't fit in buffer
	}
#ifdef __BASICWINDOWS
	CBasicString strRoot;
	// determine the root name of the volume
	_GetRoot(lpszPathOut, strRoot);

	if (!::PathIsUNCA(strRoot.c_str()))		//I know this
	{
		// get file system information for the volume
		DWORD dwFlags = 0;
		DWORD dwDummy = 0;
		if (!GetVolumeInformationA(strRoot.c_str(), NULL, 0, NULL, &dwDummy, &dwFlags, NULL, 0))		//I know this
		{
			return GetFileErrorID();   // preserving case may not be correct
		}

		// not all characters have complete uppercase/lowercase
		if (!(dwFlags & FS_CASE_IS_PRESERVED))
		{
			lpszPathOut = _strupr(lpszPathOut);
		}

		// assume non-UNICODE file systems, use OEM character set
		if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
		{
			WIN32_FIND_DATAA data;
			HANDLE h = FindFirstFileA(lpszFileIn, &data);			//I know this
			if (h != INVALID_HANDLE_VALUE)
			{
				FindClose(h);				//I know this
				if (lpszFilePart != NULL && lpszFilePart > lpszPathOut)
				{
					int nFileNameLen = strlen(data.cFileName);		//I know this
					int nIndexOfPart = (int)(lpszFilePart - lpszPathOut);
					if ((nFileNameLen + nIndexOfPart) < _MAX_PATH)
					{
						strcpy(lpszFilePart, data.cFileName);
					}
					else
					{
						return BASIC_FILE_BAD_PATH; // Path doesn't fit in the buffer.
					}
				}
				else
				{
					return GetFileErrorID();
				}
			}
		}
	}
#endif
	return BASIC_FILE_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
time_t _FileTimeToTime(const FILETIME& fileTime)
{
#ifdef __BASICWINDOWS
	// first convert file time (UTC time) to local time
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))   //I know this
	{
		return 0;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))    //I know this
	{
		return 0;
	}

	if (sysTime.wYear < 1900)
	{
		return 0;
	}

	struct tm atm;

	atm.tm_sec  = (int)sysTime.wSecond;
	atm.tm_min  = (int)sysTime.wMinute;
	atm.tm_hour = (int)sysTime.wHour;
	atm.tm_mday = (int)sysTime.wDay;
	atm.tm_mon  = (int)sysTime.wMonth - 1;        // tm_mon is 0 based
	atm.tm_year = (int)sysTime.wYear - 1900;     // tm_year is 1900 based

	return mktime(&atm);
#else
	return fileTime;
#endif
}

long _TimeToFileTime(time_t local, FILETIME* pFileTime)
{
	assert(pFileTime != NULL);

	if (pFileTime == NULL || local <= 0) 
	{
		return -1;
	}
#ifdef __BASICWINDOWS
	struct tm* pTime = localtime(&local);
	if (!pTime)
	{
		return -1;
	}
	/*struct tm atm;
	if(localtime(&atm, &local) != 0)
	{
		return -1;
	}*/

	SYSTEMTIME sysTime;
	sysTime.wYear   = (WORD)pTime->tm_year + 1900;
	sysTime.wMonth  = (WORD)pTime->tm_mon + 1;
	sysTime.wDay    = (WORD)pTime->tm_mday;
	sysTime.wHour   = (WORD)pTime->tm_hour;
	sysTime.wMinute = (WORD)pTime->tm_min;
	sysTime.wSecond = (WORD)pTime->tm_sec;
	sysTime.wMilliseconds = 0;

	// convert system time to local file time
	FILETIME localTime;
	if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, &localTime))   //I know this
	{
		return -1;
	}
	// convert local file time to UTC file time
	if (!LocalFileTimeToFileTime(&localTime, pFileTime))   //I know this
	{
		return -1;
	}
#else
	*pFileTime = local;
#endif
	return BASIC_FILE_OK;
}

void WFillFileStatusTime(TLFileStatusW& rStatus, const FILETIME& ftCreate, const FILETIME& ftAccess, const FILETIME& ftModify)
{
	rStatus.m_ctime = _FileTimeToTime(ftCreate);
	rStatus.m_atime = _FileTimeToTime(ftAccess);
	rStatus.m_mtime = _FileTimeToTime(ftModify);

	if (rStatus.m_ctime == 0)
	{
		rStatus.m_ctime = rStatus.m_mtime;
	}
	if (rStatus.m_atime == 0)
	{
		rStatus.m_atime = rStatus.m_mtime;
	}
}
void FillFileStatusTime(TLFileStatus& rStatus, const FILETIME& ftCreate, const FILETIME& ftAccess, const FILETIME& ftModify)
{
	rStatus.m_ctime = _FileTimeToTime(ftCreate);
	rStatus.m_atime = _FileTimeToTime(ftAccess);
	rStatus.m_mtime = _FileTimeToTime(ftModify);

	if (rStatus.m_ctime == 0)
	{
		rStatus.m_ctime = rStatus.m_mtime;
	}
	if (rStatus.m_atime == 0)
	{
		rStatus.m_atime = rStatus.m_mtime;
	}
}

long Basic_GetFileStatus(const char* lpszFileName, TLFileStatus& rStatus)
{
	if (STR_INVALID(lpszFileName))
	{
		return BASIC_FILE_BAD_PATH;
	}

	if (strlen(lpszFileName) >= _MAX_PATH)			//I know this
	{
		return BASIC_FILE_BAD_PATH;
	}

	long lRet = Basic_GetFileFullPath(rStatus.m_szFullName, lpszFileName);
	if (lRet != BASIC_FILE_OK)
	{
		rStatus.m_szFullName[0] = '\0';
		return lRet;
	}

	struct _stat buf;
	int result = _stat(rStatus.m_szFullName, &buf);
	if (result == 0)
	{
		rStatus.m_attribute = (BYTE)(ChangeFileAttributes(buf) & ~FILE_ATTRIBUTE_NORMAL);
		rStatus.m_size = (long)buf.st_size;
		rStatus.m_ctime = buf.st_ctime;
		rStatus.m_atime = buf.st_atime;
		rStatus.m_mtime = buf.st_mtime;
		return BASIC_FILE_OK;
	}
	else
	{
		return BASIC_FILE_NOT_FOUND;
	}
}

long Basic_GetFileName(const char* lpszPathName, char* lpszName, int nMax)
{
	assert(lpszPathName != NULL);

	const char* lpszTemp = Basic_FindFileName(lpszPathName);

	long lLen = strlen(lpszTemp) + 1;			//I know this
	if (lpszName == NULL || nMax < lLen)
	{
		return lLen;
	}
	strcpy(lpszName, lpszTemp);
	return BASIC_FILE_OK;
}

long Basic_GetFileTitle(const char* lpszPathName, char* lpszTitle, int nMax)
{
	assert(lpszPathName != NULL);

	long lRet = Basic_GetFileName(lpszPathName, lpszTitle, nMax);
	if (lRet == BASIC_FILE_OK)
	{
		char* p = strrchr(lpszTitle, '.');
		if (p != NULL)
		{
			*p = '\0';
		}
	}
	return lRet;
}

long Basic_GetFileDirPath(const char* lpszPathName, char* lpszDirPath, int nMax)
{
	if (lpszDirPath == NULL || nMax < _MAX_PATH)
	{
		return _MAX_PATH;
	}
	strcpy(lpszDirPath, lpszPathName);
	char* lpszTemp = (char*)Basic_FindFileName(lpszDirPath);
	(*lpszTemp) = '\0';
	return BASIC_FILE_OK;

}

DWORD WBasic_GetFileAttributes(LPCTSTR lpszFileName)
{
	DWORD dwRet = ::GetFileAttributes(lpszFileName);			//I know this
	if(dwRet == INVALID_FILE_ATTRIBUTES)
	{
		return 0;
	}
	return dwRet;
}
DWORD Basic_GetFileAttributes(const char* lpszFileName)
{
	DWORD dwRet = ::GetFileAttributesA(lpszFileName);			//I know this
	if (dwRet == INVALID_FILE_ATTRIBUTES)
	{
		return 0;
	}
	return dwRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FILETIME* _sfs_TimeToFileTime(time_t local, FILETIME* pFileTime)
{
	long lRet = _TimeToFileTime(local, pFileTime);
	if(lRet == BASIC_FILE_OK)
	{
		return pFileTime;
	}
	return NULL;
}
long Basic_SetFileStatus(const char* lpszFileName, const TLFileStatus& status)
{
	DWORD dwAttr = ::GetFileAttributesA(lpszFileName);			//I know this
	if (dwAttr == INVALID_FILE_ATTRIBUTES)
	{
		return GetFileErrorID();
	}

	if ((DWORD)status.m_attribute != dwAttr && (dwAttr & FILE_ATTRIBUTE_READONLY))
	{
		// Set file attribute, only if currently readonly.
		// This way we will be able to modify the time assuming the
		// caller changed the file from readonly.

		if (!::SetFileAttributesA(lpszFileName, (DWORD)status.m_attribute))  //I know this
		{
			return GetFileErrorID();
		}
	}


	long lRet = 0;
	// last modification time
	if (status.m_mtime != 0)
	{
#ifdef __BASICWINDOWS
		FILETIME creationTime;
		FILETIME lastAccessTime;
		FILETIME lastWriteTime;
		LPFILETIME lpCreationTime = _sfs_TimeToFileTime(status.m_ctime, &creationTime);
		LPFILETIME lpLastAccessTime = _sfs_TimeToFileTime(status.m_atime, &lastAccessTime);
		LPFILETIME lpLastWriteTime = _sfs_TimeToFileTime(status.m_mtime, &lastWriteTime);

		HANDLE hFile = ::CreateFileA(lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //I know this

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return GetFileErrorID();
		}

		if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))  //I know this
		{
			lRet = GetFileErrorID();
		}
		::CloseHandle(hFile);		//I know this
#else
		if (!SetFileTime(lpszFileName, &status.m_ctime, &status.m_atime, &status.m_mtime))  //I know this
		{
			lRet = GetFileErrorID();
		}
#endif
	}

	if (lRet == BASIC_FILE_OK)
	{
		if ((DWORD)status.m_attribute != dwAttr && !(dwAttr & FILE_ATTRIBUTE_READONLY))
		{
			if (!SetFileAttributesA(lpszFileName, (DWORD)status.m_attribute))		//I know this
			{
				lRet = GetFileErrorID();
			}
		}
	}
	return lRet;
}
long WBasic_SetFileStatus(LPCTSTR lpszFileName, const TLFileStatusW& status)
{
	DWORD dwAttr = ::GetFileAttributes(lpszFileName);			//I know this
	if(dwAttr == INVALID_FILE_ATTRIBUTES)
	{
		return GetFileErrorID();
	}

	if ((DWORD)status.m_attribute != dwAttr && (dwAttr & FILE_ATTRIBUTE_READONLY))
	{
		// Set file attribute, only if currently readonly.
		// This way we will be able to modify the time assuming the
		// caller changed the file from readonly.

		if (!::SetFileAttributes(lpszFileName, (DWORD)status.m_attribute))  //I know this
		{
			return GetFileErrorID();
		}
	}


	long lRet = 0;
	// last modification time
	if (status.m_mtime != 0)
	{
	#ifdef __BASICWINDOWS
		FILETIME creationTime;
		FILETIME lastAccessTime;
		FILETIME lastWriteTime;
		LPFILETIME lpCreationTime   = _sfs_TimeToFileTime(status.m_ctime, &creationTime);
		LPFILETIME lpLastAccessTime = _sfs_TimeToFileTime(status.m_atime, &lastAccessTime);
		LPFILETIME lpLastWriteTime  = _sfs_TimeToFileTime(status.m_mtime, &lastWriteTime);

		HANDLE hFile = ::CreateFile(lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //I know this

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return GetFileErrorID();
		}

		if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))  //I know this
		{
			lRet = GetFileErrorID();
		}
		::CloseHandle(hFile);		//I know this
	#else
		if (!SetFileTime(lpszFileName, &status.m_ctime, &status.m_atime, &status.m_mtime))  //I know this
		{
			lRet = GetFileErrorID();
		}
	#endif
	}

	if(lRet == BASIC_FILE_OK)
	{
		if ((DWORD)status.m_attribute != dwAttr && !(dwAttr & FILE_ATTRIBUTE_READONLY))
		{
			if (!SetFileAttributes(lpszFileName, (DWORD)status.m_attribute))		//I know this
			{
				lRet = GetFileErrorID();
			}
		}
	}
	return lRet;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long WBasic_RenameFile(LPCTSTR lpszOldName, LPCTSTR lpszNewName)
{
	if (!::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName))		//I know this
	{
		return GetFileErrorID();
	}
	return BASIC_FILE_OK;
}
long Basic_RenameFile(const char* lpszOldName, const char* lpszNewName)
{
	if (!::MoveFileA((char*)lpszOldName, (char*)lpszNewName))		//I know this
	{
		return GetFileErrorID();
	}
	return BASIC_FILE_OK;
}

long Basic_DeleteFile(const char* lpszFileName)
{
#ifdef __BASICWINDOWS
	if (!::DeleteFileA(lpszFileName))		//I know this
#else
	if (!::DeleteFile(lpszFileName))		//I know this
#endif
	{
		return GetFileErrorID();
	}
	return BASIC_FILE_OK;
}
long WBasic_DeleteFile(LPCTSTR lpszFileName)
{
	if (!::DeleteFile((LPTSTR)lpszFileName))		//I know this
	{
		return GetFileErrorID();
	}
	return BASIC_FILE_OK;
}

long WBasic_CopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, bool bFailIfExists)
{
	if(!CopyFile(lpExistingFileName, lpNewFileName, bFailIfExists))		//I know this
	{
		return GetFileErrorID();
	}
	return BASIC_FILE_OK;
}
long Basic_CopyFile(const char* lpExistingFileName, const char* lpNewFileName, bool bFailIfExists)
{
	if (!CopyFileA(lpExistingFileName, lpNewFileName, bFailIfExists))		//I know this
	{
		return GetFileErrorID();
	}
	return BASIC_FILE_OK;
}

bool WBasic_PathMatchSpec(LPCTSTR pszFile, LPCTSTR pszSpec)
{
	return ::PathMatchSpec(pszFile, pszSpec);		//I know this
}
bool Basic_PathMatchSpec(const char* pszFile, const char* pszSpec)
{
	return ::PathMatchSpecA(pszFile, pszSpec);		//I know this
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* Basic_FindFileName(const char* lpszPathName)
{
	char* lpszTemp = (char*)lpszPathName;
	for (const char* lpsz = lpszPathName; *lpsz != '\0'; lpsz = (const char*)_tcsinc_s((const unsigned char*)lpsz))
	{
		// remember last directory/drive separator
		if (*lpsz == '\\' || *lpsz == '/' || *lpsz == ':')
		{
			lpszTemp = (char*)_tcsinc_s((const unsigned char*)lpsz);
		}
	}
	return lpszTemp;
}

long Basic_mkdir(const char* lpszPath)
{
	if (STR_INVALID(lpszPath))
	{
		return BASIC_FILE_BAD_PATH;
	}

	char sz[MAX_PATH + 2];
	memset(sz, 0, sizeof(sz));
	strncpy(sz, lpszPath, MAX_PATH);

	char* pBuffer = sz + 1;
	while (*pBuffer)
	{
		if (*pBuffer == '\\' || *pBuffer == '/')
		{
			char cChar = *pBuffer;
			*pBuffer = '\0';
			if (_access(sz, 0) != 0)
			{
#ifdef __BASICWINDOWS
				if (_mkdir(sz) != 0)
				{
					return BASIC_FILE_MKDIR_ERROR;
				}
#else				 
				if (_mkdir(sz, 777))
				{
					return BASIC_FILE_MKDIR_ERROR;
				}
				chmod(sz, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
			}
			*pBuffer = cChar;
		}
		pBuffer++;
	}
	return BASIC_FILE_OK;
}


long WBasic_mkdir(LPCTSTR lpszPath)
{
	if (WSTR_INVALID(lpszPath))
	{
		return BASIC_FILE_BAD_PATH;
	}

	TCHAR sz[MAX_PATH + 2];
	memset(sz, 0, sizeof(sz));
	_tcsncpy(sz, lpszPath, MAX_PATH);

	TCHAR* pBuffer = sz + 1;
	while (*pBuffer)
	{
		if (*pBuffer == _T('\\') || *pBuffer == _T('/'))
		{
			TCHAR cChar = *pBuffer;
			*pBuffer = _T('\0');
			if (_taccess(sz, 0) != 0)
			{
#ifdef __BASICWINDOWS
				if (_tmkdir(sz) != 0)
				{
					return BASIC_FILE_MKDIR_ERROR;
				}
#else				 
				if (_tmkdir(sz, 777))
				{
					return BASIC_FILE_MKDIR_ERROR;
				}
				chmod(sz, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
			}
			*pBuffer = cChar;
		}
		pBuffer++;
	}
	return BASIC_FILE_OK;
}

char* Basic_TempFileName(const char* lpszDir, const char* lpszHead, const char* lpszExt, char* lpszBuffer, int nMax)
{
	static long g_tempnum = 0;
	if (lpszDir == NULL || lpszDir[0] == '\0' || lpszBuffer == NULL || nMax <= 0)
	{
		return NULL;
	}
	char szTemp[1024];
	memset(szTemp, 0, sizeof(szTemp));
	strcpy(szTemp, lpszDir);

	char* p = szTemp + strlen(szTemp) - 1;
	if (*p != '\\' && *p != '/')
	{
		p++;
		*p = PATHSPLIT_S;
	}
	p++;

	if ((lpszHead != NULL) && (*lpszHead != '\0'))
	{
		strcpy(p, lpszHead);
		p += strlen(lpszHead);
	}
	char szExt[16];
	memset(szExt, 0, sizeof(szExt));
	if (lpszExt != NULL && lpszExt[0] != '\0')
	{
		szExt[0] = '.';
		strcpy(&szExt[1], lpszExt);
	}

	do
	{
		sprintf(p, "%d%s", (uint32_t)BasicInterlockedIncrement(&g_tempnum), szExt);

	} while (_access(szTemp, 0) != -1);

	strcpy(lpszBuffer, szTemp);

	return lpszBuffer;
}


static char* _ChangeFileName(char* pszPath)  //I know that the length of this buffer
{
	char* pBuffer = Basic_FindFileName(pszPath);
	if(*pBuffer == '\0')
	{
		pBuffer--;
		*pBuffer = '\0';
		pBuffer  = NULL;
	}
	return pBuffer;
}

long Basic_FileOperation(uint32_t wFunc, const char* pFrom, const char* pTo)
{
	char szCPath[_MAX_PATH + 2];
	char szSPath[_MAX_PATH + 2];

	memset(szCPath, 0, sizeof(szCPath));
	memset(szSPath, 0, sizeof(szSPath));

	long lRet = Basic_GetFileFullPath(szCPath, pFrom);
	if (lRet != BASIC_FILE_OK)
	{
		return lRet;
	}

	char* pBuffer = _ChangeFileName(szCPath);
	if(wFunc == BASIC_FO_DELETE)
	{
		/* 支持删除单个文件
		if(pBuffer != NULL)
		{
			if(!_tcschr(pBuffer, '*') && !_tcschr(pBuffer, '?'))	//不支持删除单个文件，如果输入单个文件就删除整个目录
			{
				pBuffer--;
				memset(pBuffer, 0, 4);
			}
		}
		*/
		pTo = NULL;		//非NULL会产生一个内存检测异常
	}
	else if(pTo != NULL)
	{
		Basic_mkdir(pTo);			//建好目标目录
	
		lRet = Basic_GetFileFullPath(szSPath, pTo);
		if (lRet != BASIC_FILE_OK)
		{
			return lRet;
		}
		_ChangeFileName(szSPath);
		pTo = szSPath;
	}

	lRet = BASIC_FILE_GENERIC_ERROR;
#ifdef __BASICWINDOWS
	SHFILEOPSTRUCTA shFOS;
	shFOS.hwnd   = NULL;
	shFOS.wFunc  = wFunc;
	shFOS.pFrom  = szCPath;
	shFOS.pTo    = pTo;
	shFOS.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOCONFIRMMKDIR;
	int nRet = SHFileOperationA(&shFOS);  //I know this
	if(nRet == 0)  
	{
		lRet = BASIC_FILE_OK;
	}
#else
	TCHAR szCmd[1024];
	TCHAR* pFileName = Basic_FindFileName(szCPath);
	switch(wFunc)
	{
		case BASIC_FO_COPY:
		{
			//_stprintf(szCmd, "cp -f '%s' '%s' ", szCPath, szSPath);
			if(pFileName != NULL && pFileName > szCPath)
			{
				*(pFileName-1) = '\0';	
				_stprintf(szCmd, "find '%s' -maxdepth 1 -mindepth 1 -name \"%s\"|xargs -i cp -rf {} '%s' ", szCPath, pFileName, szSPath);
			}
			else
			{
				_stprintf(szCmd, "find '%s' -maxdepth 0 |xargs -i cp -rf {} '%s' ", szCPath, szSPath);
			}
			break;
		}
		case BASIC_FO_DELETE:
		{
			//_stprintf(szCmd, "rm -rf '%s' ", szCPath);
			if(pFileName != NULL && pFileName > szCPath)
			{
				*(pFileName-1) = '\0';	
				_stprintf(szCmd, "find '%s' -maxdepth 1 -mindepth 1 -name \"%s\" -print0|xargs -0 rm -rf -- ", szCPath, pFileName);
			}
			else
			{
				_stprintf(szCmd, "find '%s' -print0|xargs -0 rm -rf -- ", szCPath);
			}
			break;
		}
		case BASIC_FO_MOVE:
		case BASIC_FO_RENAME:
		{
			//_stprintf(szCmd, "mv -f '%s' '%s' ", szCPath, szSPath);
			if(pFileName != NULL && pFileName > szCPath)
			{
				*(pFileName-1) = '\0';	
				_stprintf(szCmd, "find '%s' -maxdepth 1 -mindepth 1 -name \"%s\"|xargs -i mv -f {} '%s' ", szCPath, pFileName, szSPath);
			}
			else
			{
				_stprintf(szCmd, "find '%s' -maxdepth 1 -mindepth 1 |xargs -i mv -f {} '%s' ", szCPath, szSPath);
			}
			break;
		}
		default:
		{
			return BASIC_FILE_NOT_SUPPORT;
		}
	}
	if(_basic_system(szCmd) != 0)
	{
		lRet = GetFileErrorID();
		BasicTrace("file command fail : %s = %d \n", szCmd, lRet);
	}
	else
	{
		lRet = BASIC_FILE_OK;
	}
#endif
	return lRet;

}

template<typename F>
static long _FindAllFileInPath(const char* lpszFilePath, const char* lpszFileName, const char* lpszFilterFileName, F f, DWORD dwFindMode)
{
	bool bFilter = STR_INVALID(lpszFilterFileName) ? false : true;
	
	CBasicFileFind tlFinder;
	long lRet = tlFinder.FindFile(lpszFilePath, lpszFileName);
	CBasicString strFileName;
	
	long lResult = 0;
	while(!lRet)
	{
		lRet = tlFinder.FindNextFile();
		strFileName = tlFinder.GetFilePath();

		BYTE cAttr = _basic_fa_normal;
		if(tlFinder.IsReadOnly())
		{
			cAttr |= _basic_fa_readOnly;
		}
		if(tlFinder.IsHidden())
		{
			cAttr |= _basic_fa_hidden;
		}
		if(tlFinder.IsSystem())
		{
			cAttr |= _basic_fa_sys;
		}
		if(tlFinder.IsArchived())
		{
			cAttr |= _basic_fa_archive;
		}
		if(tlFinder.IsDirectory())
		{
			cAttr |= _basic_fa_directory;
			if(dwFindMode & BASIC_FIND_DIR)
			{
				f(strFileName.c_str(), tlFinder.GetCreationTime(), tlFinder.GetLastWriteTime(), (long)tlFinder.GetLength(), cAttr);
				lResult++; 
			}
			if((dwFindMode & BASIC_FIND_SUBDIR) && !tlFinder.IsDots())
			{
				strFileName += PATHSPLIT_S;
				lResult += _FindAllFileInPath(strFileName.c_str(), lpszFileName, lpszFilterFileName, f, dwFindMode);
			}
		}
		else if (!bFilter || Basic_PathMatchSpec(strFileName.c_str(), lpszFilterFileName))
		{
			f(strFileName.c_str(), tlFinder.GetCreationTime(), tlFinder.GetLastWriteTime(), (long)tlFinder.GetLength(), cAttr);
			lResult++; 
		}
	}
	return lResult;
}

long Basic_FindAllFileInPath(const char* lpszFilePath, const char* lpszFileName, DWORD dwFindMode, const std::function<long(const char*, time_t, time_t, long, BYTE)>& f)
{
	CBasicString strFindFileName;
	CBasicString strFilePath;
	if((dwFindMode & BASIC_FIND_SUBDIR))
	{
		char* lpTemp = Basic_FindFileName(lpszFilePath);
		if (__tcslen(lpTemp) > 0)		//有文件名
		{
			strFindFileName = lpTemp;
			strFilePath = lpszFilePath;
			char* pBuffer = strFilePath.GetBuffer(0);
			int nIndex = (int)(lpTemp - lpszFilePath);
			pBuffer[nIndex] = '\0';

			lpszFilePath = pBuffer;
		}
		else if(!STR_INVALID(lpszFileName))
		{
			strFindFileName = lpszFileName;
		}
		lpszFileName = NULL;
	}
	return _FindAllFileInPath(lpszFilePath, lpszFileName, strFindFileName.c_str(), f, dwFindMode);
}

class AddStringArray
{
public:
	AddStringArray(CBasicStringArray& ay)
		: m_ayFile(ay)
	{

	}

	long operator()(const char* lpszFileName, time_t tmCreateTime, time_t tmModifyTime, long lFileLength, BYTE cAttr)
	{
		m_ayFile.Add(lpszFileName);
		return 0;
	}
protected:
	CBasicStringArray&	m_ayFile;
};

long Basic_FindAllFileInPath(const char* lpszFilePath, const char* lpszFileName, CBasicStringArray& ayFile, DWORD dwFindMode)
{
	AddStringArray addArray(ayFile);

	return Basic_FindAllFileInPath(lpszFilePath, lpszFileName, dwFindMode, addArray);
}

void Basic_RegulatePathString(CBasicString& strPath)
{
	if(!strPath.IsEmpty())
	{
		strPath.Replace('\\', '/');			//把所有的反斜杠换成正斜杠，这样 windows 和 linux 就可以通用了。
		if (strPath.GetAt(__tcslen(strPath.c_str()) - 1) != '/')
		{
			strPath += '/';
		}
	}
}

void Basic_RegulateFileNameString(CBasicString& strFileName)
{
	if(!strFileName.IsEmpty())
	{
		strFileName.Replace('\\', '/');			//把所有的反斜杠换成正斜杠，这样 windows 和 linux 就可以通用了。
	}
}

//! 读取文件
bool _BASIC_DLL_API Basic_ReadTotalFile(const char* pFileName, basiclib::CBasicSmartBuffer& smBuf) {
	FILE* fp = fopen(pFileName, "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long lLength = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (lLength > 0) {
			smBuf.SetDataLength(lLength);
			char* pBuffer = smBuf.GetDataBuffer();
			fread(pBuffer, sizeof(char), lLength, fp);
		}
		fclose(fp);
		return true;
	}
	return false;
}

__NS_BASIC_END
