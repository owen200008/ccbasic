/***********************************************************************************************
// 文件名:     extern.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 9:15:06
// 内容描述:   MFC like的扩展容器类。
主要用于使用MFC的代码移植。不推荐使用。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_EXTERN_H
#define BASIC_EXTERN_H

#pragma once


#include "../inc/basic_def.h"
#include "array.h"
#include "list.h"
#include "map.h"
#include "basic_string.h"

__NS_BASIC_START
 
typedef CArray<unsigned char, unsigned char>	CByteArray;
typedef CArray<unsigned short, unsigned short>	CWordArray;
typedef CArray<unsigned long, unsigned long>	CDWordArray;
typedef CArray<unsigned int, unsigned int>		CUintArray;
typedef CArray<void*, void*>					CPtrArray;
typedef CArray<CBasicString, const char*>		CBasicStringArray;
/*!class CPtrList</br>
用于替换MFC中的CPtrList
*/
typedef CList<void*, void*>						CPtrList;
 
typedef CMap<unsigned short, unsigned short, void*, void*>	CMapWordToPtr;
typedef CMap<void*, void*, unsigned short, unsigned short>	CMapPtrToWord;
typedef CMap<void*, void*, void*, void*>					CMapPtrToPtr;
typedef CMap<CBasicString, const char*, CBasicString, const char*>			CMapStringToString_S;




__NS_BASIC_END

#endif 
