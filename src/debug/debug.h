/***********************************************************************************************
// 文件名:     debug.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 8:33:33
// 内容描述:   
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_DEBUG_H
#define BASIC_DEBUG_H

/////////////////////////////////////////////////////////////////////////////////////////////
//! 断言，DEBUG版有效，RELEASE版无效
#ifndef ASSERT

#ifdef _DEBUG

#ifdef __BASICWINDOWS

#define ASSERT(f) \
	do \
	{ \
	if (!(f) && basiclib::BasicAssertFailedLine(__FILE__, __LINE__)) \
	basiclib::BasicDebugBreak(); \
	} while (0) \

#endif	//__BASICWINDOWS

#if (defined(__LINUX) || defined(__MAC) || defined(__ANDROID))
#include <assert.h>
#define ASSERT assert
#endif //__LINUX

#else	//_DEBUG

#define ASSERT(f)          ((void)0)

#endif //_DEBUG

#endif //ASSERT

#ifdef __ANDROID
#include <android/log.h>
#endif //__ANDROID

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//! 断言，DEBUG版有效，RELEASE版忽略断言
#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(f)          ASSERT(f)
#else
#define VERIFY(f)          ((void)(f))
#endif
#endif //VERIFY

/////////////////////////////////////////////////////////////////////////////////////////////
//下面的宏定义 用于 debug 版本，或者需要根据输出或者性能的版本（需要定义宏 _BASIC_TRACE）。
#if defined(_DEBUG) || defined(_BASIC_TRACE)
#define __basic_trace
#endif
/////////////////////////////////////////////////////////////////////////////////////////////
//! 输出跟踪信息
#ifndef TRACE
#ifdef __basic_trace
#define WTRACE              basiclib::WBasicTrace
#define TRACE              basiclib::BasicTrace
#else
#define WTRACE              
#define TRACE
#endif //__basic_trace
#endif //TRACE

#ifdef __ANDROID
#define LOG_TAG "C++"
#ifdef _DEBUG
#define BASICLOG_WARN_A(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define BASICLOG_INFO_A(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define BASICLOG_ERROR_A(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define BASICLOG_FATAL_A(...) __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)
#else
#define BASICLOG_WARN_A(...)
#define BASICLOG_INFO_A(...)
#define BASICLOG_ERROR_A(...)
#define BASICLOG_FATAL_A(...)
#endif	// __DEBUG __android_log_write
#endif////__ANDROID

//!定位断言代码行
BOOL BasicAssertFailedLine(const char* lpszFileName, int nLine);

//!中断
void BasicDebugBreak();

//!输出跟踪信息
void BasicTrace(const char* lpszFormat, ...);

//!输出跟踪信息到调试器
void BasicTraceDebugView(const char* lpszString);

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_END

#endif 
