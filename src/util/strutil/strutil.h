/***********************************************************************************************
// 文件名:     strutil.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:17:20
// 内容描述:   一些常用的字符串处理函数，包括：
拆分字符串、合并字符串、string字符串类的一些常用操作。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_STRUTIL_H
#define BASIC_STRUTIL_H


#include <string>
#include <vector>
#include <sstream>
#include "../../inc/basic_def.h"
#include "../private.h"
#include "../../types/types.h"
#include "../../mem/mem.h"
#include "../../mem/basicallocator.h"
#include "../container.h"
#include "cppstring.h"
#include "../../debug/debug.h"

#ifdef __BASICWINDOWS
#include <shlwapi.h>
#endif

#ifdef  _MSC_VER
#pragma warning(disable: 4793)
#endif  /* _MSC_VER */

#define STRING_ALLOCATOR	DEFAULT_ALLOCATOR

using namespace std;

__NS_BASIC_START

const size_t NullLen = (size_t)-1;
const char NullAString[] = "";
const WCHAR NullWString[] = { 0 };
const LPCTSTR Null_String = _T("");
#define Null_String_S		""

//! 字符串、字符串流的定义。
//! 目的：可以全局统一替换分配器。

template<typename CharType>
struct __BasicString
{
	typedef typename basic_basic_string<CharType>::type		StringType;
	typedef typename basic_basic_stringstream<CharType>::type	StringStreamType;
};
 
#pragma warning (push)
#pragma warning (disable: 4251)

template struct _BASIC_DLL_API __BasicString<char>;
class _BASIC_DLL_API CharBasiCWBasicString : public basic_string<char, std::char_traits<char>, DEFAULT_ALLOCATOR<char> >
{
public:
	 CharBasiCWBasicString(){}
};

template struct _BASIC_DLL_API __BasicString<wchar_t>;
class _BASIC_DLL_API WCharrBasiCWBasicString : public basic_string<wchar_t, std::char_traits<wchar_t>, DEFAULT_ALLOCATOR<wchar_t> >
{
public:
	WCharrBasiCWBasicString(){}
};

#pragma warning (pop)

typedef __BasicString<TCHAR>::StringType			tstring;
typedef __BasicString<TCHAR>::StringStreamType		tstringstream;

typedef __BasicString<char>::StringType				tstring_s;
typedef __BasicString<char>::StringStreamType		tsstringstream_s;



namespace __private
{
	MarkString(tstring);
	MarkString(tstring&);
	MarkString(const tstring);
	MarkString(const tstring&);
}

typedef	tstring_s									char_string;
typedef tstring										wchar_string;

// 由字符类型取得对应的usigned的类型
template<typename CharType>
struct __CharValue
{
	typedef CharType Result;
};

template<>
struct __CharValue<char>
{
	typedef unsigned char Result;
};

// 由unsigned的类型取得对应的字符类型
template<typename ValueType>
struct __ValueChar
{
	typedef ValueType Result;
};


template<>
struct __ValueChar<unsigned char>
{
	typedef char Result;
};
__NS_BASIC_END

#ifdef __GNUC__
#include <ext/hash_map>

namespace __gnu_cxx
{
	using namespace ::basiclib;
    template<> struct hash<long long>
    {
        size_t operator()(const long long __x) const
        {
            return __x;
        }
    };
	template<> struct hash<__BasicString<char>::StringType>
	{
		typedef __BasicString<char>::StringType StringType;
		size_t operator()(const StringType& __s) const
		{
            hash<const char*> fn;
            return fn(__s.c_str());
        }
	};

	template<> struct hash<__BasicString<WCHAR>::StringType>
	{
		typedef __BasicString<WCHAR>::StringType StringType;
		size_t operator()(const StringType& __s) const
		{
            ASSERT(0);
            return 0;
        }
	};
}
#elif defined(__SGI_SBASIC_PORT)
#include <hash_map>
namespace std
{
	using namespace ::basiclib;
	template<> struct hash<__BasicString<char>::StringType>
	{
		typedef __BasicString<char>::StringType StringType;
		size_t operator()(const StringType& __s) const
		{ return __stl_hash_string(__s.c_str()); }
	};

	template<> struct hash<__BasicString<WCHAR>::StringType>
	{
		typedef __BasicString<char>::StringType StringType;
		size_t operator()(const StringType& __s) const
		{ return __stl_hash_string(__s.c_str()); }
	};
}
#else
namespace stdext
{
	template <class _InIt>
		inline size_t VS2008_Hash_value(_InIt _Begin, _InIt _End)
	{	// hash range of elements
		size_t _Val = 2166136261U;
		while(_Begin != _End)
			_Val = 16777619U * _Val ^ (size_t)*_Begin++;
		return (_Val);
	}
	inline
		size_t hash_value(const basiclib::char_string& str)
	{	// hash _Keyval to size_t value one-to-one
		return (VS2008_Hash_value(str.begin(), str.end())); 
	}

	inline
	size_t hash_value(const basiclib::wchar_string& str)
	{	// hash _Keyval to size_t value one-to-one
		return (VS2008_Hash_value(str.begin(), str.end())); 
	}
}
#endif

__NS_BASIC_START

//! 判断字符串是否为空
/*!
\param p 判断字符串是否为空
*/
template<typename CharType>
bool IsStringEmpty(const CharType* p)
{
	return (NULL == p) || ((CharType)0 == p[0]);
}

//! 对字符串按照指定的分隔符分割，并依次交由func函数处理
/*!
\param p 处理的字符串。函数会修改里面的内容。不能为空
\param cTok 分割符
\param func 用于根据参数取得值的仿函数。
\remarks 该函数会修改p所指向的字符串内容
*/
template<class Functor, typename CharType>
void __SpliteString(CharType* p, CharType cTok, Functor func)
{
	CharType* e = p;
	do
	{
		e = __tcschr(p, cTok);
		if (e != NULL)
			*e++ = 0;
		func(p);
		p = e;
	}while(p != NULL);
}
//! 对字符串按照指定的分隔字符串分割，并依次交由func函数处理
/*!
\param p 源字符串。函数会修改里面的内容。不能为空。
\param pszTok 分割用字符串
\param func 用于根据参数取得值的仿函数。
\remarks 该函数会修改p所指向的字符串内容
*/
template<class Functor, typename CharType>
void __SpliteString(CharType* p, const CharType* pszTok, Functor func)
{
	int len = __tcslen(pszTok);
	CharType* e = p;
	do
	{
		e = __tcsstr(p, pszTok);
		if (e != NULL)
		{
			*e = 0;
			e += len;
		}
		func(p);
		p = e;
	}while(p != NULL);
}

