#ifndef BASIC_TIME_H
#define BASIC_TIME_H

#include <time.h>

__NS_BASIC_START

/////////////////////////////////////////////////////////////////////////////
// CTimeSpan and CTime
class CTime;

class CTimeSpan
{
public:

// Constructors
	CTimeSpan();
	CTimeSpan(time_t time);
	CTimeSpan(LONG lDays, int nHours, int nMins, int nSecs);

	CTimeSpan(const CTimeSpan& timeSpanSrc);
	const CTimeSpan& operator=(const CTimeSpan& timeSpanSrc);

// Attributes
	// extract parts
	LONG GetDays() const;   // total # of days
	LONG GetTotalHours() const;
	int GetHours() const;
	LONG GetTotalMinutes() const;
	int GetMinutes() const;
	LONG GetTotalSeconds() const;
	int GetSeconds() const;

    time_t      GetTimeSpan32(){return (time_t)m_timeSpan;}
    __time64_t  GetTimeSpan64(){return m_timeSpan;}

// Operations
	// time math
	CTimeSpan operator-(CTimeSpan timeSpan) const;
	CTimeSpan operator+(CTimeSpan timeSpan) const;
	const CTimeSpan& operator+=(CTimeSpan timeSpan);
	const CTimeSpan& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTimeSpan timeSpan) const;
	BOOL operator!=(CTimeSpan timeSpan) const;
	BOOL operator<(CTimeSpan timeSpan) const;
	BOOL operator>(CTimeSpan timeSpan) const;
	BOOL operator<=(CTimeSpan timeSpan) const;
	BOOL operator>=(CTimeSpan timeSpan) const;

	// for compatibility with MFC 3.x
	CBasicString FormatMultiByte(const char* pFormat) const;
private:
	__time64_t m_timeSpan;
	friend class basiclib::CTime;
};

class CTime
{
public:

// Constructors
	static CTime GetCurrentTime();

	CTime();
	CTime(__time64_t time);
	CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
		int nDST = -1);
	CTime(WORD wDosDate, WORD wDosTime, int nDST = -1);
	CTime(const CTime& timeSrc);

#ifdef __BASICWINDOWS
	CTime(const SYSTEMTIME& sysTime, int nDST = -1);
	CTime(const FILETIME& fileTime, int nDST = -1);
#endif
	const CTime& operator=(const CTime& timeSrc);
	const CTime& operator=(time_t t);

// Attributes
	struct tm* GetLocalTm(struct tm* ptm) const;
#ifdef __BASICWINDOWS
	BOOL GetAsSystemTime(SYSTEMTIME& timeDest) const;
#endif

	//time_t GetTime() const;
    __time64_t Get64Time() const;
	long GetTime32() const;		//32位时间值
	
	int GetYear() const;
	int GetMonth() const;       // month of year (1 = Jan)
	int GetDay() const;         // day of month
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat

	//解析格式MMMMMMMMMMMMMM 例如 20140402174213,必须14位
	BOOL ParseString(LPCTSTR lpszTimeFormat);
	
// Operations
	// time math
	CTimeSpan operator-(CTime time) const;
	CTime operator-(CTimeSpan timeSpan) const;
	CTime operator+(CTimeSpan timeSpan) const;
	const CTime& operator+=(CTimeSpan timeSpan);
	const CTime& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTime time) const;
	BOOL operator!=(CTime time) const;
	BOOL operator<(CTime time) const;
	BOOL operator>(CTime time) const;
	BOOL operator<=(CTime time) const;
	BOOL operator>=(CTime time) const;

	// formatting using "C" strftime
	CBasicString Format_S(const char* pFormat) const;
private:
	__time64_t m_time;              //这边使用64位保证不会溢出
};

enum BasicTimeSpliteType
{
    TimeBasicAllWays    = 0,
    TimeBasicFixDay     = 1,
    TimeBasicFix        = 2,
};

enum BasicTimeSpliteReturnType
{
    TimeBasic_NotReady  = 0,        //还没到开始时间
    TimeBasic_In        = 1,        //在时间段内
    TimeBasic_Finish    = 2,        //过了时间段了
};

struct BasicTimeSplite
{
    CTime                   m_tmBegin;
    CTime                   m_tmEnd;
};

