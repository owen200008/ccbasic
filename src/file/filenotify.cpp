// FileNotify.cpp: implementation of the CBasicFileNotify class.
//
//////////////////////////////////////////////////////////////////////
#include "../inc/basic.h"

#ifdef __BASICWINDOWS
#include <winbase.h>
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
__NS_BASIC_START

CBasicFileNotify::CBasicFileNotify(DWORD dwOptions)
{
	m_hCompPort	= NULL;
	m_hThread	= NULL;
	m_eRunFlag = ThreadStop;
	m_dwNotifyOps = dwOptions;
}

CBasicFileNotify::~CBasicFileNotify()
{
	StopWatch();
}

void CBasicFileNotify::StopWatch()
{
	EndWatch();
	CSingleLock lock(&m_mtxAddRemove);
	lock.Lock();
	for_each(m_ayDirInfo.begin(), m_ayDirInfo.end(), basiclib::DeleteObject());
	m_ayDirInfo.clear();
}

////////////////////////////////////////////////////////
DWORD CBasicFileNotify::AddWatch(const char* lpszPath, DWORD dwNotifyFilter, DWORD dwWatchOps, LPVOID lpVoid, Change_Handler pfuncHander)
{
	if(STR_INVALID(lpszPath))
	{
		return INVALID_WATCH_ID;
	}

	Basic_mkdir(lpszPath);
	

	_DIRECTORY_INFO* lpdi = Basic_NewObject<_DIRECTORY_INFO>();
	__tcscpyn(lpdi->m_lpszDirName, MAX_PATH, lpszPath);

	char* pszFileName = Basic_FindFileName(lpdi->m_lpszDirName);
	if(!STR_INVALID(pszFileName))
	{
		strcpy(lpdi->m_lpszFileName, pszFileName);
		*pszFileName = '\0';
	}
	lpdi->m_dwNotifyFilter	= dwNotifyFilter;
	lpdi->m_dwWathOptions	= dwWatchOps;
	lpdi->m_lpVoid			= lpVoid;
	lpdi->m_pfuncHander		= pfuncHander;

#ifdef __BASICWINDOWS
	if(IsCompPort())
	{
		lpdi->m_hDir = CreateFileA(lpdi->m_lpszDirName,FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,		//I know this
			NULL, OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if( lpdi->m_hDir == INVALID_HANDLE_VALUE )
		{
			BASIC_DeleteObject(lpdi);
			return INVALID_WATCH_ID;
		}

		m_hCompPort = CreateIoCompletionPort(lpdi->m_hDir, m_hCompPort, (DWORD)lpdi, 0);		//I know this
		ReadDirectoryChangesW(lpdi->m_hDir,lpdi->m_lpBuffer, MAX_BUFFER, lpdi->IsWatchSubTree(), lpdi->m_dwNotifyFilter, &lpdi->m_dwBufLength, &lpdi->m_olOverlapped, NULL);	//I know this
	}
#else
	if(IsCompPort())
	{
		m_dwNotifyOps &= ~TFN_OPS_MODE;
		m_dwNotifyOps |= TFN_OPS_SCANFILE;
	}
#endif

	CSingleLock lock(&m_mtxAddRemove);
	lock.Lock();
	
	m_ayDirInfo.push_back(lpdi);

	if (NULL == m_hThread)
	{
		StartWatch();
	}
	lock.Unlock();
	return (DWORD)lpdi;
}

/////////////////////////////////////////////////

bool CBasicFileNotify::RemoveWatch(DWORD dwWatchID)
{
	_DIRECTORY_INFO* pFind = (_DIRECTORY_INFO*)dwWatchID;
	if (pFind == NULL)
	{
		return false;
	}

	CSingleLock lock(&m_mtxAddRemove);
	lock.Lock();
	_di_array::iterator iFind = std::find(m_ayDirInfo.begin(), m_ayDirInfo.end(), pFind);
	bool bFound = false;
	if(iFind != m_ayDirInfo.end())
	{
		m_ayDirInfo.erase(iFind);
		BASIC_DeleteObject(pFind);
		bFound = true;
	}
	if(bFound && m_ayDirInfo.empty())
	{
		lock.Unlock();
		EndWatch();
	}

	return bFound;
}

void CBasicFileNotify::StartWatch()
{
	if (m_hThread == NULL)
	{
		m_eRunFlag = ThreadRun;
		
		DWORD dwThreadID = 0;
		m_hThread = BasicCreateThread((LPBASIC_THREAD_START_ROUTINE)HandleDirectoryChange, this, &dwThreadID);
		if (m_hThread == NULL)
		{
			m_eRunFlag = ThreadStop;
		}
	}
}

void CBasicFileNotify::EndWatch()
{
	m_eRunFlag = ThreadStop;
#ifdef __BASICWINDOWS
	if(m_hCompPort != NULL)
	{
		PostQueuedCompletionStatus( m_hCompPort, 0, 0, NULL );		//I know this
		m_hCompPort	= NULL;
	}
#endif
	BasicWaitThread(m_hThread);
	BasicCloseHandle(m_hThread);			//I know this
	m_hThread = NULL;
}

THREAD_RETURN CBasicFileNotify::HandleDirectoryChange(LPVOID lpVoid)
{
	ASSERT(lpVoid != NULL);
	CBasicFileNotify* pNotify = (CBasicFileNotify*)lpVoid;
	if(pNotify->IsScanFile())
	{
		pNotify->HandleDirectoryChange_ScanDir();
	}
#ifdef __BASICWINDOWS
	else if(pNotify->IsCompPort())
	{
		pNotify->HandleDirectoryChange_CompPort();
	}
#endif

	pNotify->m_eRunFlag = ThreadStop;
	return 0;
}

#ifdef __BASICWINDOWS
void CBasicFileNotify::HandleDirectoryChange_CompPort()
{
	DWORD numBytes = 0;
	DWORD cbOffset = 0;
	_DIRECTORY_INFO* lpdi = NULL;
	LPOVERLAPPED lpOverlapped = NULL;
	PFILE_NOTIFY_INFORMATION lpfni = NULL;

	CSingleLock lock(&m_mtxAddRemove);
	do
	{
		numBytes = 0;
		lpdi = NULL;
		lpOverlapped = NULL;
		if (GetQueuedCompletionStatus( (HANDLE)m_hCompPort, &numBytes, (LPDWORD)&lpdi, &lpOverlapped, INFINITE))		//I know this
		{
			lock.Lock();
			if(m_hThread != NULL && IsValidDirectoryInfo(lpdi))
			{
				if (numBytes > 0)
				{
					lpfni = (PFILE_NOTIFY_INFORMATION)lpdi->m_lpBuffer;
					do
					{
						cbOffset = lpfni->NextEntryOffset;
						
						if(lpfni->FileNameLength > 0)
						{
							CBasicString strFileName = basiclib::Basic_WideStringToMultiString(lpfni->FileName, lpfni->FileNameLength).c_str();
							HandleChangeNotify(lpdi, strFileName, lpfni->Action);				
						}

						lpfni = (PFILE_NOTIFY_INFORMATION)((LPBYTE) lpfni + cbOffset);


					} while(cbOffset);
				}		
				memset(lpdi->m_lpBuffer, 0, MAX_BUFFER);
				ReadDirectoryChangesW(lpdi->m_hDir,lpdi->m_lpBuffer, MAX_BUFFER, lpdi->IsWatchSubTree(), lpdi->m_dwNotifyFilter, &lpdi->m_dwBufLength, &lpdi->m_olOverlapped, NULL);  //I know this
			}
			lock.Unlock();
		}
	} while(IsThreadRunning() && !m_ayDirInfo.empty());
}
#endif

void CBasicFileNotify::HandleDirectoryChange_ScanDir()
{
	CSingleLock lock(&m_mtxAddRemove);
	do
	{
		// \u505A5\u79D2\u949F\u5EF6\u8FDF\u52A0\u5165
		time_t tBefore = time(NULL) - 5;
		lock.Lock();
		_di_array::iterator iDir = m_ayDirInfo.begin();
		_di_array::iterator iEnd = m_ayDirInfo.end();
		for(; iDir != iEnd; iDir++)
		{
			_DIRECTORY_INFO* lpdi = *iDir;
			if (lpdi->m_tmLastChange == 0) {
				lpdi->m_tmLastChange = time(NULL);
			}
			ScanDir(lpdi, lpdi->m_lpszDirName, lpdi->m_tmLastChange, lpdi->m_tmLastChange, tBefore);

		}
		lock.Unlock();
		BasicSleep(10000);		//I know this
	} while(IsThreadRunning() && !m_ayDirInfo.empty());
}

int CBasicFileNotify::ScanDir(_DIRECTORY_INFO* lpdi, const char* lpszDir, const time_t tmLastChange, time_t& tmCurChange, time_t tmBefore)
{	
	CBasicFileFind tlFinder;
	long lFind = tlFinder.FindFile(lpszDir, "*");
	CBasicString strFileName;
	
	long lResult = 0;
	while(!lFind)
	{
		lFind = tlFinder.FindNextFile();
		strFileName = tlFinder.GetFilePath();
		if(tlFinder.IsDirectory())
		{
			if(lpdi->IsWatchSubTree() && !tlFinder.IsDots())
			{
				strFileName += CBasicString(PATHSPLITSTRING_S);
				lResult += ScanDir(lpdi, strFileName.c_str(), tmLastChange, tmCurChange, tmBefore);
			}
		}
		else
		{
			time_t tmLastWrite = tlFinder.GetLastWriteTime();
			if (tmLastWrite > tmLastChange && tmLastWrite < tmBefore)
			{
				if (tmLastWrite > tmCurChange)
				{
					tmCurChange = tmLastWrite;
				}
				HandleChangeNotify(lpdi, strFileName, FILE_ACTION_MODIFIED);
				lResult++; 
			}
		}
	}
	return lResult;
}

bool CBasicFileNotify::IsValidDirectoryInfo(_DIRECTORY_INFO* lpdi)
{
	return std::find(m_ayDirInfo.begin(), m_ayDirInfo.end(), lpdi) != m_ayDirInfo.end();
}

void CBasicFileNotify::HandleChangeNotify(_DIRECTORY_INFO* lpdi, const CBasicString& strFileName, DWORD dwAction)
{
	if(lpdi->m_lpszFileName[0] != '\0' && !Basic_PathMatchSpec(strFileName.c_str(), lpdi->m_lpszFileName))
	{
		return;
	}

	CBasicString strFullFileName;
	if(IsCompPort())
	{
		strFullFileName = lpdi->m_lpszDirName + strFileName;
	}
	else
	{
		strFullFileName = strFileName;
	}
	if(lpdi->m_pfuncHander != NULL)
	{
		(*lpdi->m_pfuncHander)(lpdi->m_lpVoid, dwAction, strFullFileName.c_str());
	}
	else
	{
		_fni_array::iterator iter = lpdi->m_ayFileInfo.begin();
		_fni_array::iterator iEnd = lpdi->m_ayFileInfo.end();
		for(; iter != iEnd; iter++)
		{
			if(strFullFileName == (*iter).m_szFileName)
			{
				return;
			}
		}
		_FILE_NOTIFY_ITEM item;
		item.m_dwAction = dwAction;
		strcpy(item.m_szFileName, strFullFileName.c_str());
		lpdi->m_ayFileInfo.push_back(item);
	}	
}


DWORD CBasicFileNotify::PopFirstNotifyItem(DWORD dwWatchID, char* lpszFileName, int nMaxLength)
{
	return PopNotifyItem(dwWatchID, lpszFileName, nMaxLength, 0);
}

DWORD CBasicFileNotify::PopLastNotifyItem(DWORD dwWatchID, char* lpszFileName, int nMaxLength)
{
	return PopNotifyItem(dwWatchID, lpszFileName, nMaxLength, 1);
}

DWORD CBasicFileNotify::PopNotifyItem(DWORD dwWatchID, char* lpszFileName, int nMaxLength, int nPos)
{
	_DIRECTORY_INFO* lpdi = (_DIRECTORY_INFO*)dwWatchID;
	CSingleLock lock(&m_mtxAddRemove);
	lock.Lock();
	if(IsValidDirectoryInfo(lpdi) && !lpdi->m_ayFileInfo.empty())
	{
		_fni_array::iterator iter = nPos == 0 ? lpdi->m_ayFileInfo.begin() : lpdi->m_ayFileInfo.end() - 1;
		_FILE_NOTIFY_ITEM item = *iter;
		lpdi->m_ayFileInfo.erase(iter);
		if(lpszFileName != NULL && nMaxLength > 0)
		{
			__tcscpyn(lpszFileName, nMaxLength, item.m_szFileName);
		}
		return item.m_dwAction;
	}
	return 0;
}

long CBasicFileNotify::CheckNotifyItem(DWORD dwWatchID)
{
	_DIRECTORY_INFO* lpdi = (_DIRECTORY_INFO*)dwWatchID;
	CSingleLock lock(&m_mtxAddRemove);
	lock.Lock();
	if(IsValidDirectoryInfo(lpdi))
	{
		return lpdi->m_ayFileInfo.size();
	}
	return 0;
}

void CBasicFileNotify::ClearNotifyItem(DWORD dwWatchID)
{
	_DIRECTORY_INFO* lpdi = (_DIRECTORY_INFO*)dwWatchID;
	CSingleLock lock(&m_mtxAddRemove);
	lock.Lock();
	if(IsValidDirectoryInfo(lpdi))
	{
		lpdi->m_ayFileInfo.clear();
	}
}
__NS_BASIC_END
