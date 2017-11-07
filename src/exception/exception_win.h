#ifndef _WIN_EXCEPTION_H
#define _WIN_EXCEPTION_H

#include "exception.h"
#include <winsvc.h>
#pragma once

__NS_BASIC_START

enum BasicType  // Stolen from CVCONST.H in the DIA 2.0 SDK
{
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
};

//
//应用程序异常保护处理类 for Windows
//
// 异常保护原理：通过系统调用SetUnhandledExceptionFilter，捕获程序异常
// 堆栈回溯和符号表解析通过dbghelp API实现
// 对于DEBUG版，可解析当前路径中对应的pdb文件符号表
// 
//
class CBasicWinExceptionReport :public CBasicException
{
public:
    
    CBasicWinExceptionReport();
    ~CBasicWinExceptionReport();
    
	virtual void Restart();

	void SetLogFileName( PTSTR pszLogFileName );


    // entry point where control comes on an unhandled exception
    static LONG WINAPI WheatyUnhandledExceptionFilter(
                                PEXCEPTION_POINTERS pExceptionInfo );

    private:

	// where report info is extracted and generated 
    static void GenerateExceptionReport( PEXCEPTION_POINTERS pExceptionInfo, HANDLE hFile);

    // Helper functions
    static char* GetExceptionString( DWORD dwCode );

	static int __cdecl PrintOut(HANDLE hFile, const char* format, ...);

    // Variables used by the class
    static TCHAR m_szLogFileName[MAX_PATH];
	static TCHAR m_szDumpFileName[MAX_PATH];
    static LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter;
    static HANDLE m_hProcess;
protected:
	//
	// 生成dump文件
	//
	static long WriteDumpFile( struct _EXCEPTION_POINTERS *pExceptionInfo );

	static BOOL m_bRestart;
	static BOOL m_bLog;
    static BOOL m_bToClose;

	friend void BasicSetExceptionMode(int nMode, int nInstance);
	friend void BasicClearException();
};


extern CBasicWinExceptionReport* g_WheatyExceptionReport; //  global instance of class

__NS_BASIC_END
#endif //_WIN_EXCEPTION_H
