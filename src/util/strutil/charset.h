/***********************************************************************************************
// 文件名:     charset.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:18:55
// 内容描述:   用于编码转换的头文件。
在具体实现和操作系统有关。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_CHARSET_H
#define BASIC_CHARSET_H

#pragma once

#include "charset/charset_def.h"
#include "strutil.h"

__NS_BASIC_START

const char*	CPCodeToCPStr(int nCPCode);
int CPStrToCPCode(const char* strCPStr);

/*!字符串转为UNICODE(UTF-16LE)编码
* \param str [in]源字符串
* \param len [in]源字符串长度
* \param nCodePage [in]源字符串的code page。默认0
* \return 转换后的宽字符串
*/
wchar_string Basic_MultiStringToWideString(const char* str, int len, unsigned int nCodePage = 0);

/*!字符串转为UNICODE(UTF-16LE)编码
* \param src [in]源字符串
* \param len [in]源字符串长度
* \param dest [out]转换后输出的字符串。
* \param nCodePage [in]源字符串的code page
* \return 转换后的宽字符串,同dest
*/
wchar_string& Basic_MultiStringToWideString(const char* src, int len, wchar_string& dest, unsigned int nCodePage = 0);

/*!UNICODE(UTF-16)编码的字符串转为目标字符串编码
* \param str [in]源字符串
* \param len [in]源字符串长度
* \param nCodePage [in]目标字符串的code page
* \return 转换后的字符串
*/
char_string Basic_WideStringToMultiString(const WCHAR* str, int len, unsigned int nCodePage = 0);

/*!UNICODE(UTF-16)编码的字符串转为目标字符串编码
* \param src [in]源字符串
* \param len [in]源字符串长度
* \param dest [out]目标字符串
* \param nCodePage [in]目标字符串的code page
* \return 转换后字符串,同dest
*/
char_string& Basic_WideStringToMultiString(const WCHAR* src, int len, char_string& dest, unsigned int nCodePage = 0);


/*!两种非UTF-16编码间的转换
* \param str [in]源字符串
* \param len [in]源字符串长度
* \param nCPFrom [in]源字符串的code page
* \param nCPTo [in]目标字符串的code page
* \return 转换后的字符串
*/
char_string Basic_MultiStringToMultiString(const char* str, int len, unsigned int nCPFrom, unsigned int nCPTo);

/*!两种非UTF-16编码间的转换
* \param src [in]源字符串
* \param len [in]源字符串长度
* \param dest [out]目标字符串
* \param nCPFrom [in]源字符串的code page
* \param nCPTo [in]目标字符串的code page
* \return  转换后字符串的长度
*/
char_string& Basic_MultiStringToMultiString(const char* src, int len, char_string& dest, unsigned int nCPFrom, unsigned int nCPTo);


/*!字符串转化为UTF8
* \param str [in]源字符串。非UTF-16编码
* \param len [in]源字符串长度
* \param nCodePage [in]原字符串code page
* \return 转换后的字符串。
*/
char_string Basic_MultiStringToUTF8(const char* str, int len, unsigned int nCodePage = 0);

/*!字符串转化为UTF8,str为非UNICODE6编码
* \param src [in]源字符串。非UTF-16编码
* \param len [in]源字符串长度
* \param dest [out]目标字符串
* \param nCodePage [in]源字符串code page
* \return 转换后的字符串。同dest
*/
char_string& Basic_MultiStringToUTF8(const char* src, int len, char_string& dest, unsigned int nCodePage = 0);

//---------------------8<------------------code page 不同的分割线--------------------

/*!字符串转为UNICODE(UTF-16)编码
* \param str [in]源字符串。非UTF-16编码
* \param len [in]源字符串长度
* \param from [in]原字符串编码
* \return 转换后的字符串
*/
wchar_string Basic_MultiStringToWideString(const char* str, int len, const char* from);

