/***********************************************************************************************
// 文件名:     charset_win.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:19:56
// 内容描述:   编码转换。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_CHARSET_WIN_H
#define BASIC_CHARSET_WIN_H

#pragma once

#ifdef __BASICWINDOWS

#include <string>

using namespace std;

__NS_BASIC_START
namespace __private  
{
	// 宽字符辅助类。将单字节字符串转化为多字节字符串
	class	WideBytes
	{
	public:
		WideBytes(const char* str, int len, unsigned int nCodePage);
		~WideBytes();
		operator wchar_t* () const;
		int Length() const;
		operator bool() const;
	private:
		wchar_t*	__wchars;
		int			__length;
	};


	// 将指定编码的字符串转化为单字符串
	class	MultiBytes
	{
	public:
		MultiBytes(WideBytes& str, unsigned int nCodePage);
		MultiBytes(const wchar_t* str, int len, unsigned int nCodePage);
		~MultiBytes();
		int Length() const;
		operator const char* () const;
		operator bool() const;
	private:
		char*	__chars;
		int		__length;
	};
}

__NS_BASIC_END

#endif	// __BASICWINDOWS
#endif 
