#include "../inc/basic.h"
#include "filebase.h"

#define CHECK_OPEN			if(!IsOpen()){ return BASIC_FILE_NOT_OEPN; }
#define CHECK_READONLY		if(IsReadOnly()){ return BASIC_FILE_WRITE_DENIED; }

__NS_BASIC_START

////////////////////////////////////////////////////////////////////////////////////////
//BASIC_IMPLEMENT_DYNCREATE(CWBasicFileObj, CBasicObject)

CBasicFileObj::CBasicFileObj()
{
	ClearMember();
}

CBasicFileObj::CBasicFileObj(void* pBuffer, long lCount)
{
	ClearMember();
	OpenMemFile(pBuffer, lCount, 0);
}
CBasicFileObj::CBasicFileObj(const char* lpszFileName, DWORD dwOpenFlags)
{
	ClearMember();
	Open(lpszFileName, dwOpenFlags);
}
CBasicFileObj::~CBasicFileObj()
{
	Close();
}

BOOL CBasicFileObj::IsOpen() const
{
	if (m_pFileObj == NULL)
	{
		return FALSE;
	}
	return m_pFileObj->IsOpen();
}

BOOL CBasicFileObj::IsMemoryFile() const
{
	return GetFileOpenType() == PF_MEM_FILE;
}

long CBasicFileObj::GetPosition() const
{
	CHECK_OPEN;
	return m_pFileObj->GetPosition();
}

long CBasicFileObj::GetStatus(TLFileStatus& rStatus) const
{
	CHECK_OPEN;
	memset(&rStatus, 0, sizeof(TLFileStatus));
	strcpy(rStatus.m_szFullName, m_strFileName.c_str());
	if (!m_strFileName.IsEmpty())
	{
		rStatus.m_attribute = (BYTE)Basic_GetFileAttributes(m_strFileName.c_str());
	}
	return m_pFileObj->GetStatus(rStatus);
}

CBasicString CBasicFileObj::GetFileDirPath() const
{
	char szDirPath[_MAX_PATH];
	memset(szDirPath, 0, sizeof(szDirPath));
	Basic_GetFileDirPath(m_strFileName.c_str(), szDirPath, _MAX_PATH);
	return szDirPath;
}


CBasicString CBasicFileObj::GetFileName() const
{
	char szName[_MAX_PATH];
	memset(szName, 0, sizeof(szName));
	Basic_GetFileName(m_strFileName.c_str(), szName, _MAX_PATH);
	return szName;
}

CBasicString CBasicFileObj::GetFileTitle() const
{
	char szTitle[_MAX_PATH];
	memset(szTitle, 0, sizeof(szTitle));
	Basic_GetFileTitle(m_strFileName.c_str(), szTitle, _MAX_PATH);
	return szTitle;
}

CBasicString CBasicFileObj::GetFilePath() const
{
	return m_strFileName;
}


long CBasicFileObj::Open(const char* lpszFileName, DWORD dwOpenFlags)
{
	if (IsOpen())
	{
		return BASIC_FILE_ALREADY_OEPN;
	}
	Close();
	if (lpszFileName != NULL && lpszFileName[0] != '\0')
	{
		char szFullPath[_MAX_PATH];
		long lRet = Basic_GetFileFullPath(szFullPath, lpszFileName);
		if (lRet != BASIC_FILE_OK)
		{
			return lRet;
		}
		m_strFileName = szFullPath;
	}
	else
	{
		m_strFileName.Empty();
	}

	m_dwOpenFlags = dwOpenFlags;
	switch (GetFileOpenType())
	{
	case PF_DISK_FILE:
	{
						 m_pFileObj = new CDiskFile;
						 break;
	}
	case PF_MEM_FILE:
	{
						m_pFileObj = new CMemFileBase;
						break;
	}
	default:
	{
			   return BASIC_FILE_NOT_SUPPORT;
	}
	}
	assert(m_pFileObj != NULL);
	return m_pFileObj->Open(m_strFileName.c_str(), dwOpenFlags);
}

long CBasicFileObj::OpenMemFile(void* pBuffer, long lCount, long lLength)
{
	if (IsOpen())
	{
		return BASIC_FILE_ALREADY_OEPN;
	}

	m_dwOpenFlags = PF_MEM_FILE;

	m_pFileObj = new CMemFileBase;

	return m_pFileObj->OpenMemFile(pBuffer, lCount, lLength);
}