//! 对字符串按照指定的分隔符分割，并依次交由func函数处理
/*!
\param psz 源字符串,不能为空
\param cTok 分割符号
\param func 用于根据参数取得值的仿函数。
\remarks 和__SpliteString的区别是，该函数不修改psz指向内容。
示例可以参考仿函数IntoContainer
*/
template<class Functor, typename CharType>
void BasicSpliteString(const CharType* psz, CharType cTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	__SpliteString((CharType*)str.c_str(), cTok, func);
}

//! 对字符串按照指定的分隔符分割，并依次交由func函数处理
/*!
\param psz 源字符串，不能为空。
\param pszTok 分隔字符串
\param func 用于根据参数取得值的仿函数。
\remarks 和__SpliteString的区别是，该函数不修改psz指向内容。
示例可以参考仿函数IntoContainer
*/
template<class Functor, typename CharType>
void BasicSpliteString(const CharType* psz, const CharType* pszTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	__SpliteString((CharType*)str.c_str(), pszTok, func);
}

//! 对字符串按照指定的分隔符分割，并依次交由func函数处理
/*!
\param psz 源字符串
\
\param pszTok 分隔字符串,不能为空
\param func 用于根据参数取得值的仿函数。
\remarks 和__SpliteString的区别是，该函数不修改psz指向内容。
示例可以参考仿函数IntoContainer
*/
template<class Functor, typename CharType>
void BasicSpliteString(const CharType* psz, long length, CharType cTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz, length);
	__SpliteString((CharType*)str.c_str(), cTok, func);
}

//! 对字符串按照指定的分隔符分割，并依次交由func函数处理
/*!
\param psz 源字符串，不能为空
\param length 源字符串的长度
\param pszTok 分隔字符串
\param func 用于根据参数取得值的仿函数。
\remarks 和__SpliteString的区别是，该函数不修改psz指向内容。
示例可以参考仿函数IntoContainer
*/
template<class Functor, typename CharType>
void BasicSpliteString(const CharType* psz, long length, const CharType* pszTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz, length);
	__SpliteString((CharType*)str.c_str(), pszTok, func);
}


//! 对字符串按照指定的分隔符分割，并依次交由func函数处理
/*!
\param psz 源字符串，不能为空
\param pszTok 在pszTok中的字符都会被当作分隔符使用
\param func 用于根据参数取得值的仿函数。
\remark 该函数会修改p所指向的字符串内容
示例可以参考仿函数IntoContainer
*/
template<class Functor, typename CharType>
void __Explode(CharType* psz, const CharType* pszTok, Functor func)
{
	CharType* e = psz;
	while(0 != *e)
	{
		// 找到分隔符
		if (NULL != __tcschr(pszTok, *e))
		{
			*e = (CharType)0;
			func(psz);
			psz = e + 1;
		}
		++e;
	}

	if (*psz != 0)
	{
		func(psz);
	}
}
/*! 对字符串按照指定的分隔符表进行分割，并依次交由func函数处理
\param psz 源字符串，不能为空
\param pszTok 在pszTok中的字符都会被当作分隔符使用
\param func 用于根据参数取得值的仿函数。
\remark 和__Explode的区别是该函数不修改psz指向内容。
*/
template<class Functor, typename CharType>
void Basic_Explode(const CharType* psz, const CharType* pszTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	__Explode((CharType*)str.c_str(), pszTok, func);
}

/*! 对字符串按照指定的分隔符表进行分割，并依次交由func函数处理
\param psz 源字符串，不能为空
\param length psz字符串的长度
\param pszTok 在pszTok中的字符都会被当作分隔符使用
\param func 用于根据参数取得值的仿函数。
\remark 和__Explode的区别是该函数不修改psz指向内容。
*/
template<class Functor, typename CharType>
void Basic_Explode(const CharType* psz, long length, const CharType* pszTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz, length);
	__Explode((CharType*)str.c_str(), pszTok, func);
}




//! 对字符串按照指定的分隔符分割，并依次交由func函数处理，直到字符串尾部或者func返回true
/*!
\param psz 源字符串，不能为空。
\param cTok 分隔符
\param func 用于根据参数取得值的仿函数。当该函数返回true时，函数退出。
\remarks 该函数会修改p所指向的内容。 
*/
template<class Functor, typename CharType>
bool __SpliteStringBreak(CharType* p, CharType cTok, Functor func)
{
	CharType* e = p;
	do
	{
		e = __tcschr(p, cTok);
		if (e != NULL)
			*e++ = 0;
		if (func(p, e ? e - p - 1 : __tcslen(p)))
			return true;
		p = e;
	}while(p != NULL);
	return false;
}

//! 对字符串按照指定的分隔符分割，并依次交由func函数处理，直到字符串尾部或者func返回true
/*!
\param psz 源字符串,不能为空。
\param cTok 分隔符
\param func 用于根据参数取得值的仿函数。当该函数返回true时，函数退出。
\remarks 和__SpliteStringBreak的区别是，该函数不修改psz指向内容。
*/
template<class Functor, typename CharType>
bool BasicSpliteStringBreak(const CharType* psz, CharType cTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	return __SpliteStringBreak((CharType*)str.c_str(), cTok, func);
}

//! 对字符串按照指定的分隔符分割，并依次交由func函数处理。对于'\'开头的字符串将做转义。
/*!
\param p 源字符串,不能为空。
\param func 用于根据参数取得值的仿函数
\remarks 该函数会修改p所指向的源字符串。
*/
template<class Functor, typename CharType>
bool __SpliteStringBreakWithEscape(CharType* p, CharType cTok, Functor func)
{
	CharType* e = p;
	CharType* v = p;
	while(0 != *e)
	{
		if ((CharType)'\\' == *e &&  0 != *(e+1))	// 转义符号,略过下一个字符
		{
			*v++ = *++e;
			++ e;
		}
		else if (*e == cTok)
		{
			*e = 0;
			*v = 0;

			if (func(p))
				return true;
			p = ++ e;
			v = p;
		}
		else
		{
			*v++ = *e++;
		}
	}
	
	if (*p != 0)
	{
		*v = 0;
		return func(p);
	}
	
	return false;
}

//! 对字符串按照指定的分隔符分割，并依次交由func函数处理。支持'\'转义
/*!
\param psz 源字符串,不能为空
\param cTok 分隔符
\param func 用于根据参数取得值的仿函数
\remarks 和__SpliteStringBreakWithEscape的区别是SpliteStringBreakWithEscape不修改psz的内容。
\code
char* buf = "a\\;;b;\\\\\\;c";	//字符串 a\;;b;\\\;cd
BasicSpliteStringBreakWithEscape(buf, ';', functor);

那么functor收到的字符串应该是：
a;
b
\;c
\endcode
*/
template<class Functor, typename CharType>
bool BasicSpliteStringBreakWithEscape(const CharType* psz, CharType cTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	return __SpliteStringBreakWithEscape((CharType*)str.c_str(), cTok, func);
}

