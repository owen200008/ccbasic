/***********************************************************************************************
// 文件名:     string.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 9:17:12
// 内容描述:   实现stringt的MFC like接口。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_STRING_H
#define BASIC_STRING_H

//#pragma once

#include <stdio.h>
#include <string>
#include "../util/strutil/strutil.h"

using namespace std;

__NS_BASIC_START
class _BASIC_DLL_API CBasicString : public tstring_s
{
public:
	BASIC_CREATEFUNC(CBasicString)

	CBasicString();
	CBasicString(const char* lpsz);
	CBasicString(const char* lpch, int nLength);
	//! from a single character
	CBasicString(char ch, int nRepeat = 1);

	//! get data length
	int GetLength() const;
	//! TRUE if zero length
	BOOL IsEmpty() const;
	//! clear contents to empty
	void Empty();

	//! return single character at zero-based index
	char GetAt(int nIndex) const;
	//! set a single character at zero-based index
	void SetAt(int nIndex, char ch);

	//! straight character comparison
	int Compare(const char* lpsz) const;
	//! compare ignoring case
	int CompareNoCase(const char* lpsz) const;

	//! return nCount characters starting at zero-based nFirst
	CBasicString Mid(int nFirst, int nCount) const;
	//! return all characters starting at zero-based nFirst
	CBasicString Mid(int nFirst) const;
	//! return first nCount characters in string
	CBasicString Left(int nCount) const;
	//! return nCount characters from end of string
	CBasicString Right(int nCount) const;

	//! NLS aware conversion to uppercase
	void MakeUpper();
	//! NLS aware conversion to lowercase
	void MakeLower();
	//! reverse string right-to-left
	void MakeReverse();

	//! remove whitespace starting from right edge
	void TrimRight();
	//! remove whitespace starting from left side
	void TrimLeft();
	//! remove continuous occurrences of chTarget starting from right
	void TrimRight(char chTarget);
	//! remove continuous occcurrences of characters in passed string,
	//! starting from right
	void TrimRight(const char* lpszTargets);
	//! remove continuous occurrences of chTarget starting from left
	void TrimLeft(char chTarget);
	//! remove continuous occcurrences of characters in
	//! passed string, starting from left
	void TrimLeft(const char* lpszTargets);

	//! replace occurrences of chOld with chNew
	int Replace(char chOld, char chNew);
	//! replace occurrences of substring lpszOld with lpszNew;
	//! empty lpszNew removes instances of lpszOld
	int Replace(const char* lpszOld, const char* lpszNew);

	//! find character starting at left, -1 if not found
	int Find(char ch) const;
	//! find character starting at right
	int ReverseFind(char ch) const;
	//! find character starting at zero-based index and going right
	int Find(char ch, int nStart) const;
	//! find first instance of any character in passed string
	int FindOneOf(const char* lpszCharSet) const;
	//! find first instance of substring
	int Find(const char* lpszSub) const;
	//! find first instance of substring starting at zero-based index
	int Find(const char* lpszSub, int nStart) const;
	//! get pointer to modifiable buffer at least as long as nMinBufLength
	char* GetBuffer(int nMinBufLength);
	//! release buffer, setting length to nNewLength (or to first nul if -1)
	void ReleaseBuffer(int nNewLength = -1);
	//! get pointer to modifiable buffer exaCBasicy as long as nNewLength
	char* GetBufferSetLength(int nNewLength);

	//! format string content
	void Format(const char* lpszFormat, ...);
	//! format string content by arglist
	void FormatV(const char* lpszFormat, va_list argList);

	/*
	全局使用时（多个资源文件）使用lpszModuleName进行区分, 模块名和Basic_LoadStringSource时相同
	*/
	void FormatS(const char* lpszModuleName, const char* lpszFormatID, ...);
	void FormatS(const char* lpszModuleName, DWORD dwFormatID, ...);

	/*
	全局使用时（多个资源文件）使用lpszModuleName进行区分, 模块名和Basic_LoadStringSource时相同
	*/
	BOOL LoadString(const char* lpszFormatID, const char* lpszModuleName = "");
	BOOL LoadString(DWORD dwFormatID, const char* lpszModuleName = "");
};


