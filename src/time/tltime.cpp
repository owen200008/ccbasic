#include "../inc/basic.h"

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////
CBasicTimeSplite::CBasicTimeSplite()
{
    Empty();
}

CBasicTimeSplite::~CBasicTimeSplite()
{

}

void CBasicTimeSplite::Empty()
{
    m_timeSpliteType = TimeBasicAllWays;
    m_vtTimeSplite.clear();
}

//初始化时间段
int CBasicTimeSplite::InitBasicTimeSplite(BasicTimeSpliteType basicTimeSpliteType, CTimeSpan& timeSpan, const char* lpszFormat, int nBeginDelay, int nEndDelay)
{
    int nRet = 0;
    if (lpszFormat == NULL || basicTimeSpliteType == TimeBasicAllWays || (timeSpan.GetDays() + timeSpan.GetHours() + timeSpan.GetMinutes() + timeSpan.GetSeconds() <= 0))
    {
        return nRet;
    }
    Empty();

    m_timeSpliteType = basicTimeSpliteType;
    m_timeSpan = timeSpan;
    m_nBeginDelayTime = nBeginDelay;
    m_nEndDelayTime = nEndDelay;
	m_strFormat = lpszFormat;

    CBasicStringArray ayItem;
    BasicSpliteString(lpszFormat, ';', IntoContainer_s<CBasicStringArray>(ayItem));
    int nSize = ayItem.GetSize();
    for (int i = 0; i < nSize;i++)
    {
        CBasicString& strTime = ayItem[i];
        if (strTime.GetLength() == 29)
        {
            CBasicStringArray ayTimes;
            BasicSpliteString(strTime.c_str(), ',', IntoContainer_s<CBasicStringArray>(ayTimes));
            int nTimesNumber = ayTimes.GetSize();
            if (nTimesNumber == 2)
            {
                int Y = 0,M = 0,D = 0,h = 0,m = 0,s = 0;
				
				scanf(ayTimes[0].c_str(), "%04d%02d%02d%02d%02d%02d", &Y, &M, &D, &h, &m, &s);
                basiclib::CTime tmBegin(Y, M, D, h, m, s);
				scanf(ayTimes[1].c_str(), "%04d%02d%02d%02d%02d%02d", &Y, &M, &D, &h, &m, &s);
                basiclib::CTime tmEnd(Y, M, D, h, m, s);
                BasicTimeSplite basicTimeSplite;
                basicTimeSplite.m_tmBegin = tmBegin;
                basicTimeSplite.m_tmEnd = tmEnd;
                if (tmBegin < tmEnd)
                {
                    m_vtTimeSplite.push_back(basicTimeSplite);
                    nRet++;
                }
            }
        }
    }
    if (nRet <= 0)
    {
        m_timeSpliteType = TimeBasicAllWays;
    }
    return nRet;
}

BOOL CompareTimeSplite(const BasicTimeSplite& first, const BasicTimeSplite& second)
{
    return first.m_tmBegin < second.m_tmBegin;
}

//更新时间
void CBasicTimeSplite::UpdateBasicTime(time_t tmNow)
{
    switch (m_timeSpliteType)
    {
    case TimeBasicAllWays:
        break;
    case TimeBasicFixDay:
        {
            if (m_vtTimeSplite.size() > 0)
            {
                BasicTimeSplite basicTimeSplite = m_vtTimeSplite[0];
                int i = 0;
                while (basicTimeSplite.m_tmEnd.GetTime32() + m_nEndDelayTime <= tmNow)
                {
                    basicTimeSplite.m_tmBegin += m_timeSpan;
                    basicTimeSplite.m_tmEnd += m_timeSpan;
                    i++;
                    if (i > 10000)
                    {
                        break;
                    }
                }
                m_vtTimeSplite.erase(m_vtTimeSplite.begin());
                m_vtTimeSplite.push_back(basicTimeSplite);

                std::sort(m_vtTimeSplite.begin(), m_vtTimeSplite.end(), CompareTimeSplite);
            }
        }
        break;
    case TimeBasicFix:
        break;
    default:
        ASSERT(0);
        break;
    }
}

