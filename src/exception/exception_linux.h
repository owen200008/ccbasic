#ifndef __EXCEPTION_LINUX_H_
#define __EXCEPTION_LINUX_H_

#define ALLOCTO_STACK_MEMORY  8192	//异常堆栈的大小

__NS_BASIC_START

class CBasicExceptionLinux : public CBasicException
{
public:
	CBasicExceptionLinux();
	~CBasicExceptionLinux();

	virtual void BeforeQuit();
	virtual void Restart();

	void SetDaemonMode(int nInstance = 0);

protected:
	void CatchSignal();

protected:
	char m_pszLockFile[_MAX_PATH];

	//分配一块内存作为异常处理的堆栈
	void *	m_pAllocStack;

	char** 	m_pszCmdline;	//命令行参数
	int	m_nParaNum;	//命令行参数个数
};

__NS_BASIC_END

#endif //__EXCEPTION_LINUX_H_