#define ISSPACE(a) (((a) >=0x09 && (a) <= 0x0D) || (a)== 0x20)
#define FORWARD_SPACE(a, e, l)	while(ISSPACE(*(a)) && ((e)+l == find((e), (e)+l, (*(a))))) {++a;}
#define BACKWARD_SPACE(a)	while(ISSPACE(*(a))) {--a;}


const int PPS_OPS_LOWERKEY		= 1;	// 将第一个字符串转成小写
const int PPS_OPS_LOWERVALUE	= 2;	// 将第二个字符串转成小写
const int PPS_OPS_SKIPBLANK		= 4;	// 略过字符串前后的空格/tab/换行

namespace __private
{
// 以下函数均为ParseParamString使用
template<class CharType, class T>
void SkipBlank(CharType*& p, const CharType*& pException, int len, Type2Type<T>)
{
	FORWARD_SPACE(p, pException, len);
};

template<class CharType>
void SkipBlank(CharType*& p, const CharType*& pException, int len, Type2Type<False>)
{}

template<class CharType, class T>
void Lower(CharType* p, Type2Type<T>)
{
	*p = (CharType)tolower(*p);
};


template<class CharType>
void Lower(CharType* p, Type2Type<False>)
{}

template<class CharType, class T>
void SetBlank(CharType* p, CharType*& blank, Type2Type<T>)
{
	if (ISSPACE(*p))
	{
		if (NULL == blank)
			blank = p;
	}
	else if (blank)
	{
		blank = NULL;
	}
}

template<class CharType>
void SetBlank(CharType* p, CharType*& blank, Type2Type<False>)
{
}

template<class CharType, class T>
void SetEnd(CharType* blank, Type2Type<T>)
{
	if (blank)
	{
		*blank = (CharType)0;
		blank = NULL;
	}
}

template<class CharType>
void SetEnd(CharType* blank, Type2Type<False>)
{
}

template<class LK, class LV, class SB>
struct Parse_Trait
{
	typedef Type2Type<LK>	LowerKey;
	typedef Type2Type<LV>	LowerValue;
	typedef Type2Type<SB>	SkipBlank;
};
}

//! 解析形似与"a=b&c=d&e=f"的字符串，并将(a,b),(c,d),(e,f)依次处理
/*!
\param psz 源字符
\param tok[0]分别标识配对分隔符。tok[1]标识条目分隔符
\param func 字符串对进行处理的函数或者仿函数
\param ParseTraits	对具体一些特性的萃取
\return 无返回值
*/
template<class Functor, typename CharType, typename ParseTraits>
void __ParseParamString_Aux(CharType* psz, const CharType tok[], Functor func, ParseTraits)
{
	
	enum {
		PARSE_KEY,
		PARSE_VALUE
	} parseStatus;
	parseStatus = PARSE_KEY;
	__private::SkipBlank(psz, tok, 2, typename ParseTraits::SkipBlank());

	CharType* p = psz;
	CharType* pKey = psz;
	CharType* pValue = NULL;
	CharType* blank = NULL;
	while(*p != 0)
	{
		switch(parseStatus)
		{
		case PARSE_KEY:
			if (tok[1] == *p)
			{ // reach end before get value
				*p = 0;
				__private::SetEnd(blank, typename ParseTraits::SkipBlank());
				func(pKey, pValue);
				++ p;
				__private::SkipBlank(p, tok, 2, typename ParseTraits::SkipBlank());
				pKey = p;
			}
			else if (tok[0] == *p)
			{	// 第一个分割符
				*p = 0;
				pValue = p + 1;
				__private::SetEnd(blank, typename ParseTraits::SkipBlank());
				__private::SkipBlank(pValue, tok, 2, typename ParseTraits::SkipBlank());
				p = pValue;
				parseStatus = PARSE_VALUE;
			}
			else
			{
				__private::SetBlank(p, blank, typename ParseTraits::SkipBlank());
				__private::Lower(p++, typename ParseTraits::LowerKey());
			}
			break;
		case PARSE_VALUE:
			if (tok[1] == *p)
			{	// 到了尾部了
				*p = 0;
				__private::SetEnd(blank, typename ParseTraits::SkipBlank());
				func(pKey, pValue);
				++ p;
				__private::SkipBlank(p, tok, 2, typename ParseTraits::SkipBlank());
				pKey = p;

				pValue = NULL;
				parseStatus = PARSE_KEY;
			}
			else
			{
				__private::SetBlank(p, blank, typename ParseTraits::SkipBlank());			
				__private::Lower(p++, typename ParseTraits::LowerValue());
			}
			break;
		}
	}
	if (*pKey)
	{
		__private::SetEnd(blank, typename ParseTraits::SkipBlank());
		func(pKey, pValue);
	}
}


class break_exception : public exception
{
public:
	virtual const char* what() const throw()
	{
		return "callback functor break circle";
	}
};

//! 解析形似与"a=b&c=d&e=f"的字符串，并将(a,b),(c,d),(e,f)依次处理
/*!
\param psz 源字符,不能为空
\param psz 字符串的长度
\param tok[0]分别标识配对分隔符。tok[1]标识条目分隔符
\param func 字符串对进行处理的函数或者仿函数
\param Ops 参数选项。值为PPS_OPS_*
\return 无返回值
*/

template<class Functor, typename CharType>
void Basic_ParseParamString(const CharType* psz, size_t len, const CharType tok[], Functor func, int nOps = PPS_OPS_LOWERKEY|PPS_OPS_SKIPBLANK)
{
	using namespace __private;
	typename __BasicString<CharType>::StringType s(psz, len);

	switch(nOps)
	{
	case 0:
		__ParseParamString_Aux((CharType*)s.c_str(), tok , func, Parse_Trait<False, False, False>());
		break;
	case PPS_OPS_LOWERKEY:		// 1
		__ParseParamString_Aux((CharType*)s.c_str(), tok , func, Parse_Trait<True, False, False>());
		break;
	case PPS_OPS_LOWERVALUE:	// 2
		__ParseParamString_Aux((CharType*)s.c_str(), tok , func, Parse_Trait<False, True, False>());
		break;
	case PPS_OPS_LOWERKEY|PPS_OPS_LOWERVALUE:	// 3
		__ParseParamString_Aux((CharType*)s.c_str(), tok , func, Parse_Trait<True, True, False>());
		break;
	case PPS_OPS_SKIPBLANK:		// 4
		__ParseParamString_Aux((CharType*)s.c_str(), tok , func, Parse_Trait<False, False, True>());
		break;
	case PPS_OPS_LOWERKEY|PPS_OPS_SKIPBLANK:	// 5
		__ParseParamString_Aux((CharType*)s.c_str(), tok , func, Parse_Trait<True, False, True>());
		break;
	case PPS_OPS_LOWERVALUE|PPS_OPS_SKIPBLANK:	// 6
		__ParseParamString_Aux((CharType*)s.c_str(), tok , func, Parse_Trait<False, True, True>());
		break;
	case PPS_OPS_LOWERKEY|PPS_OPS_LOWERVALUE|PPS_OPS_SKIPBLANK:	// 7
		__ParseParamString_Aux((CharType*)s.c_str(), tok , func, Parse_Trait<True, True, True>());
		break;
	}
}

