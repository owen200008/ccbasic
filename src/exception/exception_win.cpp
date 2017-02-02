#ifdef __BASICWINDOWS

// #ifndef _WIN32_WINNT
// #ifdef WINVER
// #define _WIN32_WINNT WINVER
// #else
// #pragma message("_WIN32_WINNT not defined. Defaulting to 0x0502 (Windows Server 2003)")
// #define _WIN32_WINNT 0x0502
// #endif
// #else
// #if _WIN32_WINNT < 0x0400
// #error requires _WIN32_WINNT to be #defined to 0x0400 or greater
// #endif
// #endif

#include "../inc/basic.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <direct.h>
#pragma pack(8)	//code generate by 8 bytes
#include <dbghelp.h>
#pragma pack()

#include "exception_win.h"

__NS_BASIC_START

#pragma comment(linker, "/defaultlib:dbghelp.lib")

#pragma warning( disable:4996)

//============================== Global Variables =============================

//
// Declare the static variables of the WheatyExceptionReport class
//
TCHAR CBasicWinExceptionReport::m_szLogFileName[MAX_PATH];
TCHAR CBasicWinExceptionReport::m_szDumpFileName[MAX_PATH];
LPTOP_LEVEL_EXCEPTION_FILTER CBasicWinExceptionReport::m_previousFilter;
HANDLE CBasicWinExceptionReport::m_hProcess;
BOOL CBasicWinExceptionReport::m_bRestart = TRUE;
BOOL CBasicWinExceptionReport::m_bToClose = FALSE;

// Declare global instance of class
CBasicWinExceptionReport* g_WheatyExceptionReport = NULL;

int     g_nStartMode = 0;
//
GlobalShutdownFunc g_funcShutdown = NULL;
GlobalExceptionFunc g_funcException = NULL;
// 设置异常保护模式
//
int KillExistProcess()
{
	CWBasicString strMyModuleName = WBasicGetModuleTitle(NULL, TRUE);
	CWBasicString strMyModulePath = WBasicGetModulePath(NULL);
	int nKill = 0;
	DWORD dwCurProcessID = Basic_GetCurrentProcessId();
	PROCESSLIST* pList = BasicCreateProcessEntry();
	PROCESSLIST* pOffset = pList;
	while (pOffset != NULL)
	{
		if (_tcsicmp(pOffset->m_szExeFile, (LPCTSTR)strMyModuleName.c_str()) == 0 &&
			_tcsicmp(pOffset->m_szExePath, (LPCTSTR)strMyModulePath.c_str()) == 0 &&
			dwCurProcessID != pOffset->m_dwProcessID)
		{
			HANDLE hToKill = OpenProcess(PROCESS_TERMINATE, FALSE, pOffset->m_dwProcessID);
			if(hToKill != NULL)
			{
				BOOL bSucc = ::TerminateProcess(hToKill, -1);
				::CloseHandle(hToKill);
				nKill ++;
			}
		}
		pOffset = pOffset->m_pNext;
	}
	BasicReleaseProcessEntry(pList);

	return nKill;
}

BOOL WINAPI ConsoleHandlerRoutine( DWORD dwCtrlType)
{
	switch(dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if(g_funcShutdown)
		{
			g_funcShutdown();
		}
		break;
	}
	return TRUE;
}

//
// 设置异常保护模式
//
void BasicSetExceptionMode(int nMode, int nInstance)
{
	g_nStartMode = nMode;
	if(nMode&BASIC_EXCEPTION_DISABLE)
	{
		//禁用异常保护
		SetUnhandledExceptionFilter(NULL);
		return;
	}
	//
	if(nMode&BASIC_EXCEPTION_KILLEXIST)
	{
		//杀掉已有进程
		KillExistProcess();
	}
	//
	if(g_WheatyExceptionReport == NULL)
	{
		g_WheatyExceptionReport = new CBasicWinExceptionReport;
	}
	//
	if(nMode&BASIC_EXCEPTION_NORESTART)
	{
		//不重启
		CBasicWinExceptionReport::m_bRestart = FALSE;
	}
	//
	if(nMode&BASIC_EXCEPTION_NOLOG)
	{
		//不记录日志
		CBasicWinExceptionReport::m_bLog = FALSE;
	}

	//控制台模式, win没有服务模式
	SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
	g_nStartMode &= ~BASIC_DAEMONMODE;
}

