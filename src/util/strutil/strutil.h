/***********************************************************************************************
// Œƒº˛√˚:     strutil.h
// ¥¥Ω®’ﬂ:     ≤Ã’Ò«Ú
// Email:      zqcai@w.cn
// ¥¥Ω® ±º‰:   2012/2/17 11:17:20
// ƒ⁄»›√Ë ˆ:   “ª–©≥£”√µƒ◊÷∑˚¥Æ¥¶¿Ì∫Ø ˝£¨∞¸¿®£∫
≤∑÷◊÷∑˚¥Æ°¢∫œ≤¢◊÷∑˚¥Æ°¢string◊÷∑˚¥Æ¿‡µƒ“ª–©≥£”√≤Ÿ◊˜°£
// ∞Ê±æ–≈œ¢:   1.0V
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
#include "../../mem/tlstaticbuffer.h"
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
const char NullAString[] = { 0 };
#define Null_String_S		""

//! ◊÷∑˚¥Æ°¢◊÷∑˚¥Æ¡˜µƒ∂®“Â°£
//! ƒøµƒ£∫ø…“‘»´æ÷Õ≥“ªÃÊªª∑÷≈‰∆˜°£

template<typename CharType>
struct __BasicString
{
	typedef typename basic_basic_string<CharType>::type			StringType;
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
__NS_BASIC_END
#ifdef __GNUC__
#include <functional>
#endif
namespace std{
	template<>
	struct hash<basiclib::char_string> : public std::unary_function<basiclib::char_string, std::size_t>{
		std::size_t operator()(const basiclib::char_string &key) const{
			#ifdef __BASICWINDOWS
				return _Hash_seq((const unsigned char*)key.c_str(), key.length());
			#else
#ifdef __GNUC__
                hash<const char*> hash_fn;
                return hash_fn(key.c_str());
#else
				hash<const char*> hash_fn;
				return hash_fn(key.c_str(), key.length());
#endif
			#endif			
		}
	};
}

__NS_BASIC_START

// ”…◊÷∑˚¿‡–Õ»°µ√∂‘”¶µƒusignedµƒ¿‡–Õ
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

// ”…unsignedµƒ¿‡–Õ»°µ√∂‘”¶µƒ◊÷∑˚¿‡–Õ
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

//! ≈–∂œ◊÷∑˚¥Æ «∑ÒŒ™ø’
/*!
\param p ≈–∂œ◊÷∑˚¥Æ «∑ÒŒ™ø’
*/
template<typename CharType>
bool IsStringEmpty(const CharType* p)
{
	return (NULL == p) || ((CharType)0 == p[0]);
}

//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
/*!
\param p ¥¶¿Ìµƒ◊÷∑˚¥Æ°£∫Ø ˝ª·–ﬁ∏ƒ¿Ô√Êµƒƒ⁄»›°£≤ªƒ‹Œ™ø’
\param cTok ∑÷∏Ó∑˚
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remarks ∏√∫Ø ˝ª·–ﬁ∏ƒpÀ˘÷∏œÚµƒ◊÷∑˚¥Æƒ⁄»›
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
//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù◊÷∑˚¥Æ∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
/*!
\param p ‘¥◊÷∑˚¥Æ°£∫Ø ˝ª·–ﬁ∏ƒ¿Ô√Êµƒƒ⁄»›°£≤ªƒ‹Œ™ø’°£
\param pszTok ∑÷∏Ó”√◊÷∑˚¥Æ
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remarks ∏√∫Ø ˝ª·–ﬁ∏ƒpÀ˘÷∏œÚµƒ◊÷∑˚¥Æƒ⁄»›
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

//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
/*!
\param psz ‘¥◊÷∑˚¥Æ,≤ªƒ‹Œ™ø’
\param cTok ∑÷∏Ó∑˚∫≈
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remarks ∫Õ__SpliteStringµƒ«¯± «£¨∏√∫Ø ˝≤ª–ﬁ∏ƒpsz÷∏œÚƒ⁄»›°£
 æ¿˝ø…“‘≤Œøº∑¬∫Ø ˝IntoContainer
*/
template<class Functor, typename CharType>
void BasicSpliteString(const CharType* psz, CharType cTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	__SpliteString((CharType*)str.c_str(), cTok, func);
}