//判断时间是否为时间段内
BasicTimeSpliteReturnType CBasicTimeSplite::IsTimeInTime(time_t tmNow)
{
    BasicTimeSpliteReturnType basicRet = TimeBasic_NotReady;
    switch (m_timeSpliteType)
    {
    case TimeBasicAllWays:
        basicRet = TimeBasic_In;
        break;
    case TimeBasicFixDay:
        {
            if (m_vtTimeSplite.size() > 0)
            {
                BasicTimeSplite& basicTimeSplite = m_vtTimeSplite[0];
                if (tmNow >= basicTimeSplite.m_tmBegin.GetTime32() + m_nBeginDelayTime)
                {
                    if (tmNow < basicTimeSplite.m_tmEnd.GetTime32() + m_nEndDelayTime)
                    {
                        basicRet = TimeBasic_In;
                    }
                    else
                    {
                        UpdateBasicTime(tmNow);
                        basicRet = IsTimeInTime(tmNow);
                    }
                    break;
                }
            }
        }
        break;
    case TimeBasicFix:
        {
            for (VTTimeSpliteInfoIterator iter = m_vtTimeSplite.begin();iter != m_vtTimeSplite.end();iter++)
            {
                BasicTimeSplite& basicTimeSplite = *iter;
                if (tmNow >= basicTimeSplite.m_tmBegin.GetTime32() + m_nBeginDelayTime)
                {
                    if (tmNow <= basicTimeSplite.m_tmEnd.GetTime32() + m_nEndDelayTime)
                    {
                        basicRet = TimeBasic_In;
						break;
                    }
                    else
                    {
                        basicRet = TimeBasic_Finish;
                    }
                }
				else
				{
					basicRet = TimeBasic_NotReady;
					break;
				}
            }
        }
        break;
    default:
        ASSERT(0);
        break;
    }
    return basicRet;
}

//获取最近的时间
BasicTimeSplite CBasicTimeSplite::GetLastBasicTimeSplite()
{
    BasicTimeSplite basicTimeSplite;
    if (m_vtTimeSplite.size() > 0)
    {
        basicTimeSplite = *(m_vtTimeSplite.begin());
    }
    return basicTimeSplite;
}

// 转成字符串
CBasicString CBasicTimeSplite::FormatToString()
{
	CBasicString strRet;
	strRet.Format("%d|%d|%d|%s", m_timeSpliteType, m_timeSpan.GetDays(), m_timeSpan.GetHours(), m_strFormat.GetBuffer(0));
	return strRet;
}

// 从字符串初始化
int CBasicTimeSplite::InitFromString(CBasicString& strData)
{
	CBasicStringArray ayInfo;
	BasicSpliteString(strData.c_str(), '|', IntoContainer_s<CBasicStringArray>(ayInfo));
	if (ayInfo.size() >= 4)
	{
		basiclib::BasicTimeSpliteType timeSpliteType = (basiclib::BasicTimeSpliteType)atol(ayInfo[0].c_str());
		int nSpanDay = atol(ayInfo[1].c_str());
		int nSpanHour = atol(ayInfo[2].c_str());
		basiclib::CTimeSpan timeSpan(nSpanDay, nSpanHour, 0, 0);
		return InitBasicTimeSplite(timeSpliteType, timeSpan, ayInfo[3].c_str());
	}
	return 0;
}

CStatisticsTime::CStatisticsTime()
{
	m_nCycle = 0;
}

CStatisticsTime::~CStatisticsTime()
{

}

