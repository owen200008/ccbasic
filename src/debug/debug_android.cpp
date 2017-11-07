/***************************************************************************************************
* Copyright (c) 2014, 业联网路有限公司.All rights reserved
* 文件名称: debug_android.cpp
* 摘    要: debug  android版函数
* 当前版本: 1.0
* 作    者: 宋春光
* 完成日期: 2014-6-3
************************************************************************************************/
#include "../inc/basic.h"

#ifdef __ANDROID

__NS_BASIC_START

/**********************************************************************************************
* 函数介绍：将lpszFormat输出到debug窗口和标准输出文件
* 输入参数：lpszFormat:格式化字符串，arglist:参数列表
* 输出参数：
* 返回值  ：
**********************************************************************************************/
void WBasicTraceDebugView(LPCTSTR lpszString)
{
	BASICLOG_INFO_A("%s", lpszString);
}
void BasicTraceDebugView(const char* lpszString)
{
	BASICLOG_INFO_A("%s", lpszString);
}

__NS_BASIC_END

#endif