//定义两端时间的管理
class CBasicTimeSplite
{
public:
    CBasicTimeSplite();
    virtual ~CBasicTimeSplite();

    //初始化时间段
    int InitBasicTimeSplite(BasicTimeSpliteType basicTimeSpliteType, CTimeSpan& timeSpan, const char* lpszFormat, int nBeginDelay = 0, int nEndDelay = 0);
    //判断时间是否为时间段内
    BasicTimeSpliteReturnType IsTimeInTime(time_t tmNow);
    //获取最近的时间
    BasicTimeSplite GetLastBasicTimeSplite();

	// 转成字符串
	CBasicString FormatToString();
	// 从字符串初始化
	int	InitFromString(CBasicString& strData);
protected:
    void Empty();
    //更新时间
    void UpdateBasicTime(time_t tmNow);
public:
    typedef basic_vector<BasicTimeSplite>::type VTTimeSpliteInfo;
    typedef VTTimeSpliteInfo::iterator          VTTimeSpliteInfoIterator;
    VTTimeSpliteInfo                            m_vtTimeSplite;
    BasicTimeSpliteType                         m_timeSpliteType;
    CTimeSpan                                   m_timeSpan;
	CBasicString								m_strFormat;
    int                                         m_nBeginDelayTime;
    int                                         m_nEndDelayTime;
};

//支持单时间的时间点判断类
class CStatisticsTime
{
public:
	CStatisticsTime(void);
	virtual ~CStatisticsTime(void);

	//! 通过字符串初始化改类	
	void InitFromString(const char* lpszTime, time_t tmNow);
	//! 转成字符串
	basiclib::CBasicString FormatToString();

	//! 获取最近的时间点
	basiclib::CTime GetLastPrizeTime();
	//! 获取周期数
	int  GetCycle(){return m_nCycle;}

	//! 判断是否到颁奖的时间，如果到就设置下一个颁奖时间	
	BOOL IsPrizeTime(time_t tmNow);		

	//! dump出目前的成员变量信息
	void DumpTimeInfo(basiclib::CBasicString& strInfo);
protected:
	//更新时间
	void UpdateBasicTime(time_t tmNow);

protected:
	typedef basiclib::basic_vector<basiclib::CTime>::type	VTPrizeTime;
	typedef VTPrizeTime::iterator							VTPrizeTimeIterator;
	VTPrizeTime												m_vtPrizeTime;
	basiclib::CTimeSpan										m_tmSpan;		//颁奖的周期时间
	int														m_nCycle;		//表示第几个周期了
	CBasicString											m_strFormat;
};

// CTime and CTimeSpan
inline CTimeSpan::CTimeSpan()
{ m_timeSpan = 0; }
inline CTimeSpan::CTimeSpan(time_t time)
{ m_timeSpan = time; }
inline CTimeSpan::CTimeSpan(LONG lDays, int nHours, int nMins, int nSecs)
{ m_timeSpan = nSecs + 60* (nMins + 60* (nHours + 24* lDays)); }
inline CTimeSpan::CTimeSpan(const CTimeSpan& timeSpanSrc)
{ m_timeSpan = timeSpanSrc.m_timeSpan; }
inline const CTimeSpan& CTimeSpan::operator=(const CTimeSpan& timeSpanSrc)
{ m_timeSpan = timeSpanSrc.m_timeSpan; return *this; }
inline LONG CTimeSpan::GetDays() const
{ return (LONG)m_timeSpan / (24*3600L); }
inline LONG CTimeSpan::GetTotalHours() const
{ return (LONG)m_timeSpan/3600; }
inline int CTimeSpan::GetHours() const
{ return (int)(GetTotalHours() - GetDays()*24); }
inline LONG CTimeSpan::GetTotalMinutes() const
{ return (LONG)m_timeSpan/60; }
inline int CTimeSpan::GetMinutes() const
{ return (int)(GetTotalMinutes() - GetTotalHours()*60); }
inline LONG CTimeSpan::GetTotalSeconds() const
{ return (LONG)m_timeSpan; }
inline int CTimeSpan::GetSeconds() const
{ return (int)(GetTotalSeconds() - GetTotalMinutes()*60); }
inline CTimeSpan CTimeSpan::operator-(CTimeSpan timeSpan) const
{ return CTimeSpan(m_timeSpan - timeSpan.m_timeSpan); }
inline CTimeSpan CTimeSpan::operator+(CTimeSpan timeSpan) const
{ return CTimeSpan(m_timeSpan + timeSpan.m_timeSpan); }
inline const CTimeSpan& CTimeSpan::operator+=(CTimeSpan timeSpan)
{ m_timeSpan += timeSpan.m_timeSpan; return *this; }
inline const CTimeSpan& CTimeSpan::operator-=(CTimeSpan timeSpan)
{ m_timeSpan -= timeSpan.m_timeSpan; return *this; }
inline BOOL CTimeSpan::operator==(CTimeSpan timeSpan) const
{ return m_timeSpan == timeSpan.m_timeSpan; }
inline BOOL CTimeSpan::operator!=(CTimeSpan timeSpan) const
{ return m_timeSpan != timeSpan.m_timeSpan; }
inline BOOL CTimeSpan::operator<(CTimeSpan timeSpan) const
{ return m_timeSpan < timeSpan.m_timeSpan; }
inline BOOL CTimeSpan::operator>(CTimeSpan timeSpan) const
{ return m_timeSpan > timeSpan.m_timeSpan; }
inline BOOL CTimeSpan::operator<=(CTimeSpan timeSpan) const
{ return m_timeSpan <= timeSpan.m_timeSpan; }
inline BOOL CTimeSpan::operator>=(CTimeSpan timeSpan) const
{ return m_timeSpan >= timeSpan.m_timeSpan; }

