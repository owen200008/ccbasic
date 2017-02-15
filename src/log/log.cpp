#include "../inc/basic.h"

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
#include <syslog.h>
#endif

#ifdef __BASICWINDOWS
//?? for access
#include <io.h>
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START

static int g_nLogLimit = (10*1024*1024);		//日志文件大小限制
#define LOG_MESSAGE_SIZE 256

struct WriteLogDataBuffer
{
public:
	WriteLogDataBuffer()
	{
		InitMember();
	}
public:
	void InitLogData(const char* lpszText)
	{
		m_pText = (char*)lpszText;
	}

	void CopyLogData(WriteLogDataBuffer& logData)
	{
		if (logData.m_pText != NULL)
		{
			int nLen = __tcslen(logData.m_pText);
			m_pText = (char*)BasicAllocate((nLen + 1) * sizeof(char));
			__tcscpy(m_pText, logData.m_pText);
		}
		m_lCurTime = logData.m_lCurTime;
		m_dwProcessId = logData.m_dwProcessId;
		m_dwThreadId = logData.m_dwThreadId;
	}

	void ClearLogData()
	{
		if (m_pText != NULL)
		{
			BasicDeallocate(m_pText);
		}
		InitMember();
	}
protected:
	void InitMember()
	{
		memset(this, 0, sizeof(*this));
	}

public:
	char*		m_pText;
	time_t		m_lCurTime;
	DWORD		m_dwProcessId;
	DWORD		m_dwThreadId;
};

static long ReplaceFileName(char* szFileName, const char* lpszKey, const char* lpszToday)
{
	int nNameLen = __tcslen(szFileName);
	int nDayLen = __tcslen(lpszToday);
	char* pKeyPos = strstr(szFileName, lpszKey);
	char* pKeyEnd = NULL;
	int nLeft = 0;
	if (pKeyPos != NULL)
	{
		pKeyEnd = pKeyPos + __tcslen(lpszKey);
	}
	else
	{
		pKeyPos = strrchr(szFileName, '.');
		if (pKeyPos == NULL)
		{
			pKeyPos = &szFileName[nNameLen];
			*pKeyPos = '.';
			pKeyPos++;
			pKeyEnd = pKeyPos;
		}
		else
		{
			pKeyEnd = pKeyPos;
		}
	}
	nLeft = nNameLen - (pKeyEnd - szFileName);
	if (nLeft > 0)
	{
		char* pNewPos = pKeyPos + nDayLen;
		memmove(pNewPos, pKeyEnd, nLeft * sizeof(char));
	}
	__tcsncpy(pKeyPos, lpszToday, nDayLen);

	return (pKeyPos - szFileName);
}

static void GetTodayString(char* szToday)
{
	CTime cur = CTime::GetCurrentTime();
	sprintf(szToday, "%d%02d%02d_000", cur.GetYear(), cur.GetMonth(), cur.GetDay());
}


//日志文件属性结构
class CBasicLogChannel : public basiclib::CBasicObject
{
public:
	CBasicLogChannel(){
		m_nLogOption = 0;
		m_nSizeLimit = 0;
		m_fLog = NULL;
		m_lReplacePos = 0;

		memset(m_szLogFileName, 0, sizeof(m_szLogFileName));
		memset(m_szToday, 0, sizeof(m_szToday));

		m_pSynLogFile = NULL;
	}
	~CBasicLogChannel(){
		if (m_pSynLogFile){
			delete m_pSynLogFile;
		}
	}
public:
	long InitLogChannel(long nOption, const char* lpszLogFile);		//初始化
	long ChangeLogChannel(long nOption, const char* lpszLogFile);	//更新设置
	void CheckChannel(const char* lpszToday);						//定时检查，比如隔天重新生成文件名，判断文件大小等。

	long WriteLogData(const char* lpszText);	//写日志数据

	void WriteLogBuffer();
	static void SetLockChannel(bool bLock){
		m_bInitLock = bLock;
	}
protected:
	long OpenLogFile()
	{
		if (m_nLogOption & LOG_BY_OPEN)
		{
			m_fLog = fopen(m_szLogFileName, "ab");
			if (m_fLog == NULL)
			{
				return LOG_ERROR_OPEN_FILE;
			}
		}
		return 0;
	}
	void CloseLogFile()
	{
		if (m_fLog != NULL)
		{
			fclose(m_fLog);
			m_fLog = NULL;
		}
	}
	FILE* GetFileHandle()
	{
		if (m_fLog == NULL)
		{
			return fopen(m_szLogFileName, "ab");
		}
		return m_fLog;
	}

