/***********************************************************************************************
// 文件名:     cppstring.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:18:29
// 内容描述:   对CWBasicString函数的一个c++替换。使用相同的函数名，但通过不同参数来区分。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_CPPSTRING_H
#define BASIC_CPPSTRING_H

#pragma once

#include "../../inc/basic_def.h"
#include "../../types/types.h"
#include <wchar.h>
#include <string.h>
#include "../private.h"


__NS_BASIC_START

// strlen
template<typename char_type>
size_t __tcslen(const char_type *str)
{
	const char_type *eos = str;

	while( *eos++ ) ;

	return( eos - str - 1 );
}

template<typename char_type1, typename char_type2>
const char_type1 *__tcschr(const char_type1* string, char_type2 ch)
{
	// 避免char_type2是个字符串，从而造成运行时出错
//	typename __private::IsString<char_type2>::Result dummy = __private::False();
	typedef typename __private::IsString<char_type2>::Result DummyType;
	DummyType dummy = __private::False();

	while (*string && *string != (char_type1)ch)
		string++;

	if (*string == (char_type1)ch)
		return string;
	return(NULL);
}


template<typename char_type1, typename char_type2>
char_type1 *__tcschr(char_type1* string, char_type2 ch)
{
	return (char_type1*)__tcschr((const char_type1*)string, ch);
}


template<typename char_type1, typename char_type2>
const char_type1 *__tcsstr(const char_type1 *str1, const char_type2 *str2)
{
	char_type1 *cp = (char_type1 *) str1;
	char_type1 *s1;
	char_type2 *s2;

	if ( !*str2 )
		return((char_type1 *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char_type2 *) str2;

		while ( *s1 && *s2 && !((int)*s1-(int)*s2) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}

template<typename char_type>
char_type *__tcsstr(char_type *str1, const char_type *str2)
{
	return (char_type*)__tcsstr((const char_type*)str1, str2);
}

template<typename char_type_dst, typename char_type_src>
char_type_dst *__tcscpy(char_type_dst* dst, const char_type_src* src)
{
	char_type_dst * cp = dst;

	while( *cp++ = (char_type_dst)*src++ )
		;               /* Copy src over dst */
	return( dst );
}


template<typename char_type_dst, typename char_type_src>
char_type_dst *__tcsncpy(char_type_dst* dest, const char_type_src* source, size_t count)
{
	char_type_dst *start = dest;

	while (count && (*dest++ = (char_type_dst)*source++))    /* copy string */
		count--;

	if (count)                              /* pad out with zeroes */
		while (--count)
			*dest++ = (char_type_dst)'\0';

	return(start);
}

template<typename char_type1, typename char_type2>
int __tcscmp(const char_type1 *string1, const char_type2 *string2)
{
	int ret = 0 ;

	while( ! (ret = (int)*(char_type1 *)string1 - (int)*(char_type2 *)string2) && *string2)
		++string1, ++string2;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

template<typename char_type1, typename char_type2>
int __tcsicmp(const char_type1 *string1, const char_type2 *string2)
{
	int f,l;
	do  {
		f = (int)towlower(*string2);
		l = (int)towlower(*string1);
		string2++;
		string1++;
	} while ( (f) && (f == l) );
	return (f - l);
}

long __tcstol(const char *nptr, char **endptr, int base);
long __tcstol(const WCHAR *nptr, WCHAR **endptr, int base);

int __tcsncmp(const char *string1, const char *string2, size_t maxcount);
int __tcsncmp(const WCHAR *string1, const WCHAR *string2, size_t maxcount);


double __atof(const char* str,int nLen = -1);
long __atol(const TCHAR* str,int nLen = -1);

const TCHAR* __bedigit(const TCHAR* psz, int nLen);


__NS_BASIC_END

#endif 