void CStatisticsTime::InitFromString(const char* lpszTime, time_t tmNow)
{
	if (NULL == lpszTime)
	{
		ASSERT(0);
		return;
	}

	CBasicStringArray ayInfo;
	BasicSpliteString(lpszTime, '|', IntoContainer_s<CBasicStringArray>(ayInfo));
	if (3 != ayInfo.size())
	{
		ASSERT(0);
		return;
	}
	int nSpanDay  = atol(ayInfo[0].c_str());
	int nSpanHour = atol(ayInfo[1].c_str());
	if (nSpanDay < 0 || nSpanHour < 0 || nSpanHour > 24)
	{
		ASSERT(0);
		return;
	}
	basiclib::CTimeSpan timeSpan(nSpanDay, nSpanHour, 0, 0);
	m_tmSpan = timeSpan;

	CBasicString &strPrizeTime = ayInfo[2];
	CBasicStringArray ayItem;
	BasicSpliteString(strPrizeTime.c_str(), ';', IntoContainer_s<CBasicStringArray>(ayItem));
	int nSize = ayItem.GetSize();
	for (int i = 0; i < nSize;i++)
	{
		CBasicString& strTime = ayItem[i];
		if (strTime.GetLength() == 14)
		{
			int Y,M,D,h,m,s;
			sscanf(strTime.c_str(), "%04d%02d%02d%02d%02d%02d", &Y, &M, &D, &h, &m, &s);
			basiclib::CTime timePrize(Y, M, D, h, m, s);  //第一个周期的颁奖时间
			m_vtPrizeTime.push_back(timePrize);
		}
	}
	
	//更新到当前最新的时间
	UpdateBasicTime(tmNow);

	m_strFormat = lpszTime;
}
// 转成字符串
basiclib::CBasicString CStatisticsTime::FormatToString()
{
	return m_strFormat;
}

BOOL CompareStatisticsTimeSplite(const basiclib::CTime& first, const basiclib::CTime& second)
{
	return first < second;
}

//更新时间
void CStatisticsTime::UpdateBasicTime(time_t tmNow)
{
	for (VTPrizeTimeIterator iter = m_vtPrizeTime.begin();iter != m_vtPrizeTime.end();iter++)
	{
		int nCycle = 0;
		basiclib::CTime& tmTime = *iter;
		while (tmTime.GetTime32() <= tmNow)
		{
			if (nCycle >= 1000)
			{
				return;
			}
			if(m_tmSpan.GetTimeSpan32() != 0)
			{
				//增加的范围
				int nStep = (int)((tmNow - tmTime.GetTime32()) / m_tmSpan.GetTimeSpan32());
				if(nStep <= 0)
				{
					nStep = 1;
				}
				CTimeSpan tmAddSpan(m_tmSpan.GetTimeSpan32() * nStep);
				tmTime += tmAddSpan;
				nCycle++;
				m_nCycle++;
			}
			else
			{
				return;
			}
		}
	}

	if (m_vtPrizeTime.size() > 1)
	{
		//排序一次
		std::sort(m_vtPrizeTime.begin(), m_vtPrizeTime.end(), CompareStatisticsTimeSplite);
	}
}

//! 获取最近的时间点
basiclib::CTime CStatisticsTime::GetLastPrizeTime()
{
	basiclib::CTime tmNow;
	if (m_vtPrizeTime.size() > 0)
	{
		tmNow = m_vtPrizeTime[0];
	}
	return tmNow;
}

//判断是否到颁奖的时间，如果到就设置下一个颁奖时间	
BOOL CStatisticsTime::IsPrizeTime(time_t tmNow)
{
	BOOL bRet = FALSE;
	if (m_vtPrizeTime.size() > 0)
	{
		if (tmNow >= m_vtPrizeTime[0].GetTime32())
		{
			UpdateBasicTime(tmNow);
			bRet = TRUE;
		}
	}
	return bRet;
}

//! dump出目前的成员变量信息
void CStatisticsTime::DumpTimeInfo(basiclib::CBasicString& strInfo)
{
	basiclib::CBasicString strTmp;
	strTmp.Format("%d|%d|%d|%s\r\n", m_nCycle, m_tmSpan.GetDays(), m_tmSpan.GetHours(), m_strFormat.GetBuffer(0));
	strInfo += strTmp;

	for (VTPrizeTimeIterator iter = m_vtPrizeTime.begin();iter != m_vtPrizeTime.end();iter++)
	{
		basiclib::CTime& tmTime = *iter;
		strTmp = tmTime.Format_S("%Y%m%d-%H:%M:%S");
		strInfo += strTmp;
		strInfo += "\r\n";
	}
}
/////////////////////////////////////////////////////////////////////////////
// CTime - absolute time

