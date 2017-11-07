#include "../../../inc/basic.h"
#ifdef __BASICWINDOWS
#include "charset_win.h"
#include <windows.h>
#include <winnls.h>
#include <assert.h>
__NS_BASIC_START
namespace __private
{

WideBytes::WideBytes(const char* str, int len, unsigned int nCodePage)
{
	size_t multi = len < 0 ? __tcslen(str) : len;
	__length = multi + 1;
	__wchars = (wchar_t*)BasicAllocate(sizeof(wchar_t) * __length);
	__length = ::MultiByteToWideChar(nCodePage, 0, str, multi, __wchars, __length);
	__wchars[__length] = 0;
}

WideBytes::~WideBytes()
{
	if (__wchars)
		BasicDeallocate(__wchars);
}

WideBytes::operator WCHAR* () const
{
	return __wchars;
}

INT WideBytes::Length() const
{
	return __length;
}

WideBytes::operator bool() const
{
	return __wchars != NULL;
}

/////////////////////////////////////////////////////
MultiBytes::MultiBytes(WideBytes& str, unsigned int nCodePage)
{
	size_t multi = str.Length();
	__length = (multi + 1)* 3;
	__chars = (char*)BasicAllocate(sizeof(char) * __length);
	memset(__chars, 0, __length);
	__length = ::WideCharToMultiByte(nCodePage, 0, str, multi, __chars, __length, NULL, NULL);
}

MultiBytes::MultiBytes(const wchar_t* str, int len, unsigned int nCodePage)
{
	size_t multi = len < 0 ? __tcslen(str) : len;
	__length = (multi + 1) * 3;
	__chars = (char*)BasicAllocate(sizeof(char) * __length);
	memset(__chars, 0, __length);
	__length = ::WideCharToMultiByte(nCodePage, 0, str, multi, __chars, __length, NULL, NULL);
	if (0 == __length)
	{
		DWORD dwError = ::GetLastError();
	}
}

MultiBytes::~MultiBytes()
{
	if (__chars)
		BasicDeallocate(__chars);
}

MultiBytes::operator const char* () const
{
	return __chars;
}

int MultiBytes::Length() const
{
	return __length;
}

MultiBytes::operator bool() const
{
	return __chars != NULL;
}

}	// namespace __private

wchar_string& Basic_MultiStringToWideString(const char* str, int len, wchar_string& dest, unsigned int nCodePage /*= BASIC_CP_ACP*/)
{
//	return dest.assign(__private::WideBytes(str, len, nCodePage));
	size_t multi = len < 0 ? __tcslen(str) : len;
	int length = multi + 1;
	dest.resize(length);
	WCHAR* s = (WCHAR*)dest.c_str();
	length = ::MultiByteToWideChar(nCodePage, 0, str, multi, s, length);
	dest.resize(length);
	return dest;
}

char_string& Basic_WideStringToMultiString(const wchar_t* str, int len, char_string& dest, unsigned int nCodePage /*= BASIC_CP_ACP*/)
{
//	return dest.assign(__private::MultiBytes(str, len, nCodePage));
	size_t multi = len < 0 ? __tcslen(str) : len;
	int length = (multi + 1)* 3;
	dest.resize(length);
	char* s  = (char*)dest.c_str();
	length = ::WideCharToMultiByte(nCodePage, 0, str, multi, s, length, NULL, NULL);
	dest.resize(length);
	return dest;
}

char_string& Basic_MultiStringToMultiString(const char* str, int len, char_string& dest, unsigned int nCPFrom, unsigned int nCPTo)
{
	assert(nCPFrom != nCPTo);
//	__private::MultiBytes mb(__private::WideBytes(str, len, nCPFrom), nCPTo);
//	return dest.assign((const char*)mb);
	wchar_string strTemp;
	Basic_MultiStringToWideString(str, len, strTemp, nCPFrom);
	return Basic_WideStringToMultiString(strTemp.c_str(), strTemp.length(), dest, nCPTo);
}

char_string& Basic_MultiStringToUTF8(const char* str, int len, char_string& dest, unsigned int nCPFrom /*=BASIC_CP_ACP*/)
{
	return Basic_MultiStringToMultiString(str, len, dest, nCPFrom, BASIC_CP_UTF8);
}

wchar_string& Basic_MultiStringToWideString(const char* str, int len, wchar_string& dest, const char* from)
{
	return Basic_MultiStringToWideString(str, len, dest, CPStrToCPCode(from));
}

char_string& Basic_WideStringToMultiString(const WCHAR* str, int len, char_string& dest, const char* to)
{
	return Basic_WideStringToMultiString(str, len, dest, CPStrToCPCode(to));
}

char_string& Basic_MultiStringToMultiString(const char* str, int len, char_string& dest, const char* from, const char* to)
{
	return Basic_MultiStringToMultiString(str, len, dest, CPStrToCPCode(from), CPStrToCPCode(to));
}

char_string& Basic_MultiStringToUTF8(const char* str, int len, char_string& dest, const char* from)
{
	return Basic_MultiStringToUTF8(str, len, dest, CPStrToCPCode(from));
}

char_string& Basic_GbToBig5(const char* str, int len, char_string& dest)
{
	int res = LCMapStringA(0x0804, LCMAP_TRADITIONAL_CHINESE, str, len, NULL, 0);
	if (res > 0)
	{
		char_string buf;
		buf.resize(res + 1);
		res = LCMapStringA(0x0804, LCMAP_TRADITIONAL_CHINESE, str, len, (char*)buf.c_str(), res + 1);
		if (res)
		{
			Basic_MultiStringToMultiString(buf.c_str(), res, dest, CP_GBK, CP_BIG5);
		}
	}
	return dest;
}

wchar_string&	Basic_GbToBig5(const WCHAR* str, int len, wchar_string& dest)
{
	int res = LCMapStringW(0x0804, LCMAP_TRADITIONAL_CHINESE, str, len, NULL, 0);
	if (res > 0)
	{
		dest.resize(res + 1);
		res = LCMapStringW(0x0804, LCMAP_TRADITIONAL_CHINESE, str, len, (WCHAR*)dest.c_str(), res + 1);
		if (res)
		{
			dest.resize(res);
		}
		else
		{
			dest.clear();
		}
	}
	return dest;
}


char_string& Basic_Big5ToGb(const char* str, int len, char_string& dest)
{
	char_string buf = Basic_MultiStringToMultiString(str, len, CP_BIG5, CP_GBK);
	int res = LCMapStringA(0x0804, LCMAP_SIMPLIFIED_CHINESE, buf.c_str(), buf.length(), NULL, 0);
	if (res > 0)
	{
		dest.resize(res + 1);
		res = LCMapStringA(0x0804, LCMAP_SIMPLIFIED_CHINESE, buf.c_str(), buf.length(), (char*)dest.c_str(), res + 1);
		if (res > 0)
		{
			dest.resize(res);
		}
		else
		{
			dest.clear();
		}
	}
	return dest;
}

wchar_string&	Basic_Big5ToGb(const WCHAR* str, int len, wchar_string& dest)
{
	int res = LCMapStringW(0x0804, LCMAP_SIMPLIFIED_CHINESE, str, len, NULL, 0);
	if (res > 0)
	{
		dest.resize(res + 1);
		res = LCMapStringW(0x0804, LCMAP_SIMPLIFIED_CHINESE, str, len, (WCHAR*)dest.c_str(), res + 1);
		if (res > 0)
		{
			dest.resize(res);
		}
		else
		{
			dest.clear();
		}
	}
	return dest;
}

__NS_BASIC_END
#endif // __BASICWINDOWS

