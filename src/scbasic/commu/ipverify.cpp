#include "ipverify.h"

//////////////////////////////////////////////////////////////////////////
#define IP_LOOPADDR					"127.0.0.1"				// �ػ�IP
#define IP_LOOPMASK					"255.0.0.0"				// �ػ�IP������
#define IP_RULER_LOCAL_LAN			"$LOCAL_LAN"			// ������������


//////////////////////////////////////////////////////////////////////////
// CIpverify

CIpVerify::CIpVerify()
{
	m_bSupportAll = TRUE;
}

CIpVerify::~CIpVerify()
{
	EmptyRuler();
}


int CIpVerify::SetIPRuler(const char* lpszRuler)
{
	int nRet = 0;
	EmptyRuler();
	m_bSupportAll = FALSE;
	basiclib::CBasicStringArray ayRuler;
	basiclib::BasicSpliteString(lpszRuler, ';', basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayRuler));
	int nRulerCount = ayRuler.GetSize();
	int nRulerIndex = 0;
	for (; nRulerIndex < nRulerCount; nRulerIndex++)
	{
		basiclib::CBasicString strRuler = ayRuler.GetAt(nRulerIndex);
		if (strRuler.CompareNoCase("*") == 0)		// ����ȫ��
		{
			m_bSupportAll = TRUE;
			nRet++;
			continue;
		}
		if (strRuler.CompareNoCase(IP_RULER_LOCAL_LAN) == 0)		// Ĭ�ϱ�����������
		{
			basiclib::LOCALADDR addr[20];
			memset(addr, 0, sizeof(basiclib::LOCALADDR) * 20);
			int nTmp1 = BasicGetLocalAddrInfo(addr, 10);
			basiclib::__tcscpyn(addr[nTmp1].m_szIP, MAX_IP, IP_LOOPADDR);
			basiclib::__tcscpyn(addr[nTmp1].m_szMask, MAX_IP, IP_LOOPMASK);
			nTmp1++;
			int i;
			for (i = 0; i < nTmp1; i++)
			{
				nRet = AddIPRuler(addr[i].m_szIP, addr[i].m_szMask) ? nRet + 1 : nRet;
			}
			continue;
		}
		basiclib::CBasicString strIP, strMask;
		int nLen = strRuler.GetLength();
		int nPos1 = strRuler.Find(':');
		int nPos2 = strRuler.Find('@');
		if (nPos1 > 0)		// �ҵ�������
		{
			strIP = strRuler.Mid(0, nPos1);
			strMask = strRuler.Mid(nPos1+1, nPos2);
		}
		else if (nPos2 > 0)	// �ҵ�������,������
		{
			strIP = strRuler.Mid(0, nPos2);
		}
		else		// �ҵ�������
		{
			strIP = strRuler;
		}
		if (AddIPRuler(strIP.c_str(), strMask.c_str()))
		{
			nRet++;
		}
	}
	return nRet;
}

//************************************************************************
// Method:    IsIpTrust => ��֤ĳһIP��ַ 1���Ƚ�IP  2����������Ƚ�
// Returns:   BOOL => 
// Parameter: LPCTSTR lpszIP => IP��ֵַ
//************************************************************************
BOOL CIpVerify::IsIpTrust(const char* lpszIP)
{
	BOOL bRet = FALSE;
	BYTE szIP[MAX_IP_ITEM];
	memset(szIP, 0, MAX_IP_ITEM);
	if (m_bSupportAll)
	{
		bRet = TRUE;
	}
	else if (strlen(lpszIP) > 0 && IsIPAddr(lpszIP, szIP, MAX_IP_ITEM))
	{
		basiclib::CSingleLock lock(&m_synObj);
		lock.Lock();
		POSITION pos = m_lsIpRuler.GetHeadPosition();
		while (pos != NULL)
		{
			_IPRuler* pRuler = (_IPRuler*)m_lsIpRuler.GetNext(pos);
			if (pRuler == NULL)
			{
				continue;
			}
			if (memcmp(szIP, pRuler->m_szIP, MAX_IP_ITEM) == 0)		// �Ƚ�IP��ַ
			{
				bRet = TRUE;
				break;
			}
			else if(pRuler->m_szMask[0] != '\0')			// �ж���������
			{
				BYTE szTmp[MAX_IP_ITEM];
				memset(szTmp, 0, MAX_IP_ITEM);
				int j = 0;
				for(; j < MAX_IP_ITEM; j++)
				{
					szTmp[j] = szIP[j] & pRuler->m_szMask[j];
				}
				if (memcmp(szTmp, pRuler->m_szResult, MAX_IP_ITEM) == 0)
				{
					bRet = TRUE;
					break;
				}
			}
		}
		lock.Unlock();
	}
	return bRet;
}