CTime::CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
	int nDST)
{
	struct tm atm;
	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	ASSERT(nDay >= 1 && nDay <= 31);
	atm.tm_mday = nDay;
	ASSERT(nMonth >= 1 && nMonth <= 12);
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	ASSERT(nYear >= 1900);
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = nDST;
	m_time = _mktime64(&atm);
    ASSERT(m_time != -1);       // indicates an illegal input time
}

CTime::CTime(WORD wDosDate, WORD wDosTime, int nDST)
{
	struct tm atm;
	atm.tm_sec = (wDosTime & ~0xFFE0) << 1;
	atm.tm_min = (wDosTime & ~0xF800) >> 5;
	atm.tm_hour = wDosTime >> 11;

	atm.tm_mday = wDosDate & ~0xFFE0;
	atm.tm_mon = ((wDosDate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (wDosDate >> 9) + 80;
	atm.tm_isdst = nDST;
	m_time = _mktime64(&atm);
	ASSERT(m_time != -1);       // indicates an illegal input time
}

#ifdef __BASICWINDOWS
CTime::CTime(const SYSTEMTIME& sysTime, int nDST)
{
	if (sysTime.wYear < 1900)
	{
		time_t time0 = 0L;
		CTime timeT(time0);
		*this = timeT;
	}
	else
	{
		CTime timeT(
			(int)sysTime.wYear, (int)sysTime.wMonth, (int)sysTime.wDay,
			(int)sysTime.wHour, (int)sysTime.wMinute, (int)sysTime.wSecond,
			nDST);
		*this = timeT;
	}
}

CTime::CTime(const FILETIME& fileTime, int nDST)
{
	// first convert file time (UTC time) to local time
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))
	{
		m_time = 0;
		return;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		m_time = 0;
		return;
	}

	// then convert the system time to a time_t (C-runtime local time)
	CTime timeT(sysTime, nDST);
	*this = timeT;
}
#endif

//解析格式MMMM-MM-MM MM:MM:MM 例如 2014-04-02 17:42:13  
BOOL CTime::ParseString(const char* lpszTimeFormat)
{
	BOOL nResult = FALSE;
	if (__tcslen(lpszTimeFormat) == 14)
	{
		struct tm atm;
		sscanf(lpszTimeFormat, "%4d%2d%2d%2d%2d%2d", &atm.tm_year, &atm.tm_mon, &atm.tm_mday, &atm.tm_hour, &atm.tm_min, &atm.tm_sec);
		
		ASSERT(atm.tm_mday >= 1 && atm.tm_mday <= 31);
		ASSERT(atm.tm_mon >= 1 && atm.tm_mon <= 12);
		ASSERT(atm.tm_year >= 1900);
		if (atm.tm_mday >= 1 && atm.tm_mday <= 31 && atm.tm_mon >= 1 && atm.tm_mon <= 12 && atm.tm_year >= 1900)
		{
			atm.tm_mon = atm.tm_mon - 1;        // tm_mon is 0 based
			atm.tm_year = atm.tm_year - 1900;     // tm_year is 1900 based
			atm.tm_isdst = -1;

			m_time = _mktime64(&atm);
			nResult = TRUE;
		}
	}
	return nResult;
}

CTime CTime::GetCurrentTime()
// return the current system time
{
	return CTime(::time(NULL));
}

#ifdef __BASICWINDOWS

struct tm* CTime::GetLocalTm(struct tm* ptm) const
{
	if (ptm != NULL)
	{
		struct tm* ptmTemp = _localtime64(&m_time);
		if (ptmTemp == NULL)
			return NULL;    // indicates the m_time was not initialized!

		*ptm = *ptmTemp;
		return ptm;
	}
	else
		return _localtime64(&m_time);
}

#else

struct tm* CTime::GetLocalTm(struct tm* ptm) const
{
	if (ptm != NULL)
	{
		time_t tm = m_time;
		return localtime_r(&tm, ptm);	
	}
	else
		return NULL;
}
#endif //

