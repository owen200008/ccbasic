/***********************************************************************************************
// 文件名:     private.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:16:47
// 内容描述:   内部使用的一些用于traits的私有命名空间的实用对象
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_PRIVATE_H
#define BASIC_PRIVATE_H
#pragma once

#include <string>
using namespace std;
#include "../inc/basic_def.h"

__NS_BASIC_START

namespace __private
{
	template<typename T>
	struct Type2Type
	{
		typedef T Result;
	};


	struct True{};
	struct False{};

	template<typename T, typename U>
	struct AndOp
	{
		typedef False Result;
	};

	template<>
	struct AndOp<True, True>
	{
		typedef True Result;
	};

	template<typename T, typename U>
	struct OrOp
	{
		typedef True Result;
	};

	template<>
	struct OrOp<False, False>
	{
		typedef False Result;
	};


	template<typename T, typename U>
	struct XorOp
	{
		typedef True Result;
	};

	template<>
	struct XorOp<True, True>
	{
		typedef False Result;
	};

	template<>
	struct XorOp<False, False>
	{
		typedef False Result;
	};


	template<typename T>
	struct IsString
	{
		typedef False	Result;
	};

	#define MarkString(type)	template<>\
								struct IsString <type>\
								{\
									typedef True	Result;\
								}


	MarkString(string);
	MarkString(string&);
	MarkString(wstring);
	MarkString(wstring&);
//	MarkString(char);
//	MarkString(wchar_t);
	MarkString(char*);
	MarkString(wchar_t*);
	MarkString(const char*);
	MarkString(const wchar_t*);
	MarkString(const char[]);
	MarkString(const wchar_t[]);
}

__NS_BASIC_END

#endif 

