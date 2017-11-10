#include "../inc/basic.h"

/////////////////////////////////////////////////////////////////
__NS_BASIC_START
CBasicException::CBasicException()
{
}

CBasicException::~CBasicException()
{
}

void CBasicException::BeforeQuit()
{
}

extern int     g_nStartMode;
bool BasicIsDaemonMode()
{
	return (g_nStartMode&BASIC_DAEMONMODE);
}

extern GlobalShutdownFunc g_funcShutdown;
void BasicRegisteShutDown(GlobalShutdownFunc funcShutdown)
{
	g_funcShutdown = funcShutdown;
}

#if	!(defined(__LINUX) || defined(__MAC) || defined(__ANDROID))
//!注册异常回调函数处理
extern GlobalExceptionFunc g_funcException;
void BasicRegisteExceptionFunction(GlobalExceptionFunc funcException)
{
    g_funcException = funcException;
}
#endif

__NS_BASIC_END