#ifdef __BASICWINDOWS
BOOL CTime::GetAsSystemTime(SYSTEMTIME& timeDest) const
{
	struct tm ti;
	struct tm* ptm = GetLocalTm(&ti);
	if (ptm == NULL)
		return FALSE;

	timeDest.wYear = (WORD) (1900 + ptm->tm_year);
	timeDest.wMonth = (WORD) (1 + ptm->tm_mon);
	timeDest.wDayOfWeek = (WORD) ptm->tm_wday;
	timeDest.wDay = (WORD) ptm->tm_mday;
	timeDest.wHour = (WORD) ptm->tm_hour;
	timeDest.wMinute = (WORD) ptm->tm_min;
	timeDest.wSecond = (WORD) ptm->tm_sec;
	timeDest.wMilliseconds = 0;

	return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// String formatting

#define maxTimeBufferSize       128
	// Verifies will fail if the needed buffer size is too large

CBasicString CTimeSpan::FormatMultiByte(const char* pFormat) const
// formatting timespans is a little trickier than formatting CTimes
//  * we are only interested in relative time formats, ie. it is illegal
//      to format anything dealing with absolute time (i.e. years, months,
//         day of week, day of year, timezones, ...)
//  * the only valid formats:
//      %D - # of days -- NEW !!!
//      %H - hour in 24 hour format
//      %M - minute (0-59)
//      %S - seconds (0-59)
//      %% - percent sign
{
	char szBuffer[maxTimeBufferSize];
	char ch;
	char* pch = szBuffer;

	while ((ch = *pFormat++) != '\0')
	{
		ASSERT(pch < &szBuffer[maxTimeBufferSize]);
		if (ch == '%')
		{
			switch (ch = *pFormat++)
			{
			default:
				ASSERT(FALSE);      // probably a bad format character
			case '%':
				*pch++ = ch;
				break;
			case 'D':
				pch += sprintf(pch, "%ld", GetDays());
				break;
			case 'H':
				pch += sprintf(pch, "%02d", GetHours());
				break;
			case 'M':
				pch += sprintf(pch, "%02d", GetMinutes());
				break;
			case 'S':
				pch += sprintf(pch, "%02d", GetSeconds());
				break;
			}
		}
		else
		{
			*pch++ = ch;
			if (_istlead(ch))
			{
				ASSERT(pch < &szBuffer[maxTimeBufferSize]);
				*pch++ = *pFormat++;
			}
		}
	}

	*pch = '\0';
	return szBuffer;
}

#ifdef __BASICWINDOWS

CBasicString CTime::Format_S(const char* pFormat) const
{
	char szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = _localtime64(&m_time);
	if (ptmTemp == NULL ||
		!strftime(szBuffer, _countof(szBuffer), pFormat, ptmTemp))
		szBuffer[0] = '\0';
	return szBuffer;
}

#else

CBasicString CTime::Format_S(const char* pFormat) const
{
	char szBuffer[maxTimeBufferSize];

	//struct tm* ptmTemp = localtime(&m_time);
	struct tm ti;
	time_t tm = m_time;
	localtime_r(&tm, &ti);
	struct tm* ptmTemp = &ti;
	if (ptmTemp == NULL ||
		!strftime(szBuffer, _countof(szBuffer), pFormat, ptmTemp))
		szBuffer[0] = '\0';
	return szBuffer;
}


#endif

////////////////////////////////////////////////////////////////////////////
// out-of-line inlines for binary compatibility

#ifdef _AFXDLL
#ifndef _DEBUG

CTime::CTime()
	{ }

CTime::CTime(const CTime& timeSrc)
	{ m_time = timeSrc.m_time; }

CTimeSpan::CTimeSpan()
	{ }

#endif
#endif

//get the current time
void GetCurrentTimeYMDHMS(char* pBuffer, int nLength, time_t tmNow)
{
	if (nLength < 16)
		return;
	if (tmNow == 0)
		tmNow = time(NULL);
	CTime cur(tmNow);
	sprintf(pBuffer, "%d%02d%02d %02d%02d%02d", cur.GetYear(), cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute(), cur.GetSecond());
}

__NS_BASIC_END
/////////////////////////////////////////////////////////////////////////////