inline CTime::CTime()
{ m_time = 0; }
inline CTime::CTime(__time64_t time)
{ m_time = time; }
inline CTime::CTime(const CTime& timeSrc)
{ m_time = timeSrc.m_time; }
inline const CTime& CTime::operator=(const CTime& timeSrc)
{ m_time = timeSrc.m_time; return *this; }
inline const CTime& CTime::operator=(time_t t)
{ m_time = t; return *this; }
//inline time_t CTime::GetTime() const
//{ return m_time; }
inline __time64_t CTime::Get64Time() const
{ return m_time;}
inline long CTime::GetTime32() const
{ return (long)Get64Time(); }
inline int CTime::GetYear() const
    {
    struct tm ti;
    GetLocalTm(&ti);
    return (ti.tm_year) + 1900; }
inline int CTime::GetMonth() const
    {
    struct tm ti;
    GetLocalTm(&ti);
    return ti.tm_mon + 1; }
inline int CTime::GetDay() const
    {
    struct tm ti;
    GetLocalTm(&ti);
    return ti.tm_mday; }
inline int CTime::GetHour() const
    {
    struct tm ti;
    GetLocalTm(&ti);
    return ti.tm_hour; }
inline int CTime::GetMinute() const
    {
    struct tm ti;
    GetLocalTm(&ti);
    return ti.tm_min; }
inline int CTime::GetSecond() const
    {
    struct tm ti;
    GetLocalTm(&ti);
    return ti.tm_sec; }
inline int CTime::GetDayOfWeek() const
    {
    struct tm ti;
    GetLocalTm(&ti);
    return ti.tm_wday + 1; }

inline CTimeSpan CTime::operator-(CTime time) const
{ return CTimeSpan(m_time - time.m_time); }
inline CTime CTime::operator-(CTimeSpan timeSpan) const
{ return CTime(m_time - timeSpan.m_timeSpan); }
inline CTime CTime::operator+(CTimeSpan timeSpan) const
{ return CTime(m_time + timeSpan.m_timeSpan); }
inline const CTime& CTime::operator+=(CTimeSpan timeSpan)
{ m_time += timeSpan.m_timeSpan; return *this; }
inline const CTime& CTime::operator-=(CTimeSpan timeSpan)
{ m_time -= timeSpan.m_timeSpan; return *this; }
inline BOOL CTime::operator==(CTime time) const
{ return m_time == time.m_time; }
inline BOOL CTime::operator!=(CTime time) const
{ return m_time != time.m_time; }
inline BOOL CTime::operator<(CTime time) const
{ return m_time < time.m_time; }
inline BOOL CTime::operator>(CTime time) const
{ return m_time > time.m_time; }
inline BOOL CTime::operator<=(CTime time) const
{ return m_time <= time.m_time; }
inline BOOL CTime::operator>=(CTime time) const
{ return m_time >= time.m_time; }

__NS_BASIC_END

#endif 
