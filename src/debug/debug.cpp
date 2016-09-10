#include "../inc/basic.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

__NS_BASIC_START

/////////////////////////////////////////////////////////////////////////////////////////
//
BOOL BasicAssertFailedLine(const char* lpszFileName, int nLine)
{
	BasicTrace("ASSERT @%s:%d\n", lpszFileName, nLine);
	return TRUE;
}

//
void BasicDebugBreak()
{
	assert(FALSE);
}

void BasicTraceV(const char* lpszFormat, va_list argList)
{
	char szTraceBuff[256];
	memset(szTraceBuff, 0, sizeof(szTraceBuff));
	vsnprintf(szTraceBuff, sizeof(szTraceBuff) / sizeof(TCHAR)-1, lpszFormat, argList);

	//OutputDebugString(szTraceBuff);
	BasicTraceDebugView(szTraceBuff);
}

void BasicTrace(const char* lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	BasicTraceV(lpszFormat, argList);
	va_end(argList);
}
//////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END

