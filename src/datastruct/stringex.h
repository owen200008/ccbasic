/***********************************************************************************************
// 文件名:     stringex.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 9:17:44
// 内容描述:   支持外部加载字符串资源
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_STRINGEX_H
#define BASIC_STRINGEX_H

__NS_BASIC_START

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*\brief Basic_LoadStringSource 加载字符串资源文件
*
*\param lpszFile 字符串资源文件名 全路径 ini格式：ID=字符串
*\param lpszModuleName 模块名称，当定义不同资源文件时可用，模块内必须保持ID唯一
*\return
*/
bool BasicLoadStringSource(const char* lpszFile, const char* lpszModuleName);

/**
*\brief BASIC_LoadString 加载字符串资源
*
*\param lpszFormatID
*\param lpszModuleName
*\return
*/
const char* BasicLoadString(const char* lpszFormatID, const char* lpszModuleName);
const char* BasicLoadString(unsigned long ulID, const char* lpszModuleName);
__NS_BASIC_END

#endif 


