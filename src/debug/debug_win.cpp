
#include "../inc/basic.h"

#ifdef __BASICWINDOWS

__NS_BASIC_START

/**********************************************************************************************
* 函数介绍：将lpszFormat输出到debug窗口和标准输出文件
* 输入参数：lpszFormat:格式化字符串，arglist:参数列表
* 输出参数：
* 返回值  ：
**********************************************************************************************/

void WBasicTraceDebugView(LPCTSTR lpszString)
{
	OutputDebugString(lpszString);
	wprintf(lpszString);
}
void BasicTraceDebugView(const char* lpszString)
{
	OutputDebugStringA(lpszString);
	printf(lpszString);
}

__NS_BASIC_END

#endif