//
// 应用程序关闭前调用，忽略退出时出现的异常
//
void BasicClearException()
{
	CBasicWinExceptionReport::m_bToClose = TRUE;
	if(g_WheatyExceptionReport != NULL)
	{
		g_WheatyExceptionReport->BeforeQuit();
		delete g_WheatyExceptionReport;
		g_WheatyExceptionReport = NULL;
	}
}

//
//重启进程
//
void BasicRestart()
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	GetStartupInfo(&si);

	TCHAR szProcess[MAX_PATH];
	GetModuleFileName( 0, szProcess, MAX_PATH );

	LPTSTR lpszCommandLine = GetCommandLine();

	CreateProcess(szProcess, lpszCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	int nTry = 0;
	do
	{
		try
		{
			_stprintf(szProcess, _T("restart %d try %d"), pi.dwProcessId, nTry+1);
			OutputDebugString(szProcess);
//#ifdef _DEBUG
//			ExitProcess(0);
//#else
			::TerminateProcess(GetCurrentProcess(), -1);
//#endif
		}
		catch(...)
		{
			nTry++;
		}
	}while(nTry > 0 && nTry < 5);
}

//============================== Class Methods =============================

CBasicWinExceptionReport::CBasicWinExceptionReport()   // Constructor
{
    // Install the unhandled exception filter function
    m_previousFilter =
        SetUnhandledExceptionFilter(WheatyUnhandledExceptionFilter);
		//(PVECTORED_EXCEPTION_HANDLER)AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)WheatyUnhandledExceptionFilter);

	ZeroMemory(m_szLogFileName, sizeof(m_szLogFileName));
	ZeroMemory(m_szDumpFileName, sizeof(m_szDumpFileName));
	TCHAR szModule[MAX_PATH];
    // Figure out what the report file will be named, and store it away
    GetModuleFileName( 0, szModule, MAX_PATH );

	SYSTEMTIME systime;
	GetLocalTime(&systime);

	if(m_bLog)
	{
		PTSTR pszPath = _tcsrchr( szModule, _T('\\') );
		if(pszPath != NULL)
		{
			_tcsncpy(m_szLogFileName, szModule, pszPath-szModule);
		}
		_tcscat(m_szLogFileName, _T("\\exception\\"));
		WBasic_mkdir(m_szLogFileName);

		_tcscpy(m_szDumpFileName, m_szLogFileName);

		_stprintf(szModule, _T("dbg%04d%02d%02d_%02d%02d%02d_%04d.txt"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
		_tcscat(m_szLogFileName, szModule);

		_stprintf(szModule, _T("dbg%04d%02d%02d_%02d%02d%02d_%04d.dmp"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
		_tcscat(m_szDumpFileName, szModule);
	}
    m_hProcess = GetCurrentProcess();
}

//============
// Destructor 
//============
CBasicWinExceptionReport::~CBasicWinExceptionReport()
{
    SetUnhandledExceptionFilter( m_previousFilter );
}

//
//Restart app
//
void CBasicWinExceptionReport::Restart()
{
	BasicRestart();
}

//==============================================================
// Lets user change the name of the report file to be generated 
//==============================================================
void CBasicWinExceptionReport::SetLogFileName( PTSTR pszLogFileName )
{
    __tcscpyn( m_szLogFileName, sizeof(m_szLogFileName), pszLogFileName );
}

extern GlobalExceptionFunc g_funcException;
//===========================================================
// Entry point where control comes on an unhandled exception 
//===========================================================
LONG WINAPI CBasicWinExceptionReport::WheatyUnhandledExceptionFilter(
                                    PEXCEPTION_POINTERS pExceptionInfo )
{
	try
	{
		//生成dump文件
		WriteDumpFile(pExceptionInfo);

        if (g_funcException)
        {
            g_funcException(pExceptionInfo);
        }

		//生成异常日志
		if(m_bLog)
		{
			HANDLE hReportFile = CreateFile(m_szLogFileName,
				GENERIC_WRITE,
				0,
				0,
				OPEN_ALWAYS,
				FILE_FLAG_WRITE_THROUGH,
				0);
			if (hReportFile)
			{
				if (GetFileSize(hReportFile, NULL) > 1024 * 1024)
					SetFilePointer(hReportFile, 1024 * 1024, 0, FILE_BEGIN);
				else
					SetFilePointer(hReportFile, 0, 0, FILE_END);

				GenerateExceptionReport(pExceptionInfo, hReportFile);

				CloseHandle(hReportFile);
			}
		}
	}
	catch (...)
	{
		OutputDebugString(_T("exception in WheatyExceptionReport::WheatyUnhandledExceptionFilter"));
	}

#ifdef _DEBUG
	//调用外部调试器
 	if ( m_previousFilter )
 		return m_previousFilter( pExceptionInfo );
#endif

	if(g_WheatyExceptionReport && m_bRestart && !m_bToClose)
	{
		g_WheatyExceptionReport->Restart();
	 }
#ifdef _DEBUG	
	return EXCEPTION_CONTINUE_SEARCH;
#else
	return EXCEPTION_EXECUTE_HANDLER;
#endif
}

//===========================================================================
// Open the report file, and write the desired information to it.  Called by 
// WheatyUnhandledExceptionFilter                                               
//===========================================================================
void CBasicWinExceptionReport::GenerateExceptionReport(PEXCEPTION_POINTERS pExceptionInfo, HANDLE hFile)
{
	stacktrace::call_stack stack;
	PrintOut(hFile, stack.to_string().c_str());

    // Start out with a banner
	PrintOut(hFile, "//=====================================================\r\n");
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	PrintOut(hFile, "Exception Time: %d-%d-%d %d:%d:%d %03d\r\n",
                 systime.wYear, systime.wMonth, systime.wDay,
				 systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
    PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

    // First print information about the type of fault
	PrintOut(hFile, "Exception code: %08X %s\r\n",
                pExceptionRecord->ExceptionCode,
                GetExceptionString(pExceptionRecord->ExceptionCode));

    PCONTEXT pCtx = pExceptionInfo->ContextRecord;

    // Show the registers
    #ifdef _M_IX86  // X86 Only!
	PrintOut(hFile, "\r\nRegisters:\r\n");

	PrintOut(hFile, "EAX:%08X\r\nEBX:%08X\r\nECX:%08X\r\nEDX:%08X\r\nESI:%08X\r\nEDI:%08X\r\n"
            ,pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx,
            pCtx->Esi, pCtx->Edi );

	PrintOut(hFile, "CS:EIP:%04X:%08X\r\n", pCtx->SegCs, pCtx->Eip);
	PrintOut(hFile, "SS:ESP:%04X:%08X  EBP:%08X\r\n", pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
	PrintOut(hFile, "DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
	PrintOut(hFile, "Flags:%08X\r\n", pCtx->EFlags);

    #endif

    SymSetOptions( SYMOPT_DEFERRED_LOADS );

    // Initialize DbgHelp
    if ( !SymInitialize( GetCurrentProcess(), 0, TRUE ) )
        return;

    CONTEXT trashableContext = *pCtx;

    SymCleanup( GetCurrentProcess() );

	PrintOut(hFile, "\r\n========================//\r\n");
}

//======================================================================
// Given an exception code, returns a pointer to a static string with a 
// description of the exception                                         
//======================================================================
char* CBasicWinExceptionReport::GetExceptionString( DWORD dwCode )
{
    #define EXCEPTION( x ) case EXCEPTION_##x: return #x;

    switch ( dwCode )
    {
        EXCEPTION( ACCESS_VIOLATION )
        EXCEPTION( DATATYPE_MISALIGNMENT )
        EXCEPTION( BREAKPOINT )
        EXCEPTION( SINGLE_STEP )
        EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
        EXCEPTION( FLT_DENORMAL_OPERAND )
        EXCEPTION( FLT_DIVIDE_BY_ZERO )
        EXCEPTION( FLT_INEXACT_RESULT )
        EXCEPTION( FLT_INVALID_OPERATION )
        EXCEPTION( FLT_OVERFLOW )
        EXCEPTION( FLT_STACK_CHECK )
        EXCEPTION( FLT_UNDERFLOW )
        EXCEPTION( INT_DIVIDE_BY_ZERO )
        EXCEPTION( INT_OVERFLOW )
        EXCEPTION( PRIV_INSTRUCTION )
        EXCEPTION( IN_PAGE_ERROR )
        EXCEPTION( ILLEGAL_INSTRUCTION )
        EXCEPTION( NONCONTINUABLE_EXCEPTION )
        EXCEPTION( STACK_OVERFLOW )
        EXCEPTION( INVALID_DISPOSITION )
        EXCEPTION( GUARD_PAGE )
        EXCEPTION( INVALID_HANDLE )
    }

    // If not one of the "known" exceptions, try to get the string
    // from NTDLL.DLL's message table.

    static char szBuffer[512] = { 0 };

    FormatMessageA( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                   GetModuleHandleA("NTDLL.DLL"),
                   dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

	return szBuffer;
}

//============================================================================
// Helper function that writes to the report file, and allows the user to use 
// printf style formating                                                     
//============================================================================
int __cdecl CBasicWinExceptionReport::PrintOut(HANDLE hFile, const char* format, ...)
{
    char szBuff[1024];
    int retValue;
    DWORD cbWritten;
    va_list argptr;
          
    va_start( argptr, format );
	retValue = vsprintf(szBuff, format, argptr);
    va_end( argptr );

	WriteFile(hFile, szBuff, retValue * sizeof(char), &cbWritten, 0);

    return retValue;
}

////////////////////////////////////////////////////////////////////////
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(
	IN HANDLE hProcess,
	IN DWORD ProcessId,
	IN HANDLE hFile,
	IN MINIDUMP_TYPE DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
	IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
	IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
	);

//
// 生成dump文件
//
long CBasicWinExceptionReport::WriteDumpFile( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	long ret = EXCEPTION_CONTINUE_SEARCH;
	TCHAR szDbgHelpPath[_MAX_PATH] = {0};
	TCHAR szPath[_MAX_PATH] = {0};
	if (GetModuleFileName(NULL, szPath, _MAX_PATH))
	{
		TCHAR szDrive[_MAX_DRIVE] = {0};
		TCHAR szDir[_MAX_DIR] = {0};
		TCHAR szFileName[_MAX_FNAME] = {0};

		_tsplitpath(szPath, szDrive, szDir, szFileName, 0);
		_tcsncat(szDbgHelpPath, szDrive, _MAX_PATH);
		_tcsncat(szDbgHelpPath, szDir, _MAX_PATH - _tcslen(szDbgHelpPath) - 1);
		_tcsncat(szDbgHelpPath, _T("dbghelp.dll"), _MAX_PATH - _tcslen(szDbgHelpPath) - 1);

	}

	HMODULE hDll = ::LoadLibrary(szDbgHelpPath);
	if (hDll==NULL)
		hDll = ::LoadLibrary(_T("dbghelp.dll"));
	assert(hDll);
	if (hDll)
	{

		MINIDUMPWRITEDUMP pWriteDumpFun = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
		if (pWriteDumpFun)
		{
			// create the file
			HANDLE hFile = ::CreateFile(m_szDumpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pExceptionInfo;
				ExInfo.ClientPointers = FALSE;
				// write the dump
				if (pWriteDumpFun(GetCurrentProcess(), GetCurrentProcessId(),
					hFile, MiniDumpNormal, pExceptionInfo!=0? &ExInfo: 0, NULL, NULL))
					ret = EXCEPTION_EXECUTE_HANDLER;
				::CloseHandle(hFile);
			}
		}
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END
#endif //__BASICWINDOWS