	void CreateNewFileName()
	{
		if (m_lReplacePos < 0)
		{
			m_lReplacePos = ReplaceFileName(m_szLogFileName, LOG_NAME_DAY_S, m_szToday);
		}
		char* pPos = &m_szLogFileName[m_lReplacePos + 9];		//序号开始的位置
		char szTemp[32];
		do
		{
			sprintf(szTemp, "%04d", (uint32_t)GetNewFileNo());
			strncpy(pPos, szTemp, 4);
			if (_access(m_szLogFileName, 0) != 0)
			{
				strncpy(&m_szToday[9], szTemp, 3);
				break;
			}
		} while (1);
	}

	void _FillLogDataBuffer(WriteLogDataBuffer& logData);			//根据选项，填充数据
	void _AddLogDataBuffer(WriteLogDataBuffer& logData);			//增加到缓冲数组
	long _WriteLogDataBuffer(WriteLogDataBuffer& logData);

	long _SetLogChannel(long nOption, const char* lpszLogFile);			//设置

	static long GetNewFileNo()
	{
		return BasicInterlockedIncrement(&m_lLastFileNo);
	}

private:
	long	m_nLogOption;					//日志的属性，见定义 LOG_*
	long	m_nSizeLimit;					//日志文件的大小限制。0表示不限制
	FILE*	m_fLog;							//日志文件句柄
	char	m_szLogFileName[MAX_PATH];
	char	m_szToday[16];					//当天的日期和序号。格式：YYMMDD_XXX  如 20110720_000
	long	m_lReplacePos;					//替换日期或者序号的起始位置

	static long	m_lLastFileNo;				//对于当天多个文件自动生成的文件编号。
	static bool m_bInitLock;				//是否上锁

	CCriticalSection* m_pSynLogFile;		//操作成员变量的临界区

	typedef std::vector<WriteLogDataBuffer> vector_logdata;
	vector_logdata		m_vLogData;			//缓冲的数据
};

long	CBasicLogChannel::m_lLastFileNo = 0;		//对于当天多个文件自动生成的文件编号。
bool	CBasicLogChannel::m_bInitLock = true;		//默认上锁

long CBasicLogChannel::InitLogChannel(long nOption, const char* lpszLogFile)
{
	_SetLogChannel(nOption, lpszLogFile);
	long lRet = OpenLogFile();
	if (lRet == 0)
	{
		if (m_bInitLock)
			m_pSynLogFile = new CCriticalSection;
	}

	return lRet;

}

long CBasicLogChannel::ChangeLogChannel(long nOption, const char* lpszLogFile)
{
	CSingleLock lock(m_pSynLogFile);
	lock.Lock();

	CloseLogFile();

	_SetLogChannel(nOption, lpszLogFile);

	return OpenLogFile();
}

long CBasicLogChannel::_SetLogChannel(long nOption, const char* lpszLogFile)
{
	m_nLogOption = nOption;
	if (nOption & LOG_BY_NOLIMIT)
	{
		m_nSizeLimit = 0;	//不限制大小
	}
	else
	{
		m_nSizeLimit = (nOption & LOG_SIZE_LIMIT) * 1024 * 1024;
		if (m_nSizeLimit <= 0)
		{
			m_nSizeLimit = g_nLogLimit;	//默认大小限制
		}
	}

	__tcscpyn(m_szLogFileName, MAX_PATH, lpszLogFile);

	GetTodayString(m_szToday);
	if ((m_nLogOption & LOG_BY_DAY) && !(m_nLogOption & LOG_BY_SAMENAME))
	{
		m_lReplacePos = ReplaceFileName(m_szLogFileName, LOG_NAME_DAY_S, m_szToday);
	}
	else
	{
		m_lReplacePos = -1;
	}

	Basic_mkdir(m_szLogFileName);

	return 0;
}

