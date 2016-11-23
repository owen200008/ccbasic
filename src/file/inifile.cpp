#include "../inc/basic.h"
__NS_BASIC_START

class CWriteToFile
{
public:
	CWriteToFile(FILE* fp)
		: m_fp(fp)
	{

	}
	long funcWrite(const char* pszData, long lLength)
	{
		fwrite(pszData, 1, lLength, m_fp);
		return 0;
	}
protected:
	FILE* m_fp;
};

//!get size
int CBasicIniOp::GetIniSize()
{
    return sizeof(CBasicIniOp);
}

void CBasicIniOp::Empty()
{
    m_file.Empty();
    m_buffer.Free();

	m_dic.erase(m_dic.begin(), m_dic.end());
	m_bModified = false;
}

int CBasicIniOp::InitFromMem(const char* lpszData, size_t cbData)
{
	Empty();
	return _InitData(lpszData, cbData);
}

int CBasicIniOp::InitFromFile(const char* filename)
{
	Empty();

	FILE* fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		return -1;
	}
	m_file = filename;

	fseek(fp, 0, SEEK_END);
	long lSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	m_tmp.SetDataLength(lSize);
	long lRead = fread(m_tmp.GetDataBuffer(), 1, lSize, fp);
	fclose(fp);

	int nLength = lSize;
	m_tmp.SetDataLength(nLength);
	
	int nRet = _InitData((const char*)m_tmp.GetDataBuffer(), m_tmp.GetDataLength());

	m_tmp.Free();
	return nRet;
}

bool CBasicIniOp::EmptySection(const char* lpszSection)
{
	if (lpszSection == 0)
	{
		m_buffer.Free();
		m_dic.erase(m_dic.begin(), m_dic.end());
		m_bModified = true;
		return true;
	}
	ASSERT(lpszSection);
	CBasicString strSection = lpszSection;
	strSection.MakeUpper();
	Dict::iterator iter = m_dic.find(strSection);
	if (iter == m_dic.end())
		return false;

	m_bModified = true;
	//
	IndexData i;
	i.m_nData64 = iter->second;
	AddNewData(i.m_nOffset, i.m_nSectionLength, "", 0);


	CBasicString strSectionBegin = strSection + SEP_SIGN;
	for (Dict::iterator iter = m_dic.begin(); iter != m_dic.end();)
	{
		CBasicString strKey = iter->first;
		if (strKey == strSection || strKey.Find(strSectionBegin.c_str()) == 0){
			m_dic.erase(iter++);
		}
		else{
			++iter;
		}
	}

	UpdateOffset(i.m_nOffset, -1 * i.m_nSectionLength);
	return true;
}


int CBasicIniOp::InsertBefore(const char* key1, const char* key2, const char* value, const char* findkey2)
{
	if (key1 == NULL || key2 == NULL || value == NULL || findkey2 == NULL)
		return -1;

	SetData(key1, key2, NULL);

	CBasicString strSection = key1;
	CBasicString strfindKey2 = findkey2;

	CBasicString strKey = strSection + SEP_SIGN + strfindKey2;
	strKey.MakeUpper();

	IndexData i;
	Dict::iterator iter = m_dic.find(strKey);
	if (iter != m_dic.end())
	{
		i.m_nData64 = iter->second;

		m_tmp.SetDataLength(0);

		long  lKeyLen = strlen(key2);
		long  lValLen = strlen(value);
		m_tmp.AppendData((const char*)key2, lKeyLen * sizeof(char));
		m_tmp.AppendData((const char*)"=", sizeof(char));
		m_tmp.AppendData((const char*)value, lValLen * sizeof(char));
		m_tmp.AppendData((const char*)NEW_LINE, NEW_LINE_LEN);

		long  nDeltLen = m_tmp.GetDataLength() / sizeof(char);
		AddNewData(i.m_nOffset, 0, m_tmp.GetDataBuffer(), nDeltLen);

		this->UpdateOffset(i.m_nOffset - 1, nDeltLen);

		i.m_nKeyLen = lKeyLen;
		i.m_nValueLen = lValLen;

		CBasicString strKey2 = key2;
		strKey = strSection + SEP_SIGN + strKey2;
		strKey.MakeUpper();
		m_dic[strKey] = i.m_nData64;
	}
	else	
	{
		SetData(key1, key2, value);
	}
	return 0;
}


bool CBasicIniOp::WriteToFile(const char* filename)
{
	if (filename == NULL)
	{
		if (!IsModified())	
		{
			return true;
		}
		filename = m_file.c_str();
	}

	FILE* fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		return false;
	}

	CWriteToFile writeFile(fp);
	_WriteTo([&](const char* pszData, long lLength)->long{
		fwrite(pszData, 1, lLength, fp);
		return 0;
	});

	fclose(fp);
	return true;
}