long CBasicFileObj::ReOpen()
{
	if (m_strFileName.IsEmpty())
	{
		return BASIC_FILE_BAD_PATH;
	}
	CBasicString strFileName = m_strFileName;
	DWORD dwOpenFlags = m_dwOpenFlags;
	Close();
	return Open(strFileName.c_str(), dwOpenFlags);
}

long CBasicFileObj::SeekToEnd()
{
	return Seek(0, BASIC_FILE_END);
}

long CBasicFileObj::SeekToBegin()
{
	return Seek(0, BASIC_FILE_BEGIN);
}

long CBasicFileObj::Seek(long lOff, uint32_t nFrom)
{
	CHECK_OPEN;
	return m_pFileObj->Seek(lOff, nFrom);
}

long CBasicFileObj::SetLength(long lNewLen, char cFill)
{
	CHECK_OPEN;
	CHECK_READONLY;
	if (GetFileOpenFlags(PF_MEM_ONLY))
	{
		return BASIC_FILE_SETLEN_DENIED;
	}
	return m_pFileObj->SetLength(lNewLen, cFill);
}

long CBasicFileObj::GetLength()
{
	CHECK_OPEN;
	return m_pFileObj->GetLength();
}

long CBasicFileObj::Read(void* lpBuf, long lCount)
{
	CHECK_OPEN;
	if (lpBuf == NULL)
	{
		return BASIC_FILE_NO_BUFFER;
	}
	return m_pFileObj->Read(lpBuf, lCount);
}

long CBasicFileObj::Write(const void* lpBuf, long lCount, long lRepeat, char cFill)
{
	CHECK_OPEN;
	CHECK_READONLY;
	if (lRepeat <= 0)
	{
		lRepeat = 1;
	}
	return m_pFileObj->Write(lpBuf, lCount, lRepeat, cFill);
}

long CBasicFileObj::Insert(const void* lpBuf, long lCount, long lRepeat, char cFill)
{
	CHECK_OPEN;
	CHECK_READONLY;
	if (lRepeat <= 0)
	{
		lRepeat = 1;
	}
	long lTotal = lCount * lRepeat;
	if (lCount == 0)
	{
		return 0;
	}
	else if (lCount > 0)
	{
		if (lTotal <= 0)
		{
			return BASIC_FILE_TOO_LARGE;
		}
	}
	else if (lTotal > 0)
	{
		return BASIC_FILE_TOO_LARGE;
	}

	long lLength = GetLength();
	long lPosition = GetPosition();

	if (lLength < 0 || lPosition < 0)
	{
		return 0;
	}
	long lLeft = lLength - lPosition;

	if (lLeft > 0)
	{
		long lRet = 0;
		CMemFileBase mfTemp;
		if (mfTemp.OpenMemFile(NULL, 0, lLeft) == BASIC_FILE_OK)
		{
			char* pTemp = (char*)mfTemp.GetDataBuffer();
			if (pTemp != NULL)
			{
				lRet = Read(pTemp, lLeft);
				if (lRet == lLeft)
				{
					long lNewpos = lPosition + lTotal;
					if (lNewpos < 0)
					{
						lNewpos = 0;
					}
					lRet = Seek(lNewpos, BASIC_FILE_BEGIN);
					if (lRet == lNewpos)
					{
						lRet = Write(pTemp, lLeft);
						if (lRet == lLeft)
						{
							SetLength(lNewpos + lLeft, 0);
						}
					}
				}
			}
			Seek(lPosition, FILE_BEGIN);
		}
		if (lRet < 0)
		{
			return lRet;
		}
	}
	else if (lCount < 0)
	{
		long lNewLength = lLength + lTotal;
		if (lNewLength < 0)
		{
			lNewLength = 0;
		}
		SetLength(lNewLength, 0);
	}
	if (lCount < 0)
	{
		return lTotal;
	}
	return Write(lpBuf, lCount, lRepeat, cFill);
}

long CBasicFileObj::Flush()
{
	CHECK_OPEN;
	CHECK_READONLY;
	return m_pFileObj->Flush();
}


void CBasicFileObj::Close()
{
	if (m_pFileObj != NULL)
	{
		m_pFileObj->Close();
		delete m_pFileObj;
		m_pFileObj = NULL;
	}
}

long CBasicFileObj::CopyTo(const char* lpszFileName)
{
	CHECK_OPEN;
	return m_pFileObj->CopyTo(m_strFileName.c_str(), lpszFileName);
}