void CBasicLogChannel::CheckChannel(const char* lpszToday)
{
	CSingleLock lock(m_pSynLogFile);
	lock.Lock();
	if (m_nLogOption & LOG_BY_DAY)
	{
		if (strncmp(m_szToday, lpszToday, 8) != 0)		//日期不一致，需要更新文件名
		{
			CloseLogFile();
			if (m_nLogOption & LOG_BY_ONEFILE)
			{
				Basic_DeleteFile(m_szLogFileName);
			}
			if (!(m_nLogOption & LOG_BY_SAMENAME))
			{
				m_lReplacePos = ReplaceFileName(m_szLogFileName, m_szToday, lpszToday);
			}
			strcpy(m_szToday, lpszToday);
			OpenLogFile();
		}
	}

	TLFileStatus fStatus;
	Basic_GetFileStatus(m_szLogFileName, fStatus);
	if (m_nSizeLimit > 0 && fStatus.m_size > m_nSizeLimit)		//长度超过限制
	{
		CloseLogFile();
		if (m_nLogOption & LOG_BY_ONEFILE)
		{
			Basic_DeleteFile(m_szLogFileName);
		}
		CreateNewFileName();
		OpenLogFile();
	}
	WriteLogBuffer();
}

void CBasicLogChannel::WriteLogBuffer()
{
	CSingleLock lock(m_pSynLogFile);
	lock.Lock();

	if (!m_vLogData.empty())
	{
		int nCount = m_vLogData.size();
		for (int i = 0; i < nCount; i++)
		{
			WriteLogDataBuffer& log = m_vLogData[i];
			_WriteLogDataBuffer(log);

			log.ClearLogData();
		}
		m_vLogData.clear();
	}
}

long CBasicLogChannel::WriteLogData(const char* lpszText)
{
	WriteLogDataBuffer logData;
	logData.InitLogData(lpszText);

	_FillLogDataBuffer(logData);

	CSingleLock lock(m_pSynLogFile);
	lock.Lock();

	if (m_nLogOption & LOG_BY_BUFFER)
	{
		_AddLogDataBuffer(logData);
	}
	else
	{
		return _WriteLogDataBuffer(logData);
	}
	return 0;
}

void CBasicLogChannel::_FillLogDataBuffer(WriteLogDataBuffer& logData)
{
	if (m_nLogOption & LOG_ADD_TIME)
	{
		logData.m_lCurTime = time(NULL);
	}
	if (m_nLogOption & LOG_ADD_THREAD)
	{
		logData.m_dwProcessId = Basic_GetCurrentProcessId();
		logData.m_dwThreadId = BasicGetCurrentThreadId();
	}
}

void CBasicLogChannel::_AddLogDataBuffer(WriteLogDataBuffer& logData)
{
	WriteLogDataBuffer thisLogData;
	thisLogData.CopyLogData(logData);

	m_vLogData.push_back(thisLogData);
}