//************************************************************************
// Method:    AddIPRuler => ����IP��Ϣ�������б�
// Returns:   BOOL => 
// Parameter: LPCTSTR lpszIP => IP
// Parameter: LPCTSTR lpszMask => ��������
//************************************************************************
BOOL CIpVerify::AddIPRuler(const char* lpszIP, const char* lpszMask)
{
	BYTE szIP[MAX_IP_ITEM];
	memset(szIP, 0, MAX_IP_ITEM);
	if (basiclib::__tcslen(lpszIP) <= 0 || !IsIPAddr(lpszIP, szIP, MAX_IP_ITEM))
	{
		return FALSE;
	}
	//_IPRuler* pRuler = (_IPRuler*)basiclib::TL_NewObject<_IPRuler>();
	_IPRuler* pRuler = new _IPRuler;
	ASSERT(pRuler != NULL);
	if (pRuler == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}
	memcpy(pRuler->m_szIP, szIP, MAX_IP_ITEM);
	basiclib::CSingleLock lock(&m_synObj);
	lock.Lock();
	m_lsIpRuler.AddTail(pRuler);
	// ��������
	memset(szIP, 0, MAX_IP_ITEM);
	if (lpszMask != NULL && basiclib::__tcslen(lpszMask) > 0 && IsIPAddr(lpszMask, szIP, MAX_IP_ITEM))
	{
		memcpy(pRuler->m_szMask, szIP, MAX_IP_ITEM);
		// ������
		int j=0;
		for (; j < MAX_IP_ITEM; j++)
		{
			pRuler->m_szResult[j] = pRuler->m_szIP[j] & pRuler->m_szMask[j];
		}
	}
	lock.Unlock();
	return TRUE;
}

//************************************************************************
// Method:    IsIPAddr => ���IP��ַ�Ƿ�Ϸ����Ϸ���4����ֵ����
// Returns:   BOOL => 
// Parameter: LPCTSTR lpszIP => IP��ַ
// Parameter: BYTE * szIP => ����IP4���ֵ��ڴ�
// Parameter: int cbIP => �ڴ��С
//************************************************************************
BOOL CIpVerify::IsIPAddr(const char* lpszIP, BYTE* szIP, int cbIP)
{
	BOOL bRet = FALSE;
	basiclib::CBasicStringArray ayItem;
	basiclib::BasicSpliteString(lpszIP, '.', basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayItem));
	int nCount = ayItem.GetSize();
	if (nCount == 4)
	{
		int nIndex = 0;
		for (; nIndex < nCount; nIndex++)
		{
			// �����Ƿ���Ч
			basiclib::CBasicString strPart = ayItem.GetAt(nIndex);
			int nTmp = strPart.GetLength();
			if (nTmp <= 0 || nTmp > 3)
			{
				bRet = FALSE;
				break;
			}
			// IP�����������Ƿ���Ч
			BOOL bFlag = FALSE;
			int i;
			for (i = 0; i < nTmp; i++)
			{
				char ch = strPart.GetAt(i);
				if (ch < '0' || ch > '9')
				{
					bFlag = TRUE;
					break;
				}
			}
			// Ip������ֵ�Ƿ���Ч
			nTmp = atoi(strPart.c_str());
			if (nTmp > 255 || bFlag)
			{
				bRet = FALSE;
				break;
			}
			if (nIndex <= cbIP)
			{
				szIP[nIndex] = (BYTE)nTmp;
			}
			bRet = TRUE;
		}
	}
	return bRet;
}