bool CBasicIniOp::ParseCombine(long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End)
{
	if (nType == INI_TYPE_KEY)
	{
		CBasicString strName(lpszData + nV1Begin, nV1End - nV1Begin);
		CBasicString strSection = lpszSection;
		CBasicString strValue(lpszData + nV2Begin, nV2End - nV2Begin);
		strSection.TrimLeft();
		strSection.TrimRight();
		strName.TrimLeft();
		strName.TrimRight();

		_SetData(strSection.c_str(), strName.c_str(), strValue.c_str());
	}
	return true;
}


bool CBasicIniOp::ParseINI(long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End)
{
	if (nType == INI_TYPE_KEY)
	{
		CBasicString strName(lpszData + nV1Begin, nV1End - nV1Begin);
		CBasicString strSection = lpszSection;
		strSection.TrimLeft();
		strSection.TrimRight();
		strName.TrimLeft();
		strName.TrimRight();
		CBasicString strKey = strSection + SEP_SIGN + strName;
		strKey.MakeUpper();

		IndexData i;
		i.m_nOffset = nV1Begin;
		i.m_nKeyLen = nV1End - nV1Begin;
		i.m_nValueLen = nV2End - nV2Begin;

		m_dic[strKey] = i.m_nData64;
	}
	else if (nType == INI_TYPE_SCETION_END)
	{
		CBasicString strKey = lpszSection;
		strKey.TrimLeft();
		strKey.TrimRight();
		strKey.MakeUpper();

		IndexData i;
		i.m_nOffset = nV2Begin;
		i.m_nSectionLength = nV2End - nV2Begin;
		m_dic[strKey] = i.m_nData64;
	}

	return true;
}
void CBasicIniOp::_SetData(const char* key1, const char* key2, const char* value)
{
	if (key1 == 0)
		return;

	m_bModified = true;	

	if (key2 == 0)	//
	{
		EmptySection(key1);
		return;
	}

	CBasicString strSection = key1;
	CBasicString strKey2 = key2;

	CBasicString strKey = strSection + SEP_SIGN + strKey2;
	strKey.MakeUpper();
	strSection.MakeUpper();

	IndexData i;
	Dict::iterator iter = m_dic.find(strKey);
	if (iter != m_dic.end())
	{
		long nDeltLen = 0;	//
		i.m_nData64 = iter->second;
		do
		{
			//
			if (value == 0)
			{
				char* pszData = GetDataBuffer() + i.m_nOffset + i.m_nKeyLen + i.m_nValueLen;
				int nIndex = 0;
				while (*(pszData + nIndex) != '\n')
				{
					++nIndex;
				}

				long nDelLen = i.m_nKeyLen + i.m_nValueLen + 1 + nIndex;
				AddNewData(i.m_nOffset, nDelLen, "", 0);
				m_dic.erase(iter);

				nDeltLen = -1 * nDelLen;
				UpdateOffset(i.m_nOffset, nDeltLen);
			}
			else
			{
				//
				long nNewLen = strlen(value);	//		
				nDeltLen = nNewLen - i.m_nValueLen; //

				DWORD dwDataBegin = i.m_nOffset + i.m_nKeyLen + 1;
				AddNewData(dwDataBegin, i.m_nValueLen, value, nNewLen);

				//
				UpdateOffset(i.m_nOffset, nDeltLen);

				//
				i.m_nValueLen = nNewLen;
				iter->second = i.m_nData64;
			}
		} while (0);

		
		DWORD dwDataOffset = i.m_nOffset;
		Dict::iterator si = m_dic.find(strSection);
		ASSERT(si != m_dic.end());
		if (si != m_dic.end())
		{
			i.m_nData64 = si->second;
			if (i.m_nOffset < dwDataOffset)	//
			{
				i.m_nSectionLength += nDeltLen;
				si->second = i.m_nData64;
			}
		}
	}
	else if (value == 0)
	{
		return;
	}
	else	//
	{
		long  lKeyLen = strlen(key2);
		long  lValLen = strlen(value);

		//				
		m_tmp.SetDataLength(0);
		if (lKeyLen > 0)
		{
			m_tmp.AppendData((const char*)key2, lKeyLen * sizeof(char));
			m_tmp.AppendData((const char*)"=", sizeof(char));
			m_tmp.AppendData((const char*)value, lValLen * sizeof(char));
			m_tmp.AppendData((const char*)NEW_LINE, NEW_LINE_LEN);
		}
		//
		long  nNewLen = m_tmp.GetDataLength() / sizeof(char);

		Dict::iterator si = m_dic.find(strSection);
		if (si == m_dic.end())
		{
			CBasicSmartBuffer buf;
			buf.AppendData((const char*)"[", sizeof(char));
			buf.AppendData((const char*)key1, strlen(key1)*sizeof(char));
			buf.AppendData((const char*)"]", sizeof(char));
			buf.AppendData((const char*)NEW_LINE, NEW_LINE_LEN);
			buf.AppendData(m_tmp.GetDataBuffer(), m_tmp.GetDataLength());

			InitData(buf.GetDataBuffer(), buf.GetDataLength() / sizeof(char));
		}
		else if (nNewLen > 0) 
		{
			i.m_nData64 = si->second;

			DWORD dwDataBegin = i.m_nOffset + i.m_nSectionLength;

			AddNewData(dwDataBegin, 0, m_tmp.GetDataBuffer(), nNewLen);

			i.m_nSectionLength += nNewLen;
			si->second = i.m_nData64;

			UpdateOffset(dwDataBegin - 1, nNewLen);

			i.m_nOffset = dwDataBegin;
			i.m_nKeyLen = lKeyLen;
			i.m_nValueLen = lValLen;
			m_dic[strKey] = i.m_nData64;
			//				
		}
	}
}

