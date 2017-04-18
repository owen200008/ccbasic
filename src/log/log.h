/***********************************************************************************************
// 文件名:     log.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 13:11:33
// 内容描述:   日志记录函数库
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_LOG_H
#define BASIC_LOG_H

__NS_BASIC_START

//定义debuglevel
enum DebugLevel
{
    DebugLevel_None = 0,
    DebugLevel_Error = 1,
    DebugLevel_Info = 2,
};
//!事件记录
/*!
 * 写到默认日志文件
 *\remarks 日志文件默认路径可通过BasicSetLogEventMode设置
 *\remarks 可格式化参数
 */
_BASIC_DLL_API_C  _BASIC_DLL_API void BasicLogEventV(DebugLevel level, const char* pszLog, ...);
_BASIC_DLL_API_C  _BASIC_DLL_API void BasicLogEventErrorV(const char* pszLog, ...);

//!事件记录
/*!
* 写到默认日志文件
*\remarks 日志文件默认路径可通过BasicSetLogEventMode设置
*/
_BASIC_DLL_API_C  _BASIC_DLL_API void BasicLogEvent(DebugLevel level, const char* pszLog);
_BASIC_DLL_API_C  _BASIC_DLL_API void BasicLogEventError(const char* pszLog);
//////////////////////////////////////////////////////////////////////////////////////////////
//
//BasicSetLogEventMode选项
#define	LOG_BY_DAY		0x00010000		//!< 按天记录。如果超过限制的大小，一天可能会有多个文件。
#define	LOG_BY_SIZE		0x00020000		//!< 按大小记录。超过限制的大小，新建一个文件。
#define	LOG_BY_ONEFILE	0x00040000		//!< 始终只有一个文件，如果按天，则删除前一天的数据，超过大小限制，删除原来的文件。
#define	LOG_BY_BUFFER	0x00080000		//!< 日志不立刻写盘，先保存在缓存区里面，定时写盘。提高效率。
#define	LOG_BY_OPEN		0x01000000		//!< 日志文件保持打开状态，不关闭。
#define	LOG_BY_NOLIMIT	0x02000000		//!< 不限制文件大小
#define LOG_BY_SAMENAME	0x04000000		//!< 坚决不换名字


//
#define LOG_ADD_TIME	0x00100000		//!< 记录时间
#define LOG_ADD_THREAD	0x00200000		//!< 记录线程ID
//
#define LOG_SIZE_LIMIT	0x0000ffff		//!< 日志文件大小限制	单位：MB
#define LOG_NAME_DAY_S	"S%DAY%"


#define LOG_ERROR_NAME_EMPTY		-1	//!< 文件名为空
#define LOG_ERROR_OPEN_FILE			-2	//!< 打开文件失败
#define LOG_ERROR_FULL				-3	//!< 日志记录通道已经满了。

//! 设置日志记录的级别
_BASIC_DLL_API_C  _BASIC_DLL_API void InitBasicLogLevel(DebugLevel level);
_BASIC_DLL_API_C  _BASIC_DLL_API DebugLevel GetBasicLogLevel();
//! bThreadCheckSelf true 不启动锁(无法实时记录) false启动锁 线程安全(不开启buffer模式实时)
_BASIC_DLL_API_C  _BASIC_DLL_API bool InitBasicLog(bool bThreadCheckSelf);
//! 如果不是自己线程需要30s调用一次
_BASIC_DLL_API_C  _BASIC_DLL_API void OnTimerBasicLog();
//
//! 设置默认的事件记录模式
/*
 * 设置后全局有效，在应用程序启动时调用一次即可
 *
 *\param		nOption:	记录模式LOG_*
 *\param		pszLogFile:	日志文件名,如带有路径，会动态创建
 *					
 *\return		= 0 默认的日志句柄
 *				<0 错误
 *\remarks  只能用于设置默认的通道，即通道ID = 0 ，如果不设置，使用文件名 basiclib.log
 */
_BASIC_DLL_API_C  _BASIC_DLL_API long BasicSetDefaultLogEventMode(long nOption, const char* pszLogFile);
_BASIC_DLL_API_C  _BASIC_DLL_API long BasicSetDefaultLogEventErrorMode(long nOption, const char* pszLogFile);
//

//
//! 设置事件记录模式
/*
 * 设置后全局有效，在应用程序启动时调用一次即可
 *
 *\param		nOption:	记录模式LOG_*
 *\param		pszLogFile:	日志文件名,如带有路径，会动态创建
 *					
 *\return		>0 日志句柄，用于传递给BasicLogEvent
 *				<0 错误
 */
_BASIC_DLL_API_C  _BASIC_DLL_API long BasicSetLogEventMode(long nOption, const char* pszLogFile);
//

//! 关闭日志记录
/*
 * 在程序退出前或者关闭某个日志记录的时候调用
 *
 *\param		lLogChannel:	日志句柄，函数 BasicSetLogEventMode 的返回值 如果小于 0 ，关闭所有的
 *					
 *\return		>0 日志句柄，用于传递给BasicLogEvent
 *				<0 错误
 */
_BASIC_DLL_API_C  _BASIC_DLL_API long BasicCloseLogEvent(long lLogChannel);
//

//
//!事件记录（写到日志文件）
/*
 *\remarks 日志文件路径可通过BasicSetLogEventMode设置
 *\remarks 可格式化参数
 */
_BASIC_DLL_API void BasicLogEventV(DebugLevel level, long lLogChannel, const char* pszLog, ...);
//
//
//
//


//
//!事件记录（写到日志文件）
/*
 *\remarks 日志文件路径可通过BasicSetLogEventMode设置
 */
_BASIC_DLL_API void BasicLogEvent(DebugLevel level, long lLogChannel, const char* pszLog);
//
//
//

class _BASIC_DLL_API WriteLogDataBuffer : basiclib::CBasicObject
{
public:
    WriteLogDataBuffer();
    virtual ~WriteLogDataBuffer();

    void InitLogData(const char* lpszText);
    void CopyLogData(WriteLogDataBuffer& logData);

    void ClearLogData();
protected:
    void InitMember();
public:
	char*		m_pText;
	time_t		m_lCurTime;
	DWORD		m_dwProcessId;
	DWORD		m_dwThreadId;
};
_BASIC_DLL_API_C  _BASIC_DLL_API void BasicWriteByLogDataBuffer(long lLogChannel, WriteLogDataBuffer& data, bool bThreadSafe);

__NS_BASIC_END

#endif 