//************************************************************************
// Method:    EmptyRuler => ���IP�����б�
// Returns:   void => 
//************************************************************************
void CIpVerify::EmptyRuler()
{
	basiclib::CSingleLock lock(&m_synObj);
	lock.Lock();
	POSITION pos = m_lsIpRuler.GetHeadPosition();
	while (pos != NULL)
	{
		_IPRuler* pRuler = (_IPRuler*)m_lsIpRuler.GetNext(pos);
		if (pRuler != NULL)
		{
			delete pRuler;
			pRuler = NULL;
			//basiclib::TL_DeleteObject(pRuler);
		}
	}
	m_lsIpRuler.RemoveAll();
	lock.Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////
CBasicStringCmpInfo::CBasicStringCmpInfo()
{

}

CBasicStringCmpInfo::~CBasicStringCmpInfo()
{

}

#define ALL_INFO_CMP   "*"
//���ù���
long CBasicStringCmpInfo::InitRuleInfo(const char* lpszRule, const char* lpszSplit)
{
	basiclib::CBasicStringArray ayRuleInfo;
	basiclib::BasicSpliteString(lpszRule, lpszSplit, basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayRuleInfo));
	int nSize = ayRuleInfo.GetSize();
	for (int i = 0; i < nSize;i++)
	{
		basiclib::CBasicString& strTmp = ayRuleInfo[i];
		AddRuleInfo(strTmp);
	}
	return m_ayRuleInfo.size();
}

//�������
BOOL CBasicStringCmpInfo::AddRuleInfo(basiclib::CBasicString& strTmp)
{
	strTmp.TrimLeft();
	strTmp.TrimRight();
	if (strTmp.IsEmpty())
	{
		return FALSE;
	}
	while (strTmp.Replace("**", ALL_INFO_CMP) > 0)
	{}

	basiclib::CBasicStringArray addAyInfo;
	int nLocation = strTmp.Find(ALL_INFO_CMP);
	if(nLocation == 0)
	{
		addAyInfo.Add(ALL_INFO_CMP);
		strTmp = strTmp.Mid(1);
	}
	nLocation = strTmp.Find(ALL_INFO_CMP);
	while(nLocation > 0)
	{
		basiclib::CBasicString strFirst = strTmp.Mid(0, nLocation);
		addAyInfo.Add(strFirst.c_str());
		strTmp = strTmp.Mid(nLocation + 1);
		nLocation = strTmp.Find(ALL_INFO_CMP);
	}
	if (strTmp.IsEmpty())
	{
		addAyInfo.Add(ALL_INFO_CMP);
	}
	else
	{
		addAyInfo.Add(strTmp.c_str());
	}
	m_ayRuleInfo.push_back(addAyInfo);
	return TRUE;
}

BOOL IsTrustInfo(basiclib::CBasicString& strInfo, basiclib::CBasicStringArray& ayRule)
{
	int nSize = ayRule.GetSize();
	if (nSize <= 0)
	{
		return FALSE;
	}
	int nStart = 0;
	if (ayRule.GetAt(0) == ALL_INFO_CMP)
	{
		nStart = 1;
	}
	for (;nStart < nSize;nStart++)
	{
		basiclib::CBasicString& strTmp = ayRule[nStart];
		if (strTmp == ALL_INFO_CMP && nStart == nSize - 1)
		{
			break;
		}
		int nLength = strTmp.GetLength();
		int nLocation = strInfo.Find(strTmp.c_str());
		if (nLocation < 0)
		{
			return FALSE;
		}
		if (nStart == 0)
		{
			if (nLocation != 0)
			{
				return FALSE;
			}
		}
		strInfo = strInfo.Mid(nLocation + nLength);
	}
	return TRUE;
}

//�ж��Ƿ��ڷ�Χ��
long CBasicStringCmpInfo::IsInRule(const char* lpszData, long lIndex)
{
	if (!lpszData)
	{
		return -1;
	}
	int nLength = strlen(lpszData);
	int nSize = m_ayRuleInfo.size();
	if (nSize <= 0)
	{
		return -1;
	}
	basiclib::CBasicString strContent = lpszData;
	for (int i = lIndex;i < nSize; i++)
	{
		if (IsTrustInfo(strContent, m_ayRuleInfo[i]))
		{
			return i;
		}
	}
	return -1;
}

basiclib::CBasicString GetStringInfo(basiclib::CBasicStringArray& ayInfo)
{
	basiclib::CBasicString strRet;
	for (int i = 0;i < ayInfo.GetSize();i++)
	{
		strRet += ayInfo[i];
	}
	while (strRet.Replace("**", ALL_INFO_CMP) > 0)
	{}
	return strRet;
}