//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
/*!
\param psz ‘¥◊÷∑˚¥Æ£¨≤ªƒ‹Œ™ø’°£
\param pszTok ∑÷∏Ù◊÷∑˚¥Æ
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remarks ∫Õ__SpliteStringµƒ«¯± «£¨∏√∫Ø ˝≤ª–ﬁ∏ƒpsz÷∏œÚƒ⁄»›°£
 æ¿˝ø…“‘≤Œøº∑¬∫Ø ˝IntoContainer
*/
template<class Functor, typename CharType>
void BasicSpliteString(const CharType* psz, const CharType* pszTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	__SpliteString((CharType*)str.c_str(), pszTok, func);
}

//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
/*!
\param psz ‘¥◊÷∑˚¥Æ
\
\param pszTok ∑÷∏Ù◊÷∑˚¥Æ,≤ªƒ‹Œ™ø’
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remarks ∫Õ__SpliteStringµƒ«¯± «£¨∏√∫Ø ˝≤ª–ﬁ∏ƒpsz÷∏œÚƒ⁄»›°£
 æ¿˝ø…“‘≤Œøº∑¬∫Ø ˝IntoContainer
*/
template<class Functor, typename CharType>
void BasicSpliteString(const CharType* psz, long length, CharType cTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz, length);
	__SpliteString((CharType*)str.c_str(), cTok, func);
}

//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
/*!
\param psz ‘¥◊÷∑˚¥Æ£¨≤ªƒ‹Œ™ø’
\param length ‘¥◊÷∑˚¥Æµƒ≥§∂»
\param pszTok ∑÷∏Ù◊÷∑˚¥Æ
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remarks ∫Õ__SpliteStringµƒ«¯± «£¨∏√∫Ø ˝≤ª–ﬁ∏ƒpsz÷∏œÚƒ⁄»›°£
 æ¿˝ø…“‘≤Œøº∑¬∫Ø ˝IntoContainer
*/
template<class Functor, typename CharType>
void BasicSpliteString(const CharType* psz, long length, const CharType* pszTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz, length);
	__SpliteString((CharType*)str.c_str(), pszTok, func);
}