//! 解析形似与"a=b&c=d&e=f"的字符串，并将(a,b),(c,d),(e,f)依次处理
/*!
\param psz 源字符,不能为空。必须以'\0'结尾
\param tok[0]分别标识配对分隔符。tok[1]标识条目分隔符
\param func 字符串对进行处理的函数或者仿函数
\param Ops 参数选项。值为PPS_OPS_*
\return 无返回值
*/
template<class Functor, typename CharType>
void Basic_ParseParamString(const CharType* psz, const CharType tok[], Functor func, int nOps = PPS_OPS_LOWERKEY|PPS_OPS_SKIPBLANK)
{
	Basic_ParseParamString(psz, __tcslen(psz), tok, func, nOps);
}

//! 填充字符串中tok指定的内的参数。参数的值由func来取得。
/*!
\param str 源字符串，不能为空。
\param tok 用于指定起始。例如"<>"
\param func 用于根据参数取得值的仿函数
\remarks str内的内容会被修改。
*/
template<class Functor, typename CharType>
typename __BasicString<CharType>::StringType __FillParamString(CharType* str, const CharType tok[], Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType	result;
	CharType *left = NULL;
	CharType *right = NULL;
	do 
	{
		left = __tcschr(str, tok[0]);
		if (left)
		{
			++ left;
			right = __tcschr(left, tok[1]);
			if (right)
			{
				*right++ = '\0';
				result.append(str, left - str - 1);
				result.append(func(left));

				str = right;
			}
		}
	} while(left && right);

	result += str;

	return result;
}


//! 根据format字符串，取出源字符串中的值
/*!
\param pszSrc 源字符,不能为空
\param tok[0]分别标识配对分隔符。tok[1]标识条目分隔符
\param func 字符串对进行处理的函数或者仿函数
\return 无返回值
\remarks str内的内容会被修改。
*/
template <class CharType, class Functor>
void __GetParamString(CharType* lpszSrc, CharType* lpszFormat, const CharType tok[], Functor func)
{
	ASSERT(!IsStringEmpty(lpszFormat) && !IsStringEmpty(lpszSrc));

	CharType *pEnd, *pKey, *pSep;
	pEnd = pKey = pSep = NULL;
	size_t len = 0;
	while(lpszFormat && lpszSrc && 
		*lpszFormat != _T('\0') && *lpszSrc != _T('\0'))
	{
		if (*lpszFormat == *lpszSrc)
		{
			++ lpszFormat;
			++ lpszSrc;			
		}
		else if (*lpszFormat == tok[0])
		{
			pKey = lpszFormat + 1;
			pEnd = __tcschr(pKey,  tok[1]);
			if (pEnd == NULL)
				break;
			*pEnd++ = (CharType)0;
			lpszFormat = pEnd;
			// FindNext
			pEnd = __tcschr(lpszFormat, tok[0]);
			if (pEnd)
			{
				len = pEnd - lpszFormat;
				*pEnd = (CharType)0;
			}
			else
			{
				len = __tcslen(lpszFormat);
			}

			if (len == 0)
			{
				func(pKey, lpszSrc);
				break;
			}
			else
			{
				pSep = __tcsstr(lpszSrc, lpszFormat);
				if (NULL == pSep)
					break;
				*pSep = (CharType)0;
				func(pKey, lpszSrc);
				lpszSrc = pSep + len;

				if (pEnd)
				{
					*pEnd = tok[0];
				}
				lpszFormat = pEnd;
			}
		}
		else
		{
			break;
		}
	}
}

//! 根据format字符串，取出源字符串中的值
/*!
\param pszSrc 源字符,不能为空
\param tok[0]分别标识配对分隔符。tok[1]标识条目分隔符
\param func 字符串对进行处理的函数或者仿函数
\return 无返回值
\code
Basic_GetParamString("/shase/600000.txt", "/{market}/{code}.txt", "{}", InfoMapContainer<tstring, tstring>());
\endcode
输出:
market=>shase
code=>600000
*/
template <class CharType, class Functor>
void Basic_GetParamString(const CharType* lpszSrc, const CharType* lpszFormat, const CharType tok[], Functor func)
{
	using namespace __private;
	typename __BasicString<CharType>::StringType format(lpszFormat);
	typename __BasicString<CharType>::StringType src(lpszSrc);

	__GetParamString((CharType*)src.c_str(),(CharType*)format.c_str(), tok, func);
}


//! 填充字符串中tok[0]和tok[1]内的参数。参数的值由func来取得。
/*!
\param psz 源字符串，不能为空
\param tok 标识关键字区间的字符，至少两个字符，分别用于标识起始和结束
\param func 用于根据参数取得值的仿函数
\remarks 和__FillParamString的区别是Basic_FillParamString不修改psz的内容。
*/
template<class Functor, typename CharType>
typename __BasicString<CharType>::StringType	Basic_FillParamString(const CharType* psz, const CharType tok[], Functor func)
{
	typename __BasicString<CharType>::StringType	str(psz);
	return __FillParamString((CharType*)str.c_str(), tok, func);
}

namespace __private
{
	//! 合并数据对象到字符串的处理仿函数
	/*!
	\struct __combine_string_helper
	*/
	template<typename CharType>
	struct __combine_string_helper
	{
		typedef typename __BasicString<CharType>::StringType	StringType;
		typedef typename __BasicString<CharType>::StringStreamType StringStreamType;

		__combine_string_helper(CharType tok, StringType& str)
			: __tok(tok), __str(str){}

		template<class T>
		__combine_string_helper& operator()(const T& s)
		{
			if (__tok != 0 && !__str.empty())
				__str += __tok;

			typedef typename IsString<T>::Result Result;
			AddString(s, Result());
			return *this;
		}

		template<class T, class U>
		void AddString(const T& s, U)
		{
			__str += s;
		}

		template<class T>
		void AddString(const T& s, struct False)
		{
			StringStreamType stream;
			stream << s;
			__str += stream.str();
		}

		CharType	__tok;
		StringType&	__str;
	};

	//! 合并数据对象到字符串流的处理仿函数
	/*!
	\struct __combine_stream_helper
	*/
	template<typename CharType>
	struct __combine_stream_helper
	{
		typedef typename __BasicString<CharType>::StringStreamType stream_type;
		__combine_stream_helper(CharType tok, stream_type& stream)
			: __tok(tok), __stream(stream), __first(true){}

