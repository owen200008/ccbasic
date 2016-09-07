#include "../../../inc/basic.h"
#ifndef __BASICWINDOWS
#include "../../../../def/util/strutil/charset.h"
#include "../../../../def/util/strutil/charset/charset_def.h"
#include "../../../../def/mem/mem.h"
#include "../../../../def/util/strutil/strutil.h"
#include <iconv.h>

__NS_BASIC_START

const size_t wchar_len = sizeof(WCHAR);
const char* CPS_UNICODE_TYPES[] = {CPS_UTF16LE, CPS_UTF32LE};
const char* CPS_UNICODE_TYPE = CPS_UNICODE_TYPES[wchar_len / 4];	// 除以4来确定下标

wchar_string& Basic_MultiStringToWideString(const char* str, int len, wchar_string& dest, const char* from)
{
	char_string strToAppend(CPS_UNICODE_TYPE);
	strToAppend += ICONV_TARGET_APPEND;
	iconv_t cd = iconv_open(strToAppend.c_str(), from);
	size_t inbytes = len < 0 ? __tcslen(str) : len;
	size_t outbytes = (inbytes + 4) * wchar_len;
	size_t outbytesleft = outbytes;
	dest.resize(outbytes * wchar_len/sizeof(char));
	char* s = (char*)dest.c_str();//outptr;
	size_t conv = iconv(cd, (char**)&str, &inbytes, &s, &outbytesleft);
	iconv_close(cd);
	dest.resize((outbytes-outbytesleft)/wchar_len);
	return dest;
}

char_string& Basic_WideStringToMultiString(const WCHAR* str, int len, char_string& dest, const char* to)
{
	char_string strToAppend = to;
	strToAppend += ICONV_TARGET_APPEND;
	iconv_t cd = iconv_open(strToAppend.c_str(), CPS_UNICODE_TYPE);
	size_t inbytes = len < 0 ? __tcslen(str)  : len ;
	inbytes *= wchar_len;
	size_t outbytes = inbytes + 1;
	size_t outbytesleft = outbytes;
	dest.resize(outbytes);
	char* s = (char*)dest.c_str();
	size_t nconv = iconv(cd, (char**)&str, &inbytes, &s, &outbytesleft);
	iconv_close(cd);
	dest.resize(outbytes - outbytesleft);
	return dest;
}

char_string& Basic_MultiStringToMultiString(const char* str, int len, char_string& dest, const char* from, const char* to)
{
	char_string strToAppend = to;
	strToAppend += ICONV_TARGET_APPEND;
	iconv_t cd = iconv_open(strToAppend.c_str(), from);
	size_t inbytes = len < 0 ? __tcslen(str) : len;
	size_t outbytes = (inbytes+1)*2;
	size_t outbytesleft = outbytes;
	dest.resize(outbytes);
	char* s = (char*)dest.c_str();
	size_t nconv = iconv(cd, (char**)&str, &inbytes, &s, &outbytesleft);
	iconv_close(cd);
	dest.resize(outbytes - outbytesleft);
	return dest;
}

char_string& Basic_MultiStringToUTF8(const char* str, int len, char_string& dest, const char* from)
{
	return Basic_MultiStringToMultiString(str, len, dest, from, CPS_UTF8);
}

/*!字符串转为UNICODE(UTF-16LE/UTF-32LE)编码*/
wchar_string& Basic_MultiStringToWideString(const char* str, int len, wchar_string& dest, unsigned int nCodePage /*= 0*/)
{
	return Basic_MultiStringToWideString(str, len, dest, CPCodeToCPStr(nCodePage));
}
/*!UNICODE(UTF-16)编码的字符串转为目标字符串编码*/
char_string& Basic_WideStringToMultiString(const WCHAR* str, int len, char_string& dest, unsigned int nCodePage /*= 0*/)
{
	return Basic_WideStringToMultiString(str, len, dest, CPCodeToCPStr(nCodePage));
}

/*!两种非UTF-16编码间的转换*/
char_string& Basic_MultiStringToMultiString(const char* str, int len, char_string& dest, unsigned int nCPFrom, unsigned int nCPTo)
{
	return Basic_MultiStringToMultiString(str, len, dest, CPCodeToCPStr(nCPFrom), CPCodeToCPStr(nCPTo));
}

/*!字符串转化为UTF8,str为非UNICODE6编码*/
char_string& Basic_MultiStringToUTF8(const char* str, int len, char_string& dest, unsigned int nCodePage /*= 0*/)
{
	return Basic_MultiStringToUTF8(str, len, dest, CPCodeToCPStr(nCodePage));
}

char_string& Basic_GbToBig5(const char* str, int len, char_string& dest)
{
	return Basic_MultiStringToMultiString(str, len, dest, "gb2312", "big5");
}

wchar_string& Basic_GbToBig5(const WCHAR* str, int len, wchar_string& dest)
{
	char_string msGbk, msBig5;
	Basic_WideStringToMultiString(str, len, msGbk, "gbk");
	Basic_GbToBig5(msGbk.c_str(), msGbk.length(), msBig5);
	return Basic_MultiStringToWideString(msBig5.c_str(), msBig5.length(), dest, "big5");
}

char_string& Basic_Big5ToGb(const char* str, int len, char_string& dest)
{
	return Basic_MultiStringToMultiString(str, len, dest, "big5", "gb2312");
}

wchar_string& Basic_Big5ToGb(const WCHAR* str, int len, wchar_string& dest)
{
	char_string msGbk, msBig5;
	Basic_WideStringToMultiString(str, len, msBig5, "big5");
	Basic_Big5ToGb(msBig5.c_str(), msBig5.length(), msGbk);
	return Basic_MultiStringToWideString(msGbk.c_str(), msGbk.length(), dest, "gbk");
}

__NS_BASIC_END

#endif