/*!字符串转为UNICODE(UTF-16)编码
* \param src [in]源字符串。非UTF-16编码
* \param len [in]源字符串长度
* \param dest [out]目标字符串
* \param from [in]原字符串编码
* \return 转换后的字符串，同dest
*/
wchar_string& Basic_MultiStringToWideString(const char* src, int len, wchar_string& dest, const char* from);

/*!UNICODE(UTF-16)编码的字符串转为目标字符串编码
* \param str [in]源字符串
* \param len [in]源字符串长度
* \param to [in]目标字符串的code page
* \return char_string 转换后的字符串
*/
char_string	Basic_WideStringToMultiString(const WCHAR* str, int len, const char* to);


/*!UNICODE(UTF-16)编码的字符串转为目标字符串编码
* \param src [in]源字符串
* \param len [in]源字符串长度
* \param dest [out]目标字符串
* \param to [in]目标字符串的code page
* \return 转换后字符串,同dest
*/
char_string& Basic_WideStringToMultiString(const WCHAR* src, int len, char_string& dest, const char* to);


/*!两种非UTF-16编码间的转换
* \param str [in]源字符串
* \param len [in]源字符串长度
* \param from [in]源字符串的code page
* \param to [in]目标字符串的code page
* \return 转换后的字符串
*/
char_string	Basic_MultiStringToMultiString(const char* str, int len, const char* from, const char* to);

/*!两种非UTF-16编码间的转换
* \param src [in]源字符串
* \param len [in]源字符串长度
* \param dest [out]目标字符串
* \param from [in]源字符串的code page
* \param to [in]目标字符串的code page
* \return  转换后字符串的长度
*/
char_string& Basic_MultiStringToMultiString(const char* src, int len, char_string& dest, const char* from, const char* to);


/*!字符串转化为UTF8
* \param str [in]源字符串。非UTF-16编码
* \param len [in]源字符串长度
* \param from [in]原字符串code page
* \return 转换后的字符串。
*/
char_string	Basic_MultiStringToUTF8(const char* str, int len, const char* from);

/*!字符串转化为UTF8,str为非UNICODE6编码
* \param src [in]源字符串。非UTF-16编码
* \param len [in]源字符串长度
* \param dest [out]目标字符串
* \param from [in]源字符串code page
* \return 转换后的字符串。同dest
*/
char_string& Basic_MultiStringToUTF8(const char* src, int len, char_string& dest, const char* from);

// --------------------8<-----------------简繁体处理的分割线------------------

/*!简体转繁体*/
template<typename CharType>
typename __BasicString<CharType>::StringType Basic_GbToBig5(const CharType* str, int len)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType dest;
	return Basic_GbToBig5(str, len, dest);

}
/*!简体转繁体*/
char_string&	Basic_GbToBig5(const char* src, int len, char_string& dest);
/*!简体转繁体*/
wchar_string&	Basic_GbToBig5(const WCHAR* str, int len, wchar_string& dest);

/*!繁体转简体*/
template<typename CharType>
typename __BasicString<CharType>::StringType Basic_Big5ToGb(const CharType* str, int len)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType dest;
	return Basic_Big5ToGb(str, len, dest);
}
/*!简体转繁体*/
char_string&	Basic_Big5ToGb(const char* str, int len, char_string& dest);
/*!繁体转简体*/
wchar_string&	Basic_Big5ToGb(const WCHAR* str, int len, wchar_string& dest);


/*!判断一个字符串是否是UTF8编码*/
bool			Basic_IsUTF8Str(const char* str, int len);

//! 把一个 TCHAR 字符串转换成 char 字符串，只是在 UNICODE 版本下面才要转换。
class CTStringToMultiString
{
public:
	CTStringToMultiString(LPCTSTR lpszText);
public:
	const char* GetMultiString(long& lTextLength);
protected:
	const char* m_psz;
#if defined(_UNICODE)
	char_string	m_str;
#endif
};

__NS_BASIC_END
#endif 