		template<class T>
		__combine_stream_helper& operator()(const T& s)
		{
			if (__tok != 0 && !__first)
				__stream << __tok;
			else if (__first)
				__first = false;
			
			__stream << s;			
			return *this;
		}

		__combine_stream_helper& operator()(const unsigned char& s)
		{
			return operator()((const unsigned int)s);
		}

		__combine_stream_helper& operator()(unsigned char& s)
		{
			return operator()((unsigned int)s);
		}

		CharType		__tok;
		stream_type&	__stream;
		bool			__first;
	};
}


namespace __private
{
	const char*		__get_key_string(__private::Type2Type<char>);
	const char*		__get_esc_string(__private::Type2Type<char>);

	const WCHAR*	__get_key_string(__private::Type2Type<WCHAR>);
	const WCHAR*	__get_esc_string(__private::Type2Type<WCHAR>);

	const char*		__get_crlf_string(__private::Type2Type<char>);
	const WCHAR*	__get_crlf_string(__private::Type2Type<WCHAR>);
}


//! 合并容器内的字符串，之间用分隔符分割
/*!
\param tok 分隔符
\param first input iterator，用于标识起始元素
\param last input iterator，用于标识结束元素
如果分割符值为'\0',则字符串直接合并。
\code
char* buf[] = {"aa", "bb", "cc"};
string str = ComineString(',', buf, sizeof(buf)/sizeof(char*));
\endcode
这时str的值是"aa,bb,cc"
另外也可以对非字符串数组做操作。例如：
\code
int buf[] = {11, 22, 33};
string str = CombineString(',', buf, size(buf)/sizeof(int));
\endcode
这时str的值是"11,22,33"
*/
template<class InputIterator, typename CharType>
typename __BasicString<CharType>::StringType  Basic_CombineString(CharType cTok, InputIterator first, InputIterator last)
{
	typename __BasicString<CharType>::StringStreamType stream;
	for_each(first, last, __private::__combine_stream_helper<CharType>(cTok, stream));
	return stream.str();
}

//! 合并容器内的字符串，之间用分隔符分割
/*!
\param tok 分隔符
\param lhs 第一个字段
\param rhs 第二个字段
\code
string str = ComineString(',', 'abc', 456);
\endcode
这时str的值是"abc,456"
*/
template<typename CharType, class T1, class T2>
typename __BasicString<CharType>::StringType Basic_CombineString(CharType cTok, const T1& lhs, const T2& rhs)
{
	typename __BasicString<CharType>::StringStreamType stream;
	__private::__combine_stream_helper<CharType>(cTok, stream)(lhs)(rhs);
	return stream.str();
}

// 合并头
template<typename CharType, class T>
int Basic_StuffHeader(typename __BasicString<CharType>::StringType& str, const CharType* pszName, const T& value)
{
	str += pszName;
	__private::__combine_string_helper<CharType>((CharType)'=', str)(value);
	str += __private::__get_crlf_string(__private::Type2Type<CharType>());
	return str.length();
}

//! 合并字符串
template<typename CharType, class T>
int Basic_StuffString(typename __BasicString<CharType>::StringType& str, const T& psz)
{
	__private::__combine_string_helper<CharType>('\0', str)(psz);
	return str.length();
}

//! 对字符串编码
template<typename CharType>
typename __BasicString<CharType>::StringType Basic_EnesCWBasicString(const CharType* psz)
{
	typedef typename __BasicString<CharType>::StringType	StringType;
	StringType str;
	const CharType* key = __private::__get_key_string(__private::Type2Type<CharType>());
	const CharType* esc = __private::__get_esc_string(__private::Type2Type<CharType>());
	if(psz != NULL)
	{
		size_t nLength = __tcslen(psz);
		str.resize(nLength*2+1);
		size_t nOffset = 0;
		for(size_t i = 0; i < nLength; i++)
		{
			const CharType* p = __tcschr(key, psz[i]);
			if(p)
			{
				str[nOffset++] = esc[0];
				str[nOffset++] = esc[p - key];
			}
			else
			{
				str[nOffset++] = psz[i];
			}
		}
		str.resize(nOffset);
	}
	return str;
}

//! 对字符串解码
template<typename CharType>
typename __BasicString<CharType>::StringType Basic_DeesCWBasicString(const CharType* psz)
{
	typedef typename __BasicString<CharType>::StringType	StringType;
	StringType str;
	const CharType* key = __private::__get_key_string(__private::Type2Type<CharType>());
	const CharType* esc = __private::__get_esc_string(__private::Type2Type<CharType>());

	if(psz != NULL)
	{
		size_t nLength = __tcslen(psz);
		str.resize(nLength);
		size_t nOffset = 0;
		for(size_t i = 0; i < nLength; i++)
		{
			if(psz[i] == esc[0])
			{
				const CharType* p = __tcschr(esc, psz[i + 1]);
				if(p)
				{
					str[nOffset++] = key[p - esc];
					i++;
				}
			}
			else
			{
				str[nOffset++] = psz[i];
			}
		}
		str.resize(nOffset);
	}
	return str;
}

//! 对字符串解码
template<typename CharType>
int Basic_DeesCWBasicString(const CharType* psz, CharType* pszDest)
{
	int n = 0;
	const CharType* key = __private::__get_key_string(__private::Type2Type<CharType>());
	const CharType* esc = __private::__get_esc_string(__private::Type2Type<CharType>());
	if(psz != NULL)
	{
		size_t nLength = __tcslen(psz);
		for(size_t i = 0; i < nLength; i++)
		{
			if(psz[i] == esc[0])
			{
				size_t nPos = __tcschr(esc, psz[i + 1]) - esc;
				if(nPos >= 0)
				{
					pszDest[n++] = key[nPos];
					i++;
				}
			}
			else
			{
				pszDest[n++] = psz[i];
			}
		}
		pszDest[n] = '\0';
	}
	return n;
}

//! 	将字符串lpszBuffer圆整到长度lRoundLength,末尾添加"..."
/*!
\param lpszbuffer 传入字符串
\param lRoundLength 目标长度
\return 圆整后的长度
*/
long Basic_RoundString(char* lpszBuffer, long lRoundLength);

#ifdef __BASICWINDOWS
BOOL Basic_StringMatchSpec(const WCHAR* pszFile, const WCHAR* pszSpec);
BOOL Basic_StringMatchSpec(const char* pszFile, const char* pszSpec);

