/***********************************************************************************************
// 文件名:     exception.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:20:31
// 内容描述:   程序异常处理基类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_EXCEPTION_H
#define BASIC_EXCEPTION_H

///////////////////////////////////////////////////////////////////
__NS_BASIC_START


class CBasicException;
/*!\class CBasicException
 * \brief CBasicException class.
 *
 * 程序异常处理基类
 *
 * 应用程序编译时包含本模块，就可实现程序异常保护
 * 默认方式：捕获异常后自动重启，并记录在日志文件
 * 禁用或修改保护方式，通过调用全局函数BasicSetExceptionMode实现
 *
 * 异常记录文件:
 * Windows: 生成到当前进程目录下的exception目录，命名规则dbgYYYYMMDD.txt
 * Linux: 生成到/tmp下，命名规则：进程名.异常线程ID.进程ID
*/

class _BASIC_DLL_API CBasicException : public basiclib::CBasicObject
{
public:
	CBasicException();
	~CBasicException();

	//
	//程序退出时调用
	virtual void BeforeQuit();
	//
	//重新启动，
	virtual void Restart() = 0;

	friend void SetExceptionMode(int nMode);
};


//BasicSetExceptionMode选项
#define BASIC_EXCEPTION_DISABLE			0x10000000	//!< 禁止异常保护
#define BASIC_EXCEPTION_NORESTART		0x00000001	//!< 捕获异常后，不需要重启
#define BASIC_EXCEPTION_NOLOG			0x00000002	//!< 不记录异常日志
#define BASIC_DAEMONMODE				0x00000010	//!< 设置为后台运行模式
#define BASIC_WINDOWSRUNDAEMON    		0x00000020  //!< windows服务模式
#define BASIC_EXCEPTION_KILLEXIST		0x20000000	//!< 启动时杀掉已有进程
#define BASIC_EXCEPTION_DUMP			0x80000000	//!< 堆栈DUMP标记
#define BIT_POS_DUMP					31

/*! \fn void SetExceptionMode(int nMode);
 * \brief  设置异常保护模式
 * \param  nMode 异常保护模式 BASIC_EXCEPTION_*
*/
_BASIC_DLL_API void BasicSetExceptionMode(int nMode = 0, int nInstance = 0);
//

/*! \fn void BasicClearException();
* \brief  退出前取消异常保护模式
*/
_BASIC_DLL_API void BasicClearException();

/*! \fn bool BasicIsDaemonMode();
* \brief 判断是否后台运行模式
*/
_BASIC_DLL_API bool BasicIsDaemonMode();

/*! \fn void BasicRestart();
* \brief  重启进程
*/
_BASIC_DLL_API void BasicRestart();

/*! \fn void BasicRegisteShutDown();
* \brief  注册退出函数
*/
typedef fastdelegate::FastDelegate0<long> GlobalShutdownFunc;
_BASIC_DLL_API void BasicRegisteShutDown(GlobalShutdownFunc funcShutdown);

#if	!(defined(__LINUX) || defined(__MAC) || defined(__ANDROID))
typedef fastdelegate::FastDelegate1<PEXCEPTION_POINTERS> GlobalExceptionFunc;
//!注册异常回调函数处理
_BASIC_DLL_API void BasicRegisteExceptionFunction(GlobalExceptionFunc funcException);
#endif


__NS_BASIC_END

#endif 