//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
/*!
\param psz ‘¥◊÷∑˚¥Æ£¨≤ªƒ‹Œ™ø’
\param pszTok ‘⁄pszTok÷–µƒ◊÷∑˚∂ºª·±ªµ±◊˜∑÷∏Ù∑˚ π”√
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remark ∏√∫Ø ˝ª·–ﬁ∏ƒpÀ˘÷∏œÚµƒ◊÷∑˚¥Æƒ⁄»›
 æ¿˝ø…“‘≤Œøº∑¬∫Ø ˝IntoContainer
*/
template<class Functor, typename CharType>
void __Explode(CharType* psz, const CharType* pszTok, Functor func)
{
	CharType* e = psz;
	while(0 != *e)
	{
		// ’“µΩ∑÷∏Ù∑˚
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
/*! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚±ÌΩ¯––∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
\param psz ‘¥◊÷∑˚¥Æ£¨≤ªƒ‹Œ™ø’
\param pszTok ‘⁄pszTok÷–µƒ◊÷∑˚∂ºª·±ªµ±◊˜∑÷∏Ù∑˚ π”√
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remark ∫Õ__Explodeµƒ«¯± «∏√∫Ø ˝≤ª–ﬁ∏ƒpsz÷∏œÚƒ⁄»›°£
*/
template<class Functor, typename CharType>
void Basic_Explode(const CharType* psz, const CharType* pszTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	__Explode((CharType*)str.c_str(), pszTok, func);
}

/*! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚±ÌΩ¯––∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì
\param psz ‘¥◊÷∑˚¥Æ£¨≤ªƒ‹Œ™ø’
\param length psz◊÷∑˚¥Æµƒ≥§∂»
\param pszTok ‘⁄pszTok÷–µƒ◊÷∑˚∂ºª·±ªµ±◊˜∑÷∏Ù∑˚ π”√
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£
\remark ∫Õ__Explodeµƒ«¯± «∏√∫Ø ˝≤ª–ﬁ∏ƒpsz÷∏œÚƒ⁄»›°£
*/
template<class Functor, typename CharType>
void Basic_Explode(const CharType* psz, long length, const CharType* pszTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz, length);
	__Explode((CharType*)str.c_str(), pszTok, func);
}




//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì£¨÷±µΩ◊÷∑˚¥ÆŒ≤≤øªÚ’ﬂfunc∑µªÿtrue
/*!
\param psz ‘¥◊÷∑˚¥Æ£¨≤ªƒ‹Œ™ø’°£
\param cTok ∑÷∏Ù∑˚
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£µ±∏√∫Ø ˝∑µªÿtrue ±£¨∫Ø ˝ÕÀ≥ˆ°£
\remarks ∏√∫Ø ˝ª·–ﬁ∏ƒpÀ˘÷∏œÚµƒƒ⁄»›°£ 
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

//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì£¨÷±µΩ◊÷∑˚¥ÆŒ≤≤øªÚ’ﬂfunc∑µªÿtrue
/*!
\param psz ‘¥◊÷∑˚¥Æ,≤ªƒ‹Œ™ø’°£
\param cTok ∑÷∏Ù∑˚
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝°£µ±∏√∫Ø ˝∑µªÿtrue ±£¨∫Ø ˝ÕÀ≥ˆ°£
\remarks ∫Õ__SpliteStringBreakµƒ«¯± «£¨∏√∫Ø ˝≤ª–ﬁ∏ƒpsz÷∏œÚƒ⁄»›°£
*/
template<class Functor, typename CharType>
bool BasicSpliteStringBreak(const CharType* psz, CharType cTok, Functor func)
{
	typedef typename __BasicString<CharType>::StringType StringType;
	StringType str(psz);
	return __SpliteStringBreak((CharType*)str.c_str(), cTok, func);
}

//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì°£∂‘”⁄'\'ø™Õ∑µƒ◊÷∑˚¥ÆΩ´◊ˆ◊™“Â°£
/*!
\param p ‘¥◊÷∑˚¥Æ,≤ªƒ‹Œ™ø’°£
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝
\remarks ∏√∫Ø ˝ª·–ﬁ∏ƒpÀ˘÷∏œÚµƒ‘¥◊÷∑˚¥Æ°£
*/
template<class Functor, typename CharType>
bool __SpliteStringBreakWithEscape(CharType* p, CharType cTok, Functor func)
{
	CharType* e = p;
	CharType* v = p;
	while(0 != *e)
	{
		if ((CharType)'\\' == *e &&  0 != *(e+1))	// ◊™“Â∑˚∫≈,¬‘π˝œ¬“ª∏ˆ◊÷∑˚
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

//! ∂‘◊÷∑˚¥Æ∞¥’’÷∏∂®µƒ∑÷∏Ù∑˚∑÷∏Ó£¨≤¢“¿¥ŒΩª”…func∫Ø ˝¥¶¿Ì°£÷ß≥÷'\'◊™“Â
/*!
\param psz ‘¥◊÷∑˚¥Æ,≤ªƒ‹Œ™ø’
\param cTok ∑÷∏Ù∑˚
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝
\remarks ∫Õ__SpliteStringBreakWithEscapeµƒ«¯± «SpliteStringBreakWithEscape≤ª–ﬁ∏ƒpszµƒƒ⁄»›°£
\code
char* buf = "a\\;;b;\\\\\\;c";	//◊÷∑˚¥Æ a\;;b;\\\;cd
BasicSpliteStringBreakWithEscape(buf, ';', functor);

ƒ«√¥functor ’µΩµƒ◊÷∑˚¥Æ”¶∏√ «£∫
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


const int PPS_OPS_LOWERKEY		= 1;	// Ω´µ⁄“ª∏ˆ◊÷∑˚¥Æ◊™≥…–°–¥
const int PPS_OPS_LOWERVALUE	= 2;	// Ω´µ⁄∂˛∏ˆ◊÷∑˚¥Æ◊™≥…–°–¥
const int PPS_OPS_SKIPBLANK		= 4;	// ¬‘π˝◊÷∑˚¥Æ«∞∫Ûµƒø’∏Ò/tab/ªª––

namespace __private
{
// “‘œ¬∫Ø ˝æ˘Œ™ParseParamString π”√
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

//! Ω‚Œˆ–ŒÀ∆”Î"a=b&c=d&e=f"µƒ◊÷∑˚¥Æ£¨≤¢Ω´(a,b),(c,d),(e,f)“¿¥Œ¥¶¿Ì
/*!
\param psz ‘¥◊÷∑˚
\param tok[0]∑÷±±Í ∂≈‰∂‘∑÷∏Ù∑˚°£tok[1]±Í ∂Ãıƒø∑÷∏Ù∑˚
\param func ◊÷∑˚¥Æ∂‘Ω¯––¥¶¿Ìµƒ∫Ø ˝ªÚ’ﬂ∑¬∫Ø ˝
\param ParseTraits	∂‘æﬂÃÂ“ª–©Ãÿ–‘µƒ›Õ»°
\return Œﬁ∑µªÿ÷µ
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
			{	// µ⁄“ª∏ˆ∑÷∏Ó∑˚
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
			{	// µΩ¡ÀŒ≤≤ø¡À
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

//! Ω‚Œˆ–ŒÀ∆”Î"a=b&c=d&e=f"µƒ◊÷∑˚¥Æ£¨≤¢Ω´(a,b),(c,d),(e,f)“¿¥Œ¥¶¿Ì
/*!
\param psz ‘¥◊÷∑˚,≤ªƒ‹Œ™ø’
\param psz ◊÷∑˚¥Æµƒ≥§∂»
\param tok[0]∑÷±±Í ∂≈‰∂‘∑÷∏Ù∑˚°£tok[1]±Í ∂Ãıƒø∑÷∏Ù∑˚
\param func ◊÷∑˚¥Æ∂‘Ω¯––¥¶¿Ìµƒ∫Ø ˝ªÚ’ﬂ∑¬∫Ø ˝
\param Ops ≤Œ ˝—°œÓ°£÷µŒ™PPS_OPS_*
\return Œﬁ∑µªÿ÷µ
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

//! Ω‚Œˆ–ŒÀ∆”Î"a=b&c=d&e=f"µƒ◊÷∑˚¥Æ£¨≤¢Ω´(a,b),(c,d),(e,f)“¿¥Œ¥¶¿Ì
/*!
\param psz ‘¥◊÷∑˚,≤ªƒ‹Œ™ø’°£±ÿ–Î“‘'\0'Ω·Œ≤
\param tok[0]∑÷±±Í ∂≈‰∂‘∑÷∏Ù∑˚°£tok[1]±Í ∂Ãıƒø∑÷∏Ù∑˚
\param func ◊÷∑˚¥Æ∂‘Ω¯––¥¶¿Ìµƒ∫Ø ˝ªÚ’ﬂ∑¬∫Ø ˝
\param Ops ≤Œ ˝—°œÓ°£÷µŒ™PPS_OPS_*
\return Œﬁ∑µªÿ÷µ
*/
template<class Functor, typename CharType>
void Basic_ParseParamString(const CharType* psz, const CharType tok[], Functor func, int nOps = PPS_OPS_LOWERKEY|PPS_OPS_SKIPBLANK)
{
	Basic_ParseParamString(psz, __tcslen(psz), tok, func, nOps);
}

//! ÃÓ≥‰◊÷∑˚¥Æ÷–tok÷∏∂®µƒƒ⁄µƒ≤Œ ˝°£≤Œ ˝µƒ÷µ”…func¿¥»°µ√°£
/*!
\param str ‘¥◊÷∑˚¥Æ£¨≤ªƒ‹Œ™ø’°£
\param tok ”√”⁄÷∏∂®∆ º°£¿˝»Á"<>"
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝
\remarks strƒ⁄µƒƒ⁄»›ª·±ª–ﬁ∏ƒ°£
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


//! ∏˘æ›format◊÷∑˚¥Æ£¨»°≥ˆ‘¥◊÷∑˚¥Æ÷–µƒ÷µ
/*!
\param pszSrc ‘¥◊÷∑˚,≤ªƒ‹Œ™ø’
\param tok[0]∑÷±±Í ∂≈‰∂‘∑÷∏Ù∑˚°£tok[1]±Í ∂Ãıƒø∑÷∏Ù∑˚
\param func ◊÷∑˚¥Æ∂‘Ω¯––¥¶¿Ìµƒ∫Ø ˝ªÚ’ﬂ∑¬∫Ø ˝
\return Œﬁ∑µªÿ÷µ
\remarks strƒ⁄µƒƒ⁄»›ª·±ª–ﬁ∏ƒ°£
*/
template <class CharType, class Functor>
void __GetParamString(CharType* lpszSrc, CharType* lpszFormat, const CharType tok[], Functor func)
{
	ASSERT(!IsStringEmpty(lpszFormat) && !IsStringEmpty(lpszSrc));

	CharType *pEnd, *pKey, *pSep;
	pEnd = pKey = pSep = NULL;
	size_t len = 0;
	while(lpszFormat && lpszSrc && 
		*lpszFormat != '\0' && *lpszSrc != '\0')
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

//! ∏˘æ›format◊÷∑˚¥Æ£¨»°≥ˆ‘¥◊÷∑˚¥Æ÷–µƒ÷µ
/*!
\param pszSrc ‘¥◊÷∑˚,≤ªƒ‹Œ™ø’
\param tok[0]∑÷±±Í ∂≈‰∂‘∑÷∏Ù∑˚°£tok[1]±Í ∂Ãıƒø∑÷∏Ù∑˚
\param func ◊÷∑˚¥Æ∂‘Ω¯––¥¶¿Ìµƒ∫Ø ˝ªÚ’ﬂ∑¬∫Ø ˝
\return Œﬁ∑µªÿ÷µ
\code
Basic_GetParamString("/shase/600000.txt", "/{market}/{code}.txt", "{}", InfoMapContainer<tstring, tstring>());
\endcode
 ‰≥ˆ:
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


//! ÃÓ≥‰◊÷∑˚¥Æ÷–tok[0]∫Õtok[1]ƒ⁄µƒ≤Œ ˝°£≤Œ ˝µƒ÷µ”…func¿¥»°µ√°£
/*!
\param psz ‘¥◊÷∑˚¥Æ£¨≤ªƒ‹Œ™ø’
\param tok ±Í ∂πÿº¸◊÷«¯º‰µƒ◊÷∑˚£¨÷¡…Ÿ¡Ω∏ˆ◊÷∑˚£¨∑÷±”√”⁄±Í ∂∆ º∫ÕΩ· ¯
\param func ”√”⁄∏˘æ›≤Œ ˝»°µ√÷µµƒ∑¬∫Ø ˝
\remarks ∫Õ__FillParamStringµƒ«¯± «Basic_FillParamString≤ª–ﬁ∏ƒpszµƒƒ⁄»›°£
*/
template<class Functor, typename CharType>
typename __BasicString<CharType>::StringType	Basic_FillParamString(const CharType* psz, const CharType tok[], Functor func)
{
	typename __BasicString<CharType>::StringType	str(psz);
	return __FillParamString((CharType*)str.c_str(), tok, func);
}

namespace __private
{
	//! ∫œ≤¢ ˝æ›∂‘œÛµΩ◊÷∑˚¥Æµƒ¥¶¿Ì∑¬∫Ø ˝
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

	//! ∫œ≤¢ ˝æ›∂‘œÛµΩ◊÷∑˚¥Æ¡˜µƒ¥¶¿Ì∑¬∫Ø ˝
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


//! ∫œ≤¢»›∆˜ƒ⁄µƒ◊÷∑˚¥Æ£¨÷Æº‰”√∑÷∏Ù∑˚∑÷∏Ó
/*!
\param tok ∑÷∏Ù∑˚
\param first input iterator£¨”√”⁄±Í ∂∆ º‘™Àÿ
\param last input iterator£¨”√”⁄±Í ∂Ω· ¯‘™Àÿ
»Áπ˚∑÷∏Ó∑˚÷µŒ™'\0',‘Ú◊÷∑˚¥Æ÷±Ω”∫œ≤¢°£
\code
char* buf[] = {"aa", "bb", "cc"};
string str = ComineString(',', buf, sizeof(buf)/sizeof(char*));
\endcode
’‚ ±strµƒ÷µ «"aa,bb,cc"
¡ÌÕ‚“≤ø…“‘∂‘∑«◊÷∑˚¥Æ ˝◊È◊ˆ≤Ÿ◊˜°£¿˝»Á£∫
\code
int buf[] = {11, 22, 33};
string str = CombineString(',', buf, size(buf)/sizeof(int));
\endcode
’‚ ±strµƒ÷µ «"11,22,33"
*/
template<class InputIterator, typename CharType>
typename __BasicString<CharType>::StringType  Basic_CombineString(CharType cTok, InputIterator first, InputIterator last)
{
	typename __BasicString<CharType>::StringStreamType stream;
	for_each(first, last, __private::__combine_stream_helper<CharType>(cTok, stream));
	return stream.str();
}

//! ∫œ≤¢»›∆˜ƒ⁄µƒ◊÷∑˚¥Æ£¨÷Æº‰”√∑÷∏Ù∑˚∑÷∏Ó
/*!
\param tok ∑÷∏Ù∑˚
\param lhs µ⁄“ª∏ˆ◊÷∂Œ
\param rhs µ⁄∂˛∏ˆ◊÷∂Œ
\code
string str = ComineString(',', 'abc', 456);
\endcode
’‚ ±strµƒ÷µ «"abc,456"
*/
template<typename CharType, class T1, class T2>
typename __BasicString<CharType>::StringType Basic_CombineString(CharType cTok, const T1& lhs, const T2& rhs)
{
	typename __BasicString<CharType>::StringStreamType stream;
	__private::__combine_stream_helper<CharType>(cTok, stream)(lhs)(rhs);
	return stream.str();
}

// ∫œ≤¢Õ∑
template<typename CharType, class T>
int Basic_StuffHeader(typename __BasicString<CharType>::StringType& str, const CharType* pszName, const T& value)
{
	str += pszName;
	__private::__combine_string_helper<CharType>((CharType)'=', str)(value);
	str += __private::__get_crlf_string(__private::Type2Type<CharType>());
	return str.length();
}

//! ∫œ≤¢◊÷∑˚¥Æ
template<typename CharType, class T>
int Basic_StuffString(typename __BasicString<CharType>::StringType& str, const T& psz)
{
	__private::__combine_string_helper<CharType>('\0', str)(psz);
	return str.length();
}

//! ∂‘◊÷∑˚¥Æ±‡¬Î
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

//! ∂‘◊÷∑˚¥ÆΩ‚¬Î
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

//! ∂‘◊÷∑˚¥ÆΩ‚¬Î
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
				int nPos = __tcschr(esc, psz[i + 1]) - esc;
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

//! 	Ω´◊÷∑˚¥ÆlpszBuffer‘≤’˚µΩ≥§∂»lRoundLength,ƒ©Œ≤ÃÌº”"..."
/*!
\param lpszbuffer ¥´»Î◊÷∑˚¥Æ
\param lRoundLength ƒø±Í≥§∂»
\return ‘≤’˚∫Ûµƒ≥§∂»
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
/*!	\brief  µœ÷‘⁄windowsœ¬PathMatchSpec¿‡À∆µƒπ¶ƒ‹°£÷ß≥÷?∫Õ*Õ®≈‰∑˚
*	\param pszFile[in] ÷∏œÚ“‘\0Ω·Œ≤µƒ◊÷∑˚¥Æ
*	\param pszSpec[in] ÷∏œÚ“‘\0Ω·Œ≤µƒ◊÷∑˚¥Æ£¨”√”⁄√Ë ˆ∆•≈‰πÊ‘Ú°£
*	\return ∆•≈‰∑µªÿTRUE,∑Ò‘ÚFALSE
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
	BasicDeallocate(sp);
	return bMatch;
}
#endif

const int	ENT_NOQUOTES	= 0;	//! ≤ª◊™ªªµ•“˝∫≈∫ÕÀ´“˝∫≈
const int	ENT_COMPAT	= 1;		//! ÷ª◊™ªªÀ´“˝∫≈
const int	ENT_SQUOTE	= 2;		//! ÷ª◊™ªªµ•“˝∫≈
const int	ENT_QUOTES	= ENT_COMPAT|ENT_SQUOTE;	//! Õ¨ ±◊™ªªµ•“˝∫≈∫ÕÀ´“˝∫≈
namespace __private

{
	const char*		__get_html_spec_string(char c, int quotestyle);
	const WCHAR*	__get_html_spec_string(WCHAR c, int quotestyle);
}


//! \brief: ∂‘html÷–Ãÿ ‚µƒ◊÷∑˚Ω¯––±‡¬Î°£
/*!
	\param	psz[in]			÷∏œÚ“‘\0Ω·Œ≤µƒ◊÷∑˚¥Æ
	\param	quotestyle[in]	∂‘“˝∫≈µƒ◊™ªªπÊ‘Ú
	\return	◊™ªª∫Ûµƒ◊÷∑˚¥Æ
	\remark ◊™ªªµƒ◊÷∑˚±Ì:
			'&' (ampersand)		=> "&amp;"
			'"'	(double quote)	=> "&quot;"	(√ª”–…Ë÷√ENT_NOQUOTESµƒ«Èøˆœ¬)
			''' (single quote)	=> "&#039;'	(Ωˆµ±…Ë÷√¡ÀENT_QUOTESµƒ«Èøˆœ¬)
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
		else if (((*psz) >= 0 && ((*psz) < 0x20)) || *psz == 127)	// ≤ªø…º˚
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

//! \brief: ∂‘html÷–Ãÿ ‚µƒ◊÷∑˚Ω¯––Ω‚¬Î
/*!
\param	psz[in]			÷∏œÚ“‘\0Ω·Œ≤µƒ◊÷∑˚¥Æ
\return	◊™ªª∫Ûµƒ◊÷∑˚¥Æ
\remark ◊™ªªµƒ◊÷∑˚±Ì:
	'&' (ampersand)		=> "&amp;"
	'"'	(double quote)	=> "&quot;"
	''' (single quote)	=> "&#039;"
	'<' (less than)		=> "&lt;"
	'>'	(greater than)	=> "&gt;"
	∆‰À˚“‘"&#xxx;"∏Ò Ωµƒ◊÷∑˚¥Æ°£
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
//! \brirf: ∂‘◊÷∑˚¥Æ◊ˆurl±‡¬Î
/*!
\param	psz[in]		÷∏œÚ“‘\0Ω·Œ≤µƒ◊÷∑˚¥Æ
\return ◊™ªª∫Ûµƒ◊÷∑˚¥Æ
\remark	
◊™ªªπÊ‘Ú£∫Ω´◊÷∑˚¥Æ÷–≥˝¡À'-'∫Õ'_'Õ‚µƒÀ˘”–∑«◊÷ƒ∏∫Õ∑« ˝◊÷◊÷∑˚∂ºÃÊªª≥…%∫Û∏˙¡ΩŒª Æ¡˘Ω¯÷∆ ˝£¨ø’∏Ò‘Ú±‡¬ÎŒ™º”∫≈(+)°£
∏√∫Ø ˝Ωˆ”–Multi∞Ê±æ£¨√ª”–UNICODE∞Ê±æ°£
*/
char_string	Basic_URLEncode(const char* psz);

//! ∂‘url±‡¬Î∫Ûµƒ◊÷∑˚¥Æ◊ˆΩ‚¬Î
/*!
\param	psz[in]		÷∏œÚ“‘\0Ω·Œ≤µƒ◊÷∑˚¥Æ
\return ◊™ªª∫Ûµƒ◊÷∑˚¥Æ
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
			case CharSecond:	// øœ∂®≤ª «±Í◊ºµƒ,∆’Õ®¥¶¿Ì
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

//! ◊™≥…16Ω¯÷∆µƒ◊÷∑˚¥Æ
int Basic_ConvertStringToHexString(const char *pszSrc, int nCount, char* pszDest, int nDest);

//! Ω´HEX±‡¬Î∫Ûµƒ◊÷∑˚¥Æ◊™ªØŒ™hex‘¥¥Æ
/*!
\param	pszSrc[in]		‘¥◊÷∑˚¥Æ
\param	nCount[in]		pszSrcµƒ≥§∂»
\param	pszDest[out]	◊™ªª∫Û ‰≥ˆ
\param	nDest[out]		 ‰≥ˆµƒpszDestµƒ◊÷∑˚¥Æ≥§∂»(≤ª «ƒ⁄¥Ê≥§∂»)
\return ◊™ªª∫Ûµƒ◊÷∑˚¥Æµƒ≥§∂»
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


//! Ω´HEX±‡¬Î∫Ûµƒ◊÷∑˚¥Æ◊™ªØŒ™hex‘¥¥Æ
/*!
\param	pszSrc[in]		‘¥ ˝æ›
\param	nCount[in]		pszSrcµƒ≥§∂»
\param	pszDest[out]	◊™ªª∫Û ‰≥ˆ
\param	nDest[out]		 ‰≥ˆµƒ◊÷∑˚¥Æ≥§∂»¥Û–°.(CharType*nDestµƒ¥Û–°)
\return ◊™ªª∫ÛBufferµƒ≥§∂»
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

//! ∂‘◊÷∑˚¥Æ±‡¬Î
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

//! ∂‘◊÷∑˚¥ÆΩ‚¬Î
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

const char* Basic_LTrim(const char* str);
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
*\brief __atoi64 ◊÷∑˚¥Æ◊™64Œª’˚ ˝
* 
*\param str 
*\param nLen ƒ¨»œ÷µ -1
*\return  
*/
int64_t __atoi64_s(const char* str, int nLen/* = -1*/);


// “ª–©∂‘”⁄basic_stringµƒ≤Ÿ◊˜
namespace strutil
{
	const char*		__get_blank_string(__private::Type2Type<char>);
	const WCHAR*	__get_blank_string(__private::Type2Type<WCHAR>);

	//! ◊÷∑˚¥Æ±‰¥Û–¥
	template<class StringType>
	void makeupper(StringType& s)
	{
		transform(s.begin(), s.end(), s.begin(), (int(*)(int))toupper);
	}

	//! ◊÷∑˚¥Æ±‰–°–¥
	template<class StringType>
	void makelower(StringType& s)
	{
		transform(s.begin(), s.end(), s.begin(), (int(*)(int))tolower);
	}

	//! »•≥˝◊Û≤‡∞¸∫¨‘⁄lpszTarget÷–µƒ◊÷∑˚
	template<class StringType>
	void ltrim(StringType& s, typename StringType::const_pointer lpszTarget)
	{
		s.erase(0, s.find_first_not_of(lpszTarget));
	}


	//! »•≥˝◊Û≤‡ø’◊÷∑˚
	template<class StringType>
	void ltrim(StringType& s)
	{
		typedef typename StringType::value_type value_type;
		ltrim(s, __get_blank_string(__private::Type2Type<value_type>()));
	}

	//! »•≥˝◊Û≤‡÷∏∂®◊÷∑˚
	template<class StringType>
	void ltrim(StringType& s, typename StringType::value_type cTarget)
	{
		s.erase(0, s.find_first_not_of(cTarget));
	}

	//! »•≥˝”“≤‡∞¸∫¨‘⁄lpszTarget÷–µƒ◊÷∑˚
	template<class StringType>
	void rtrim(StringType& s, typename StringType::const_pointer lpszTarget)
	{
		s.erase(s.find_last_not_of(lpszTarget) + 1);
	}

	//! »•≥˝”“≤‡ø’◊÷∑˚
	template<class StringType>
	void rtrim(StringType& s)
	{
		typedef typename StringType::value_type value_type;
		rtrim(s, __get_blank_string(__private::Type2Type<value_type>()));
	}

	//! »•≥˝”“≤‡÷∏∂®◊÷∑˚
	template<class StringType>
	void rtrim(StringType& s, typename StringType::value_type cTarget)
	{
		s.erase(s.find_last_not_of(cTarget) + 1);
	}

	//! ÃÊªª◊÷∑˚¥Æ
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

	//! ÃÊªª◊÷∑˚
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
