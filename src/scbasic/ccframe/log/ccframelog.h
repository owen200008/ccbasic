#ifndef SCBASIC_CCFRAME_LOG_H
#define SCBASIC_CCFRAME_LOG_H

#include "../../scbaisc_head.h"

//!事件记录
/*!
* 写到默认日志文件
*\remarks 日志文件默认路径可通过BasicSetLogEventMode设置
*\remarks 可格式化参数
*/
_SCBASIC_EXTERNC  _SCBASIC_DLL_API void CCFrameSCBasicLogEventV(const char* pszLog, ...);
_SCBASIC_EXTERNC  _SCBASIC_DLL_API void CCFrameSCBasicLogEventErrorV(const char* pszLog, ...);

//!事件记录
/*!
* 写到默认日志文件
*\remarks 日志文件默认路径可通过BasicSetLogEventMode设置
*/
_SCBASIC_EXTERNC  _SCBASIC_DLL_API void CCFrameSCBasicLogEvent(const char* pszLog);
_SCBASIC_EXTERNC  _SCBASIC_DLL_API void CCFrameSCBasicLogEventError(const char* pszLog);
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
#define LOG_SIZE_LIMIT	0x0000ffff		//!< 日志文件大小限制	单位：MB
#define LOG_NAME_DAY_S	"S%DAY%"


#define LOG_ERROR_NAME_EMPTY		-1	//!< 文件名为空
#define LOG_ERROR_OPEN_FILE			-2	//!< 打开文件失败
#define LOG_ERROR_FULL				-3	//!< 日志记录通道已经满了。
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
_SCBASIC_EXTERNC  _SCBASIC_DLL_API uint32_t CCFrameSCBasicSetDefaultLogEventMode(long nOption, const char* pszLogFile);
_SCBASIC_EXTERNC  _SCBASIC_DLL_API uint32_t CCFrameSCBasicSetDefaultLogEventErrorMode(long nOption, const char* pszLogFile);
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
_SCBASIC_EXTERNC  _SCBASIC_DLL_API uint32_t CCFrameSCBasicSetLogEventMode(long nOption, const char* pszLogFile);
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
_SCBASIC_EXTERNC  _SCBASIC_DLL_API uint32_t CCFrameSCBasicCloseLogEvent(long lLogChannel);
//

//
//!事件记录（写到日志文件）
/*
*\remarks 日志文件路径可通过BasicSetLogEventMode设置
*\remarks 可格式化参数
*/
_SCBASIC_EXTERNC _SCBASIC_DLL_API void CCFrameSCBasicLogEventV(long lLogChannel, const char* pszLog, ...);
//
//
//
//


//
//!事件记录（写到日志文件）
/*
*\remarks 日志文件路径可通过BasicSetLogEventMode设置
*/
_SCBASIC_EXTERNC _SCBASIC_DLL_API void BasicLogEvent(long lLogChannel, const char* pszLog);
//
//
//


#endif