//��ȡ״̬
void CBasicStringCmpInfo::GetStatus(basiclib::CBasicString& strInfo)
{
	if (!m_strInfo.IsEmpty())
	{
		strInfo += m_strInfo;
		return;
	}
	ContainRuleInfoIterator iter = m_ayRuleInfo.begin();
	for (; iter != m_ayRuleInfo.end();iter++)
	{
		m_strInfo += GetStringInfo(*iter) + "\r\n";
	}
	strInfo += m_strInfo;
}


//////////////////////////////////////////////////////////////////////////////////////////
CIpDomainVerify::CIpDomainVerify()
{
	m_bTrustAll = TRUE;
}

CIpDomainVerify::~CIpDomainVerify()
{

}

//���ù���
long CIpDomainVerify::InitRuleInfo(const char* lpszRule, const char* lpszSplit)
{
	//������֮��Ͳ�ȫ������
	m_bTrustAll = FALSE;

	basiclib::CBasicStringArray ayRuleInfo;
	basiclib::BasicSpliteString(lpszRule, lpszSplit, basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayRuleInfo));
	int nSize = ayRuleInfo.GetSize();
	for (int i = 0; i < nSize;i++)
	{
		basiclib::CBasicStringArray ayItem;
		basiclib::CBasicString& strTmp = ayRuleInfo[i];
		basiclib::BasicSpliteString(strTmp.c_str(), ":", basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayItem));
		if (ayItem.GetSize() < 2)
		{
			continue;
		}
		if(AddRuleInfo(ayItem[0]))
		{
			basiclib::CBasicStringArray ayPort;
			ContainPort wordInfo;
			BasicSpliteString(ayItem[1].c_str(), ",",  basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayPort));
			if (ayPort.GetSize() > 0)
			{
				for (int j = 0;j < ayPort.GetSize();j++)
				{
					WORD wPort = (WORD)atol((ayPort[j]).c_str());
					wordInfo.push_back(wPort);
				}
			}
			else
			{
				wordInfo.push_back(0);
			}

			m_ayRuleExtre.push_back(wordInfo);
		}
	}

	return m_ayRuleInfo.size();
}

//��ȡ״̬
void CIpDomainVerify::GetStatus(basiclib::CBasicString& strInfo)
{
	if(m_bTrustAll)
	{
		strInfo += "TrustAll\r\n";
	}
	if (!m_strInfo.IsEmpty())
	{
		strInfo += m_strInfo;
		return;
	}
	int nSize = m_ayRuleInfo.size();
	for (int i = 0;i < nSize; i++)
	{
		m_strInfo += GetStringInfo(m_ayRuleInfo[i]) + ": " + GetPortInfo(i) + "\r\n";
	}
	strInfo += m_strInfo;
}

//
basiclib::CBasicString CIpDomainVerify::GetPortInfo(int nIndex)
{
	basiclib::CBasicString strRet;
	if (nIndex < 0 || nIndex >= (int)m_ayRuleExtre.size())
	{
		return strRet;
	}
	ContainPort& wordInfo = m_ayRuleExtre[nIndex];
	for (int i = 0;i < (int)wordInfo.size();i++)
	{
		basiclib::CBasicString strTmp;
		if (wordInfo[i] == 0)
		{
			strTmp = "*,";
		}
		else
		{
			strTmp.Format("%d,", wordInfo[i]);
		}
		strRet += strTmp;
	}
	return strRet;
}

//�Ƿ�������IP�Ͷ˿�
BOOL CIpDomainVerify::IsTrust(const char* lpszData, WORD wPort)
{
	if (m_bTrustAll)
	{
		//Ĭ��ȫ������
		return TRUE; 
	}

	long lTrust = 0;
	do 
	{
		lTrust = IsInRule(lpszData, lTrust);
		if (lTrust >= 0)
		{
			ContainPort& ayPort = m_ayRuleExtre[lTrust];
			ContainPort::iterator iter = find(ayPort.begin(), ayPort.end(), wPort);
			if (iter != ayPort.end() || (ayPort.size() == 1 || ayPort[0] == 0))
			{
				return TRUE;
			}
			lTrust++;
		}
	} while (lTrust >= 0);
	
	return FALSE;
}