#else
namespace __private
{
template<typename CharType>
const CharType* StepSpec(const CharType* pszFileParam, const CharType* pszSpec, bool bWildMatch)
{
	int nParamLen = __tcslen(pszFileParam), nSpecLen = __tcslen(pszSpec);
	if(bWildMatch == false && nParamLen != nSpecLen)
		return NULL;
	int nParamIndex = 0, nSpecIndex = 0;

	while(nParamIndex < nParamLen && nSpecIndex < nSpecLen)
	{
		if (pszFileParam[nParamIndex] == pszSpec[nSpecIndex]
		|| pszSpec[nSpecIndex] == (CharType)'?')
		{
			++ nParamIndex; ++ nSpecIndex;
		}
		else if (bWildMatch)
		{
			nParamIndex -= (nSpecIndex - 1);
			nSpecIndex = 0;
		}
		else
		{
			return NULL;
		}
	}
	if (nSpecIndex == nSpecLen)
	{
		return pszFileParam + nParamIndex;
	}
	return NULL;
}


template<typename CharType>
BOOL MatchLast(const CharType* pszFileParam, const CharType* pszSpec)
{
	if ( (CharType)'\0' == *pszSpec)
		return TRUE;

	int nSpecLen = __tcslen(pszSpec);
	int nFileParamLen = __tcslen(pszFileParam);

	if (nSpecLen > nFileParamLen)
		return FALSE;

	return NULL != StepSpec(pszFileParam + nFileParamLen - nSpecLen, pszSpec, false);
//	return  memcmp(pszFileParam + nFileParamLen - nSpecLen, pszSpec, nSpecLen * sizeof(CharType)) == 0;
}

}
/*!	\brief 实现在windows下PathMatchSpec类似的功能。支持?和*通配符
*	\param pszFile[in] 指向以\0结尾的字符串
*	\param pszSpec[in] 指向以\0结尾的字符串，用于描述匹配规则。
*	\return 匹配返回TRUE,否则FALSE
*/
template<typename CharType>
BOOL Basic_StringMatchSpec(const CharType* pszFile, const CharType* pszSpec)
{
	ASSERT(NULL != pszFile);
	ASSERT(NULL != pszSpec);

	BOOL bMatch = TRUE;
	int nLenSpec = __tcslen(pszSpec);
	size_t nAlloc = (nLenSpec + 1) * sizeof(CharType);
	CharType* p = (CharType*)BasicAllocate(nAlloc);
	memcpy(p, pszSpec, nAlloc - sizeof(CharType));
	p[nLenSpec] = (CharType)'\0';

	CharType* sp = p;
	CharType* e = p;
	bool bWildMatch = false;
	do
	{
		e = __tcschr(p, (CharType)'*');
		if (e != NULL)
		{
			*e++ = (CharType)'\0';
			bWildMatch = true;
		}
		else if (bWildMatch)
		{
			bMatch = __private::MatchLast(pszFile, p);
			break;
		}
		pszFile = __private::StepSpec(pszFile, p, bWildMatch);
		if (NULL == pszFile)
		{
			bMatch = FALSE;
			break;
		}
		p = e;
	}while(p != NULL);
	BasicDeallocate(sp, nAlloc);
	return bMatch;
}
#endif

const int	ENT_NOQUOTES	= 0;	//! 不转换单引号和双引号
const int	ENT_COMPAT	= 1;		//! 只转换双引号
const int	ENT_SQUOTE	= 2;		//! 只转换单引号
const int	ENT_QUOTES	= ENT_COMPAT|ENT_SQUOTE;	//! 同时转换单引号和双引号
namespace __private

{
	const char*		__get_html_spec_string(char c, int quotestyle);
	const WCHAR*	__get_html_spec_string(WCHAR c, int quotestyle);
}


//! \brief: 对html中特殊的字符进行编码。
/*!
	\param	psz[in]			指向以\0结尾的字符串
	\param	quotestyle[in]	对引号的转换规则
	\return	转换后的字符串
	\remark 转换的字符表:
			'&' (ampersand)		=> "&amp;"
			'"'	(double quote)	=> "&quot;"	(没有设置ENT_NOQUOTES的情况下)
			''' (single quote)	=> "&#039;'	(仅当设置了ENT_QUOTES的情况下)
			'<' (less than)		=> "&lt;"
			'>'	(greater than)	=> "&gt;"
*/
template<typename CharType>
typename __BasicString<CharType>::StringType	Basic_HtmlEncode(const CharType* psz, int quotestyle = ENT_COMPAT)
{
	typedef typename __BasicString<CharType>::StringType	StringType;
	StringType ret;
	const CharType* spec = NULL;
	CharType buf[7];
	while(*psz)
	{
		spec = __private::__get_html_spec_string(*psz, quotestyle);
		if (spec)
		{
			ret += spec;
		}
		else if ((*psz) >= 0 && ((*psz) < 0x20) || *psz == 127)	// 不可见
		{
			buf[0] = (CharType)'&';
			buf[1] = (CharType)'#';
			buf[2] = (CharType)((*psz)/100 + '0');
			buf[3] = (CharType)((*psz)%100/10 + '0');
			buf[4] = (CharType)((*psz)%10 + '0');
			buf[5] = (CharType)';';
			buf[6] = 0;
			ret += buf;
		}
		else
		{
			ret += *psz;
		}
		++ psz;
	}
	return ret;
}

namespace __private
{
	const char* __get_html_spec_char(const char* psz);
	const WCHAR* __get_html_spec_char(const WCHAR* psz);
	const char* __get_html_sep_tok(Type2Type<char>);
	const WCHAR* __get_html_sep_tok(Type2Type<WCHAR>);

	struct ReplaceHtmlSpec
	{
		template <typename CharType>
		typename __BasicString<CharType>::StringType	 operator()(const CharType* psz)
		{
			typedef typename __BasicString<CharType>::StringType	StringType;
			StringType ret;
			if (*psz == (CharType)'#')
			{
				ret = (CharType)__tcstol(psz + 1, NULL, 10);
			}
			else
			{
				const CharType* s = __get_html_spec_char(psz);
				if (s)
				{
					ret += (s);
				}
				else
				{
					ret += (CharType)'&';
					ret += psz;
					ret += (CharType)';';
				}
			}
			return ret;
		}
	};
}

//! \brief: 对html中特殊的字符进行解码
/*!
\param	psz[in]			指向以\0结尾的字符串
\return	转换后的字符串
\remark 转换的字符表:
	'&' (ampersand)		=> "&amp;"
	'"'	(double quote)	=> "&quot;"
	''' (single quote)	=> "&#039;"
	'<' (less than)		=> "&lt;"
	'>'	(greater than)	=> "&gt;"
	其他以"&#xxx;"格式的字符串。
*/
template<typename CharType>
typename __BasicString<CharType>::StringType	Basic_HtmlDecode(const CharType* psz)
{
	typedef typename __BasicString<CharType>::StringType	StringType;
	StringType ret = Basic_FillParamString(psz, __private::__get_html_sep_tok(__private::Type2Type<CharType>()), __private::ReplaceHtmlSpec());
	return ret;
}



