
#include "../inc/basic.h"

#ifdef __MAC

__NS_BASIC_START

/**********************************************************************************************
* 函数介绍：将lpszFormat输出到debug窗口
* 输入参数：lpszFormat:格式化字符串，arglist:参数列表
* 输出参数：
* 返回值  ：
**********************************************************************************************/

void OutputDebugString(const char* lpszString)
{
    fprintf(stdout, lpszString);
    fflush(stdout);
}
#define OutputDebugStringA OutputDebugString;
void WBasicTraceDebugView(LPCTSTR lpszString)
{
	OutputDebugString(lpszString);
}
void BasicTraceDebugView(const char* lpszString)
{
	OutputDebugStringA(lpszString);
}

__NS_BASIC_END

#endif