inline CBasicString	operator+(const CBasicString& lhs, const CBasicString& rhs)
{
	CBasicString str = lhs;
	str += rhs;
	return str;
}

inline CBasicString	operator+(const CBasicString& lhs, const char* rhs)
{
	CBasicString str = lhs;
	str += rhs;
	return str;
}

inline CBasicString	operator+(const char* lhs, const CBasicString& rhs)
{
	CBasicString str = lhs;
	str += rhs;
	return str;
}

inline bool	operator == (const CBasicString& lhs, const CBasicString& rhs)
{
	return lhs.Compare(rhs.c_str()) == 0;
}

inline bool	operator == (const CBasicString& lhs, const char* rhs)
{
	return lhs.Compare(rhs) == 0;
}


inline bool	operator == (const char* lhs, const CBasicString& rhs)
{
	return rhs.Compare(lhs) == 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class _BASIC_DLL_API CWBasicString : public tstring
{
public:
	// Constructors

	//! constructs empty CWBasicString
	CWBasicString();
	//! copy constructor
	CWBasicString(const CWBasicString& stringSrc);
	//! from a single character
	CWBasicString(TCHAR ch, int nRepeat = 1);
	//! from an ANSI string (converts to TCHAR)
	CWBasicString(LPCSTR lpsz);
	//! from a UNICODE string (converts to TCHAR)
	CWBasicString(LPCWSTR lpsz);
	//! subset of characters from an ANSI string (converts to TCHAR)
	CWBasicString(LPCSTR lpch, int nLength);
	// subset of characters from a UNICODE string (converts to TCHAR)
	CWBasicString(LPCWSTR lpch, int nLength);
	//! from unsigned characters, not support
	//	CWBasicString(const unsigned char* psz);
	//! from tstring
	CWBasicString(const tstring& stringSrc);


	// Attributes & Operations

	//! get data length
	int GetLength() const;
	//! TRUE if zero length
	BOOL IsEmpty() const;
	//! clear contents to empty
	void Empty();

	//! return single character at zero-based index
	TCHAR GetAt(int nIndex) const;
	//! set a single character at zero-based index
	void SetAt(int nIndex, TCHAR ch);
	//! return pointer to const string
	operator LPCTSTR() const;
	// string comparison

	//! straight character comparison
	int Compare(LPCTSTR lpsz) const;
	//! compare ignoring case
	int CompareNoCase(LPCTSTR lpsz) const;
	//! NLS aware comparison, case sensitive
	int Collate(LPCTSTR lpsz) const;
	//! NLS aware comparison, case insensitive
	int CollateNoCase(LPCTSTR lpsz) const;

	// simple sub-string extraction

	//! return nCount characters starting at zero-based nFirst
	CWBasicString Mid(int nFirst, int nCount) const;
	//! return all characters starting at zero-based nFirst
	CWBasicString Mid(int nFirst) const;
	//! return first nCount characters in string
	CWBasicString Left(int nCount) const;
	//! return nCount characters from end of string
	CWBasicString Right(int nCount) const;

	//!  characters from beginning that are also in passed string
	CWBasicString SpanIncluding(LPCTSTR lpszCharSet) const;
	//! characters from beginning that are not also in passed string
	CWBasicString SpanExcluding(LPCTSTR lpszCharSet) const;

	// upper/lower/reverse conversion

	//! NLS aware conversion to uppercase
	void MakeUpper();
	//! NLS aware conversion to lowercase
	void MakeLower();
	//! reverse string right-to-left
	void MakeReverse();

	// trimming whitespace (either side)

	//! remove whitespace starting from right edge
	void TrimRight();
	//! remove whitespace starting from left side
	void TrimLeft();

	// trimming anything (either side)

	//! remove continuous occurrences of chTarget starting from right
	void TrimRight(TCHAR chTarget);
	//! remove continuous occcurrences of characters in passed string,
	//! starting from right
	void TrimRight(LPCTSTR lpszTargets);
	//! remove continuous occurrences of chTarget starting from left
	void TrimLeft(TCHAR chTarget);
	//! remove continuous occcurrences of characters in
	//! passed string, starting from left
	void TrimLeft(LPCTSTR lpszTargets);

	// advanced manipulation

	//! replace occurrences of chOld with chNew
	int Replace(TCHAR chOld, TCHAR chNew);
	//! replace occurrences of substring lpszOld with lpszNew;
	//! empty lpszNew removes instances of lpszOld
	int Replace(LPCTSTR lpszOld, LPCTSTR lpszNew);
	//! remove occurrences of chRemove
	int Remove(TCHAR chRemove);
	//! insert character at zero-based index; concatenates
	//! if index is past end of string
	int Insert(int nIndex, TCHAR ch);
	//! insert substring at zero-based index; concatenates
	//! if index is past end of string
	int Insert(int nIndex, LPCTSTR pstr);
	//! delete nCount characters starting at zero-based index
	int Delete(int nIndex, int nCount = 1);

	// searching

	//! find character starting at left, -1 if not found
	int Find(TCHAR ch) const;
	//! find character starting at right
	int ReverseFind(TCHAR ch) const;
	//! find character starting at zero-based index and going right
	int Find(TCHAR ch, int nStart) const;
	//! find first instance of any character in passed string
	int FindOneOf(LPCTSTR lpszCharSet) const;
	//! find first instance of substring
	int Find(LPCTSTR lpszSub) const;
	//! find first instance of substring starting at zero-based index
	int Find(LPCTSTR lpszSub, int nStart) const;
	//! get pointer to modifiable buffer at least as long as nMinBufLength
	LPTSTR GetBuffer(int nMinBufLength);
	//! release buffer, setting length to nNewLength (or to first nul if -1)
	void ReleaseBuffer(int nNewLength = -1);
	//! get pointer to modifiable buffer exaCBasicy as long as nNewLength
	LPTSTR GetBufferSetLength(int nNewLength);

	//! format string content
	void Format(LPCTSTR lpszFormat, ...);
	//! format string content by arglist
	void FormatV(LPCTSTR lpszFormat, va_list argList);

	/*
	全局使用时（多个资源文件）使用lpszModuleName进行区分, 模块名和Basic_LoadStringSource时相同
	*/
	void FormatS(LPCTSTR lpszModuleName, LPCTSTR lpszFormatID, ...);
	void FormatS(LPCTSTR lpszModuleName, DWORD dwFormatID, ...);

	/*
	全局使用时（多个资源文件）使用lpszModuleName进行区分, 模块名和Basic_LoadStringSource时相同
	*/
	BOOL LoadString(LPCTSTR lpszFormatID, LPCTSTR lpszModuleName = _T(""));
	BOOL LoadString(DWORD dwFormatID, LPCTSTR lpszModuleName = _T(""));
};


inline CWBasicString	operator+(const CWBasicString& lhs, const CWBasicString& rhs)
{
	CWBasicString str = lhs;
	str += rhs;
	return str;
}

inline CWBasicString	operator+(const CWBasicString& lhs, LPCTSTR rhs)
{
	CWBasicString str = rhs;
	str = lhs + str;
	return str;
}

inline CWBasicString	operator+(LPCTSTR lhs, const CWBasicString& rhs)
{
	CWBasicString str = lhs;
	str += rhs;
	return str;
}

inline bool	operator == (const CWBasicString& lhs, const CWBasicString& rhs)
{
	return lhs.Compare(rhs) == 0;
}

inline bool	operator == (const CWBasicString& lhs, LPCTSTR rhs)
{
	return lhs.Compare(rhs) == 0;
}


inline bool	operator == (LPCTSTR lhs, const CWBasicString& rhs)
{
	return rhs.Compare(lhs) == 0;
}

__NS_BASIC_END

namespace std{
	template<>
	struct hash<basiclib::CBasicString> : public std::unary_function<basiclib::CBasicString, std::size_t>{
		std::size_t operator()(const basiclib::CBasicString &key) const{
#ifdef __BASICWINDOWS
			return _Hash_seq((const unsigned char*)key.c_str(), key.length());
#else
			hash<const char*> hash_fn;
			return hash_fn(key.c_str());
#endif	
		}
	};
	template <>
	struct equal_to<basiclib::CBasicString> : public binary_function<basiclib::CBasicString, basiclib::CBasicString, bool>{
		bool operator()(const basiclib::CBasicString& __x, const basiclib::CBasicString& __y) const{
			return __x == __y;
		}
	};
}

#endif 