long CBasicFileObj::CopyFrom(const char* lpszFileName)
{
	CHECK_OPEN;
	return m_pFileObj->CopyFrom(m_strFileName.c_str(), lpszFileName);
}


void* CBasicFileObj::GetDataBuffer()
{
	return IsOpen() ? m_pFileObj->GetDataBuffer() : NULL;
}

void* CBasicFileObj::GetCurDataBuffer(long& lCount)
{
	return IsOpen() ? m_pFileObj->GetCurDataBuffer(lCount) : NULL;
}

void CBasicFileObj::ClearMember()
{
	m_dwOpenFlags = 0;
	m_pFileObj = NULL;
}

#define _RW_MAX_SIZE		(1 * 1024 * 1024)		//一次写入(读出)的最大长度，超出这个长度要分批处理。
long CBasicFileObj::CopyFileContent(CBasicFileObj* pFile)
{
	long lTotalSize = pFile->GetLength();
	if (lTotalSize <= 0)
	{
		return 0;
	}
	long lReadCount = _RW_MAX_SIZE;
	long lRepeat = lTotalSize / lReadCount;
	long lLeft = lTotalSize % lReadCount;

	char* pBuffer = (char*)BasicAllocate(lReadCount);
	if (pBuffer == NULL)
	{
		return BASIC_FILE_NO_MEMORY;
	}

	SeekToBegin();
	pFile->SeekToBegin();
	for (int i = 0; i < lRepeat; i++)
	{
		memset(pBuffer, 0, lReadCount);
		pFile->Read(pBuffer, lReadCount);
		Write(pBuffer, lReadCount);
	}
	if (lLeft != 0)
	{
		memset(pBuffer, 0, lLeft);
		pFile->Read(pBuffer, lLeft);
		Write(pBuffer, lLeft);
	}

	BasicDeallocate(pBuffer);
	return lTotalSize;
}

long CBasicFileObj::CopyFromFile(CBasicFileObj* pFile)
{
	CHECK_OPEN;
	if (pFile == NULL || !pFile->IsOpen())
	{
		return BASIC_FILE_INVALID;
	}
	SetLength(0);
	return CopyFileContent(pFile);
}

long CBasicFileObj::OpenMemFromFile(CBasicFileObj* pFile)
{
	if (IsOpen())
	{
		return BASIC_FILE_ALREADY_OEPN;
	}
	if (pFile == NULL || !pFile->IsOpen())
	{
		return BASIC_FILE_INVALID;
	}

	return OpenMemFile(pFile->m_pFileObj, 0, -2);
}

//long CWBasicFileObj::CloneToMemFile(CWBasicFileObj*& pNewFile)
//{
//	if(!IsValidFile())
//	{
//		return BASIC_FILE_INVALID;
//	}
//
//	pNewFile = CreateFileObj();
//	long lRet = pNewFile->OpenMemFromFile(this);
//	if(lRet < 0)
//	{
//		delete pNewFile;
//		pNewFile = NULL;
//	}
//
//	return lRet;
//}

//long CWBasicFileObj::OpenCloneFile(LPCTSTR lpszFileName, CWBasicFileObj*& pNewFile, DWORD dwOpenFlags)
//{
//	pNewFile = CreateFileObj();
//	if(dwOpenFlags == 0)
//	{
//		dwOpenFlags = m_dwOpenFlags;
//	}
//	long lRet = pNewFile->Open(lpszFileName, dwOpenFlags);
//	if(lRet == BASIC_FILE_OK && (dwOpenFlags != 0 || pNewFile->IsValidFile()))
//	{
//		return BASIC_FILE_OK;
//	}
//
//	delete pNewFile;
//	pNewFile = NULL;
//
//	return BASIC_FILE_GENERIC_ERROR;
//}

long CBasicFileObj::Rename(const char* lpszOldName, const char* lpszNewName)
{
	return Basic_RenameFile(lpszOldName, lpszNewName);
}

long CBasicFileObj::Remove(const char* lpszFileName)
{
	return Basic_DeleteFile(lpszFileName);
}

long CBasicFileObj::GetStatus(const char* lpszFileName, TLFileStatus& rStatus)
{
	return Basic_GetFileStatus(lpszFileName, rStatus);
}

long CBasicFileObj::SetStatus(const char* lpszFileName, const TLFileStatus& rStatus)
{
	return Basic_SetFileStatus(lpszFileName, rStatus);
}
////////////////////////////////////////////////////////////////////////////////////////


__NS_BASIC_END