void CBasicIniOp::UpdateOffset(DWORD dwBeginOffset, long nMove)
{
	IndexData i;
	for (Dict::iterator iter = m_dic.begin(); iter != m_dic.end(); ++iter)
	{
		i.m_nData64 = iter->second;
		if (i.m_nOffset > dwBeginOffset)
		{
			i.m_nOffset += nMove;
			iter->second = i.m_nData64;
		}
	}
}

void CBasicIniOp::AddNewData(DWORD dwStartPos, long cbOldData, const char* lpszNewData, long cbData)
{
	dwStartPos *= sizeof(char);
	cbOldData *= sizeof(char);
	cbData *= sizeof(char);

	long nDeltLen = cbData - cbOldData;	
	long nOldAllLen = m_buffer.GetDataLength();
	m_buffer.CommitData(nDeltLen);
	char* pszData = m_buffer.GetDataBuffer();

	//œ»∞—‘≠¿¥µƒ ˝æ›“∆∂ØµΩŒª
	DWORD dwDataBegin = dwStartPos;
	DWORD dwMoveLen = nOldAllLen - dwDataBegin - cbOldData;
	memmove(pszData + dwDataBegin + cbData, pszData + dwDataBegin + cbOldData, dwMoveLen);

	memcpy(pszData + dwDataBegin, lpszNewData, cbData);
	//	
	m_buffer.AppendDataEx(NULL, 2);
}


int CBasicIniOp::_InitData(const char* lpszData, size_t cbData)
{
	return InitData(lpszData, cbData);
}


void CBasicIniOp::_WriteTo(const std::function<long(const char*, long)>& func)
{
	int nLen = GetDataLength();
	const char* lpsz = GetDataBuffer();

	func(lpsz, nLen);
}

int CBasicIniOp::InitData(const char* lpszData, size_t cbData)
{
	if (cbData == 0)
	{
		return 0;
	}

	size_t nOldLen = GetDataLength();
	m_buffer.AppendData((const char *)lpszData, cbData * sizeof(char));
	size_t lSize = GetDataLength();
	if (GetDataBuffer()[lSize - 1] != '\n')
	{
		m_buffer.AppendData((const char*)NEW_LINE, NEW_LINE_LEN);
		lSize += (NEW_LINE_LEN / sizeof(char));
	}
	m_buffer.AppendData(NULL, 2);

	return ParseIniChar(GetDataBuffer(), lSize, [&](long nType, const char* lpszSection, const char* lpszData, long nV1Begin, long nV1End, long nV2Begin, long nV2End)->bool{
		return ParseINI(nType, lpszSection, lpszData, nV1Begin, nV1End, nV2Begin, nV2End);
	}, nOldLen);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BASICWINDOWS
static CMutex   g_IniMutex;	
static CMutex*  GetIniMutex()
{	
	return &g_IniMutex;
}
BOOL WritePrivateProfileString(LPCTSTR lpszAppName, LPCTSTR lpszKeyName, LPCTSTR lpszString, LPCTSTR lpszFilename)
{
	CSingleLock lock(GetIniMutex());
	if(!lock.Lock(3000))
		return FALSE;

	CWBasicIniOp ini(lpszFilename);
	ini.SetData(lpszAppName, lpszKeyName, lpszString);
	return ini.WriteToFile(lpszFilename);
}

DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName,LPCTSTR lpDefault,LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	CSingleLock lock(GetIniMutex());
	if(!lock.Lock(3000))
		return FALSE;

	CWBasicIniOp ini(lpFileName);
	return _snprintf(lpReturnedString, nSize, "%s", ini.GetData(lpAppName, lpKeyName, lpDefault).c_str());
}

int   GetPrivateProfileInt( LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName)
{
	CSingleLock lock(GetIniMutex());
	if(!lock.Lock(3000))
		return FALSE;

	char szBuf[128];
	_snprintf(szBuf, sizeof(szBuf), "%d", nDefault);
	CWBasicIniOp ini(lpFileName);
	return ini.GetLong(lpAppName, lpKeyName, szBuf);
}
#endif

__NS_BASIC_END