long CBasicLogChannel::_WriteLogDataBuffer(WriteLogDataBuffer& logData)
{
	FILE* pFile = GetFileHandle();
	if (pFile == NULL)
	{
		return -1;		//文件打开不成功
	}
	basiclib::CBasicSmartBuffer smBuf;
	smBuf.SetDataLength(1024);
	smBuf.SetDataLength(0);

	if (logData.m_lCurTime > 0)	//写入时间
	{
		CTime cur(logData.m_lCurTime);
		char szLog[128];
		sprintf(szLog, "%d%02d%02d %02d%02d%02d  ", cur.GetYear(), cur.GetMonth(), cur.GetDay(), cur.GetHour(), cur.GetMinute(), cur.GetSecond());
		smBuf.AppendString(szLog);
	}

	if (logData.m_dwThreadId != 0)	//写入线程ID
	{
		char szLog[128];
		sprintf(szLog, "pid:%d thread:%d ", (uint32_t)logData.m_dwProcessId, (uint32_t)logData.m_dwThreadId);
		smBuf.AppendString(szLog);
	}

	if (logData.m_pText != NULL)
	{
		smBuf.AppendString(logData.m_pText);
	}
	smBuf.AppendString("\r\n");
	fwrite(smBuf.GetDataBuffer(), 1, smBuf.GetDataLength(), pFile);
	if (pFile != m_fLog)
	{
		fclose(pFile);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_LOG_CHANNEL		8			//日志文件个数

static long g_nCurrentChannel = 0;

CBasicLogChannel* g_pLogChannel[MAX_LOG_CHANNEL] = { 0 };

static CBasicLogChannel* _CreateLogChannel(long nOption, const char* pszLogFile)
{
	CBasicLogChannel* pChannel = new CBasicLogChannel;
	long lRet = pChannel->InitLogChannel(nOption, pszLogFile);
	if (lRet < 0)
	{
		delete pChannel;
		pChannel = NULL;
	}
	return pChannel;
}

CBasicLogChannel* _GetChannel(int nIndex)
{
	if (nIndex < 0 || nIndex >= MAX_LOG_CHANNEL)
	{
		return NULL;
	}
	if (g_pLogChannel[nIndex] == NULL && nIndex == 0)  //默认的内部创建
	{
		g_pLogChannel[nIndex] = _CreateLogChannel(LOG_BY_DAY | LOG_BY_SIZE | LOG_ADD_TIME | LOG_ADD_THREAD, "basiclibs.log");
	}
	return g_pLogChannel[nIndex];
}

class CWriteLogThread : public CBasic_Thread
{
public:
	CWriteLogThread()
	{
		m_bRuning = false;
	}
	~CWriteLogThread()
	{
		Stop();
	}
public:
	virtual void Run() 
	{
		int nCount = 0;
		while(m_bRuning)
		{
			CheckChannel();
			BasicSleep(30000);		//休息 30 秒钟
		}
	}
	void Start()
	{
		m_bRuning = true;
		CBasic_Thread::Start();
	}
	void Stop()
	{
		m_bRuning = false;
		Join();
		CheckChannel();
	}
protected:
	void CheckChannel()
	{
		int nSize = g_nCurrentChannel + 1;
		if(nSize > MAX_LOG_CHANNEL)
		{
			nSize = MAX_LOG_CHANNEL;
		}
		if(nSize > 1)
		{
			char szToday_s[32];
			GetTodayString(szToday_s);
			for(int i = 0; i < nSize; i++)
			{
				if (g_pLogChannel[i] != NULL)
				{
					g_pLogChannel[i]->CheckChannel(szToday_s);
				}
			}
		}
	}
protected:
	volatile bool m_bRuning;
};

static CWriteLogThread* g_LogThread = NULL;

long BasicSetLogEventMode(long nOption, const char* pszLogFile)
{
	if (g_nCurrentChannel < 0 || g_nCurrentChannel >= MAX_LOG_CHANNEL)
	{
		return LOG_ERROR_FULL;
	}
	if (pszLogFile == NULL || pszLogFile[0] == '\0')
	{
		return LOG_ERROR_NAME_EMPTY;
	}


	int nIndex = BasicInterlockedIncrement(&g_nCurrentChannel);
	if (nIndex >= MAX_LOG_CHANNEL)
	{
		return LOG_ERROR_FULL;
	}

	g_pLogChannel[nIndex] = _CreateLogChannel(nOption, pszLogFile);

	return nIndex;
}

long BasicSetDefaultLogEventErrorMode(long nOption, const char* pszLogFile)
{
	if (pszLogFile == NULL || pszLogFile[0] == '\0')
	{
		return LOG_ERROR_NAME_EMPTY;
	}
	if (g_pLogChannel[1] == NULL)
	{
		g_pLogChannel[1] = _CreateLogChannel(nOption, pszLogFile);
	}
	else
	{
		g_pLogChannel[1]->ChangeLogChannel(nOption, pszLogFile);
	}
	return 0;
}

long BasicSetDefaultLogEventMode(long nOption, const char* pszLogFile)
{
	if (pszLogFile == NULL || pszLogFile[0] == '\0')
	{
		return LOG_ERROR_NAME_EMPTY;
	}
	if (g_pLogChannel[0] == NULL)
	{
		g_pLogChannel[0] = _CreateLogChannel(nOption, pszLogFile);
	}
	else
	{
		g_pLogChannel[0]->ChangeLogChannel(nOption, pszLogFile);
	}


	return 0;
}

long BasicCloseLogEvent(long lLogChannel)
{
	if(lLogChannel < 0)		//关闭所有的
	{
		if(g_LogThread != NULL)
		{
			g_LogThread->Stop();
		}
		g_nCurrentChannel = 0;			//这里会造成内存泄露
	}
	else if(lLogChannel >= 0 || lLogChannel < MAX_LOG_CHANNEL)
	{
		if(g_pLogChannel[lLogChannel] != NULL)
		{
			g_pLogChannel[lLogChannel]->WriteLogBuffer();
			g_pLogChannel[lLogChannel] = NULL;		//这里会造成内存泄露
		}
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//事件记录（写到默认日志文件）
//
void BasicLogEventErrorV(const char* pszLog, ...)
{
	char tmp[LOG_MESSAGE_SIZE];
	va_list argList;
	va_start(argList, pszLog);
	int len = vsnprintf(tmp, LOG_MESSAGE_SIZE, pszLog, argList);
	va_end(argList);
	if (len >= 0 && len < LOG_MESSAGE_SIZE)
	{
		BasicLogEvent(1, tmp);
	}
	else
	{
		CBasicString strLog;
		va_start(argList, pszLog);
		strLog.FormatV(pszLog, argList);
		va_end(argList);
		BasicLogEvent(1, strLog.c_str());
	}
}
void BasicLogEventV(const char* pszLog, ...)
{
	char tmp[LOG_MESSAGE_SIZE];
	va_list argList;
	va_start(argList, pszLog);
	int len = vsnprintf(tmp, LOG_MESSAGE_SIZE, pszLog, argList);
	va_end(argList);
	if (len >= 0 && len < LOG_MESSAGE_SIZE)
	{
		BasicLogEvent(0, tmp);
	}
	else
	{
		CBasicString strLog;
		va_start(argList, pszLog);
		strLog.FormatV(pszLog, argList);
		va_end(argList);
		BasicLogEvent(0, strLog.c_str());
	}
}

void BasicLogEventError(const char* pszLog)
{
	BasicLogEvent(1, pszLog);
}
void BasicLogEvent(const char* pszLog)
{
	BasicLogEvent(0, pszLog);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void BasicLogEventV(long lLogChannel, const char* pszLog, ...)
{
	char tmp[LOG_MESSAGE_SIZE];
	va_list argList;
	va_start(argList, pszLog);
	int len = vsnprintf(tmp, LOG_MESSAGE_SIZE, pszLog, argList);
	va_end(argList);
	if (len >= 0 && len < LOG_MESSAGE_SIZE)
	{
		BasicLogEvent(lLogChannel, tmp);
	}
	else
	{
		CBasicString strLog;
		va_start(argList, pszLog);
		strLog.FormatV(pszLog, argList);
		va_end(argList);
		BasicLogEvent(lLogChannel, strLog.c_str());
	}
}


//
//事件记录（写到日志文件）
//
void BasicLogEvent(long lLogChannel, const char* pszLog)
{
	if (pszLog == NULL || pszLog[0] == '\0')
	{
		return;
	}

	TRACE("LOG%d:%s\r\n", lLogChannel, pszLog);

	//如果找不到直接让他崩溃掉
	_GetChannel(lLogChannel)->WriteLogData(pszLog);
}

//设置是否启动锁，启动锁的话日志变成线程安全，不然就是单线程使用, 自动启动检测线程，只有在lock情况下才开启
void InitBasicLog(bool bLock, bool bThreadCheckSelf)
{
	CBasicLogChannel::SetLockChannel(bLock);
	if (bLock){
		if (bThreadCheckSelf){
			if (g_LogThread == NULL)
			{
				g_LogThread = new CWriteLogThread;
				g_LogThread->Start();
			}
		}
	}
	else{
		if (g_LogThread){
			g_LogThread->Stop();
			delete g_LogThread;
		}
	}
}

__NS_BASIC_END

#define CHECKSUPPORTATOMIC(a) \
	{\
		std::atomic<a> tmp;\
		if (!tmp.is_lock_free())\
			return false;\
	}
//判断basic库是否可用
_BASIC_DLL_API bool IsSupportBasiclib()
{
	CHECKSUPPORTATOMIC(bool);
	CHECKSUPPORTATOMIC(char);
	CHECKSUPPORTATOMIC(signed char);
	CHECKSUPPORTATOMIC(unsigned char);
	CHECKSUPPORTATOMIC(short);
	CHECKSUPPORTATOMIC(unsigned short);
	CHECKSUPPORTATOMIC(int);
	CHECKSUPPORTATOMIC(unsigned int);
	CHECKSUPPORTATOMIC(unsigned int);
	CHECKSUPPORTATOMIC(long long);
	CHECKSUPPORTATOMIC(unsigned long long);
	CHECKSUPPORTATOMIC(void*);
	return true;
}