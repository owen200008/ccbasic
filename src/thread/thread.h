/***********************************************************************************************
// 文件名:     thread.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:01:19
// 内容描述:   线程操作模型
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_THREAD_H
#define BASIC_THREAD_H

#if defined(__LINUX) || defined(__MAC) || defined(__ANDROID)
#define HAVE_PTHREAD_H
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#else
#ifdef __BASICWINDOWS
#include <windows.h>
#include <process.h>
#endif
#endif

#if defined HAVE_PTHREAD_H
#define CRFPP_USE_THREAD 1
#endif

__NS_BASIC_START
//////////////////////////////////////////////////////////////////////////////////////////
//!线程的函数操作模型
//!
//线程工作函数返回值定义
#ifdef __BASICWINDOWS
#define THREAD_RETURN DWORD		//!< Windows线程函数返回值
#else
#define THREAD_RETURN void*		//!< Linux线程函数返回值
#endif

//!
//线程工作函数定义
//如：THREAD_RETURN DemoProc(void* pParam);
typedef THREAD_RETURN ( *PBASIC_THREAD_START_ROUTINE)( void* lpThreadParameter );
typedef PBASIC_THREAD_START_ROUTINE LPBASIC_THREAD_START_ROUTINE;

/*!
* 创建线程
* \param 工作函数指针、工作函数参数
* \return 返回 线程ID
*/
_BASIC_DLL_API HANDLE BasicCreateThread(LPBASIC_THREAD_START_ROUTINE lpStartAddress, void* lpParameter, LPDWORD lpThreadId);

/*!
* 等待线程退出
* \param hThread 线程句柄
* \param dwWaitTime 等待退出的时间(毫秒)
* \return 句柄无效或者成功退出返回true，超时返回false
*/
_BASIC_DLL_API BOOL BasicWaitThread(HANDLE hThread, DWORD  dwWaitTime = INFINITE);

/*!
* 强行结束线程
* \param hThread 线程句柄
* \return 无
*/
_BASIC_DLL_API void BasicTerminateThread(HANDLE hThread);

/*!
* 取得线程ID
* \return 线程ID
*/
_BASIC_DLL_API DWORD BasicGetCurrentThreadId();

/*!
* 取得线程
* \return 线程
*/
_BASIC_DLL_API HANDLE Basic_GetCurrentThread();

/*!
* 取得进程ID
* \return 进程ID
*/
_BASIC_DLL_API DWORD Basic_GetCurrentProcessId();


/*!
* 休眠
* \param dwMilliseconds 休眠时间，单位毫秒
* \return 无
*/
_BASIC_DLL_API_C  _BASIC_DLL_API void BasicSleep( DWORD dwMilliseconds );

//////////////////////////////////////////////////////////////////////////////////////////
//!线程的类操作模型
#if(defined(_WIN32) && ! defined (__CYGWIN__))
#define CRFPP_USE_THREAD 1
#define BEGINTHREAD(src, stack, func, arg, flag, id) \
     (HANDLE)_beginthreadex((void *)(src),(unsigned)(stack), \
                       (unsigned(_stdcall *)(void *))(func),(void *)(arg), \
                       (unsigned)(flag),(unsigned *)(id))
#endif

//!
//线程类
class CBasic_Thread : public CBasicObject
{
public:

	//! 线程函数
	/*! 
	*/
	static THREAD_RETURN Wrapper(void *ptr);

	//! 线程过程，派生实现
	/*! 
	*/
    virtual void Run() {}

	//! 启动线程
	/*! 
	*/
    void Start();

	//! 等待线程结束
	/*! 
	*/
	void Join();

    virtual ~CBasic_Thread() {};

private:
#ifdef HAVE_PTHREAD_H
	pthread_t hnd_;		//!< Linux线程类型
#else
#ifdef _WIN32
	HANDLE  hnd_;		//!< Windows线程句柄
#endif
#endif

};

#ifdef __BASICWINDOWS
#define BasicTLS_Key	DWORD
#else
#define BasicTLS_Key	pthread_key_t
#endif
class _BASIC_DLL_API CBasicThreadTLS : basiclib::CBasicObject
{
public:
	CBasicThreadTLS();
	virtual ~CBasicThreadTLS();

	bool CreateTLS();
	void* GetValue();
	BOOL SetValue(void* pValue);
protected:
	bool			m_bCreate;
	BasicTLS_Key	m_key;
};

__NS_BASIC_END

#endif 