namespace __private
{
	template<typename CharType>
	typename __CharValue<CharType>::Result get_v(CharType c)
	{
		if (c >= (CharType)'0' && c <= (CharType)'9')
		{
			return c - (CharType)'0';
		}
		else if (c >= (CharType)'a' && c <= (CharType)'f')
		{
			return c - (CharType)'a' + 10;
		}
		else if (c >= (CharType)'A' && c <= (CharType)'F')
		{
			return c - (CharType)'A' + 10;
		}
		else
		{
			return 0;
		}
	}
}
//! \brirf: 对字符串做url编码
/*!
\param	psz[in]		指向以\0结尾的字符串
\return 转换后的字符串
\remark	
转换规则：将字符串中除了'-'和'_'外的所有非字母和非数字字符都替换成%后跟两位十六进制数，空格则编码为加号(+)。
该函数仅有Multi版本，没有UNICODE版本。
*/
char_string	Basic_URLEncode(const char* psz);

//! 对url编码后的字符串做解码
/*!
\param	psz[in]		指向以\0结尾的字符串
\return 转换后的字符串
*/
template<typename CharType>
typename __BasicString<CharType>::StringType Basic_URLDecode(const CharType* psz)
{
	typedef typename __BasicString<CharType>::StringType StringType;

	enum {
		CharNone,
		CharFirst,
		CharSecond
	} status = CharNone;
	typename __CharValue<CharType>::Result c = 0;
	StringType ret;
	while(psz && *psz)
	{
		switch(*psz)
		{
		case (CharType)'+':
			ret += ' ';
			break;
		case (CharType)'%':
			switch(status)
			{
			case CharNone:
				status = CharFirst;
				break;
			case CharFirst:
				status = CharNone;
				ret += (CharType)'%';
				break;
			case CharSecond:	// 肯定不是标准的,普通处理
				ret += (CharType)'%';
				ret += (CharType)c;
				status = CharFirst;
				break;
			}
			c = 0;
			break;
		default:
			switch(status)
			{
			case CharNone:
				ret += *psz;
				break;
			case CharFirst:
				c = *psz;
				status = CharSecond;
				break;
			case CharSecond:
				c = __private::get_v(c) << 4 | __private::get_v(*psz);
				ret += (CharType)c;
				status = CharNone;
				break;
			}
			break;
		}
		++ psz;
	}
	return ret;
}

//! 转成16进制的字符串
int Basic_ConvertStringToHexString(const char *pszSrc, int nCount, char* pszDest, int nDest);

//! 将HEX编码后的字符串转化为hex源串
/*!
\param	pszSrc[in]		源字符串
\param	nCount[in]		pszSrc的长度
\param	pszDest[out]	转换后输出
\param	nDest[out]		输出的pszDest的字符串长度(不是内存长度)
\return 转换后的字符串的长度
*/
template<class CharType>
int Basic_ConvertStringToHex(const char *pszSrc, int nCount, CharType* pszDest, int nDest)
{
	if(pszSrc == NULL || nCount <= 0 || pszDest == NULL || nDest <= 0 || nCount*2 > nDest)
	{
		return nCount << 1; // *2
	}
	int i = 0;
	for (i = 0; i < nCount; i++)
	{
		int n1 = (*(unsigned char *)(pszSrc+i)) % 16;
		int n2 = (*(unsigned char *)(pszSrc+i)) / 16;
		pszDest[2 * i] = (CharType)'A' + n1;
		pszDest[2 * i + 1] = (CharType)'A' + n2;
	}
	return nCount * 2;
}


//! 将HEX编码后的字符串转化为hex源串
/*!
\param	pszSrc[in]		源数据
\param	nCount[in]		pszSrc的长度
\param	pszDest[out]	转换后输出
\param	nDest[out]		输出的字符串长度大小.(CharType*nDest的大小)
\return 转换后Buffer的长度
*/
template<class CharType>
int Basic_ConvertHexToString(const CharType *pszSrc, int nCount, char* pszDest, int nDest)
{
	if(pszSrc == NULL || pszDest == NULL || nCount > nDest * 2)
	{
		return nCount >> 1;	// /2
	}

	nDest = nCount / 2;
	int i = 0, j = 0;
	for (i = 0; i < nDest; i++)
	{
		j = i << 1;	// * 2
		//ASSERT(j < nCount - 1);
		int n1 = pszSrc[j] - (CharType)('A');
		int n2 = pszSrc[j + 1]  - (CharType)('A');
		pszDest[i] = (char)(n2 << 4) | n1;
	}
	return nDest;
}


//! 

//! 对字符串编码
template<typename CharType>
typename __BasicString<CharType>::StringType Basic_EncodeString(const CharType* psz)
{
	typedef typename __BasicString<CharType>::StringType	StringType;
	StringType str;

	if(psz != NULL)
	{
		size_t nLength = __tcslen(psz);
		StringType encode;
		encode.resize(nLength + 2);

		CharType *p = (CharType*)encode.c_str();
		srand((unsigned)time(NULL)); 
		p[0] = (CharType)'\0';
		while(p[0] == (CharType)'\0')
		{
			p[0] = rand() % 256;
		}

		__tcscpy(&p[1], psz);
		int nCount = __tcslen(p);
		int i = 0;
		for (i = 1; i < nCount; i++)
		{
			p[i] = p[i - 1] ^ p[i];
		}

		CharType EncodeBufffer[256];
		EncodeBufffer[0] = (CharType)'1';
		int nRet = Basic_ConvertStringToHex((const char*)p, sizeof(CharType) * nCount, &EncodeBufffer[1], sizeof(EncodeBufffer)-1);
		if (nRet < 255)
		{
			EncodeBufffer[nRet + 1] = (CharType)'\0';
		}
		str = EncodeBufffer;
	}
	return str;
}

//! 对字符串解码
template<typename CharType>
typename __BasicString<CharType>::StringType Basic_DecodeString(const CharType* psz)
{
	typedef typename __BasicString<CharType>::StringType	StringType;
	StringType str;

	CharType DecodeBufffer[256];
	int nCount = __tcslen(psz);

	memset(DecodeBufffer, 0, sizeof(DecodeBufffer));
	int nOffset = 0;
	CharType cVer = 0;
	if(nCount % 2)
	{
		cVer = psz[0];
		nOffset = 1;
		if(cVer != (CharType)'1')
		{
			return str;
		}
	}
	int nDest = nCount / 2 + 2;
	CharType *p = new CharType[nDest];

	Basic_ConvertHexToString(&psz[nOffset], nCount, (char*)p, nDest);
	nDest -= 2;
	nDest /= sizeof(CharType);
	if(cVer == 0)
	{
		int i = 1;
		for (i = 1; i < nDest; i++)
		{
			p[i] = p[i - 1] ^ p[i];
		}
		p[i] = (CharType)'\0';
	}
	else
	{
		CharType cMask = p[0];
		CharType cOld  = cMask;
		int i = 1;
		for (i = 1; i < nDest; i++)
		{
			cOld  = p[i];
			p[i]  = cMask ^ p[i];
			cMask = cOld;
		}
		p[i] = (CharType)'\0';
	}

	str = &p[1];
	delete [] p;

	return str;
}

template<class CharType>
const CharType*	Basic_LTrim(const CharType* str)
{
	FORWARD_SPACE(str, Null_String, 0);
	return str;
}
const char* Basic_LTrim(const char* str);

template<class CharType>
CharType*	Basic_LTrim(CharType* str)
{
	FORWARD_SPACE(str, Null_String, 0);
	return str;
}
char* Basic_LTrim(char* str);

template<class CharType>
CharType* Basic_RTrim(CharType* str, size_t len = NullLen)
{
	if (NullLen == len)
		len = __tcslen(str);
	if (len > 0)
	{
		CharType* p = str + len - 1;
		while(ISSPACE(*p) && (len--) > 0)
			-- p;
		*(p + 1) = (CharType)0;
	}

	return str;   
}

template<class CharType>
CharType* Basic_Trim(CharType* str)
{
	return Basic_RTrim(Basic_LTrim((CharType*)str));
}

template<class CharType>
void Basic_InsertNumberSpace(typename __BasicString<CharType>::StringType& str, CharType tok)
{
	const CharType* ps = str.c_str();
	size_t len = str.size();
	size_t pos = len;
	const CharType* p = __tcschr(ps, (CharType)'.');
	if (p)
		pos = p - ps;

	do 
	{
		int count = pos / 3;
		if (count == 0)
			break;

		pos = pos % 3;
		CBasicStaticBuffer buf;
		buf.Assign(2*(len + 1) * sizeof(CharType), 0);
		CharType* p = (CharType*)buf.GetBuffer();

		if (pos > 0)
		{
			p = __tcsncpy(p, ps, pos);
			p += pos;
			*p++ = _T(',');
			ps += pos;

		}
		for (int i = 0; i < count - 1; ++ i)
		{
			__tcsncpy(p, ps, 3);
			p += 3;
			*p ++ = _T(',');
			ps += 3;
		}
		__tcscpy(p, ps);
		str = buf.GetString();
	} while (false);
}


template<typename CharType>
CharType* __tcscpyn(CharType* strDest, size_t nDest, const CharType* strSource, size_t nSource = NullLen, bool bTrim = false)
{
	if(nSource == NullLen)
	{
		nSource = __tcslen(strSource);
	}
	if(bTrim)
	{
		size_t i = 0;
		for(i = nSource - 1; i != NullLen; -- i)
		{
			if(!ISSPACE(strSource[i]))
			{
				break;
			}
		}
		nSource = i + 1;
	}
	size_t nLen = MIN(nSource, nDest);
	if(nLen > 0)
	{
		memcpy(strDest, strSource, nLen * sizeof(CharType));
	}
	if(nLen < nDest)
	{
		strDest[nLen] = (CharType)0;
	}
	return strDest;
}

/** 
*\brief __atoi64 字符串转64位整数
* 
*\param str 
*\param nLen 默认值 -1
*\return  
*/
LONG64 __atoi64W(const TCHAR* str, int nLen/* = -1*/);
LONG64 __atoi64_s(const char* str, int nLen/* = -1*/);


// 一些对于basic_string的操作
namespace strutil
{
	const char*		__get_blank_string(__private::Type2Type<char>);
	const WCHAR*	__get_blank_string(__private::Type2Type<WCHAR>);

	//! 字符串变大写
	template<class StringType>
	void makeupper(StringType& s)
	{
		transform(s.begin(), s.end(), s.begin(), (int(*)(int))toupper);
	}

	//! 字符串变小写
	template<class StringType>
	void makelower(StringType& s)
	{
		transform(s.begin(), s.end(), s.begin(), (int(*)(int))tolower);
	}

	//! 去除左侧包含在lpszTarget中的字符
	template<class StringType>
	void ltrim(StringType& s, typename StringType::const_pointer lpszTarget)
	{
		s.erase(0, s.find_first_not_of(lpszTarget));
	}


	//! 去除左侧空字符
	template<class StringType>
	void ltrim(StringType& s)
	{
		typedef typename StringType::value_type value_type;
		ltrim(s, __get_blank_string(__private::Type2Type<value_type>()));
	}

	//! 去除左侧指定字符
	template<class StringType>
	void ltrim(StringType& s, typename StringType::value_type cTarget)
	{
		s.erase(0, s.find_first_not_of(cTarget));
	}

	//! 去除右侧包含在lpszTarget中的字符
	template<class StringType>
	void rtrim(StringType& s, typename StringType::const_pointer lpszTarget)
	{
		s.erase(s.find_last_not_of(lpszTarget) + 1);
	}

	//! 去除右侧空字符
	template<class StringType>
	void rtrim(StringType& s)
	{
		typedef typename StringType::value_type value_type;
		rtrim(s, __get_blank_string(__private::Type2Type<value_type>()));
	}

	//! 去除右侧指定字符
	template<class StringType>
	void rtrim(StringType& s, typename StringType::value_type cTarget)
	{
		s.erase(s.find_last_not_of(cTarget) + 1);
	}

	//! 替换字符串
	template<class StringType>
	int replace(StringType& s, typename StringType::const_pointer string_to_replace, typename StringType::const_pointer new_string)
	{
		typedef typename StringType::size_type size_type;
		int ret = 0;
		size_t oldlen = __tcslen(string_to_replace);
		size_t newlen = __tcslen(new_string);
		size_type pos = s.find(string_to_replace);
		while(StringType::npos != pos)
		{
			++ ret;
			s.replace(pos, oldlen, new_string);
			pos = s.find(string_to_replace, pos + newlen);
		}
		return ret;
	}


	template<typename CharType>
	struct __replace_char
	{
		__replace_char(CharType cOld, CharType cNew, int& ret) : __old(cOld), __new(cNew), __ret(ret){}

		CharType operator()(CharType c)
		{
			if (__old == c)
			{
				++ __ret;
				return __new;
			}
			else
				return c;
		}

		int&		__ret;
		CharType	__old;
		CharType	__new;
	};

	//! 替换字符
	template<class StringType>
	int replace(StringType& s, typename StringType::value_type cOld, typename StringType::value_type cNew)
	{
		typedef typename StringType::value_type value_type;
		int ret = 0;
		transform(s.begin(), s.end(), s.begin(), __replace_char<value_type>(cOld, cNew, ret));
		return ret;
	}
}

__NS_BASIC_END

#endif 
