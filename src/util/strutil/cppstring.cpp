#include "cppstring.h"
#include "../../inc/basic_def.h"
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include "strutil.h"

#pragma warning(disable: 4996)

__NS_BASIC_START

long __tcstol(const char *nptr, char **endptr, int base)
{
	return strtol(nptr, endptr, base);
}

long __tcstol(const WCHAR *nptr, WCHAR **endptr, int base)
{
	return wcstol((const wchar_t*)nptr, (wchar_t**)endptr, base);
}

int __tcscmp(const char *string1, const char *string2)
{
	return strcmp(string1, string2);
}
int __tcscmp(const WCHAR *string1, const WCHAR *string2)
{
//	return wcscmp(string1, string2);
	int ret = 0 ;

	while( ! (ret = *(WCHAR*)string1 - *(WCHAR*)string2) && *string2)
		++string1, ++string2;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

int __tcsncmp(const char *string1, const char *string2, size_t maxcount)
{
	return strncmp(string1, string2, maxcount);
}
int __tcsncmp(const WCHAR *string1, const WCHAR *string2, size_t maxcount)
{
	return wcsncmp((const wchar_t*)string1, (const wchar_t*)string2, maxcount);
}


double __atof(const char* str, int nLen/* = -1*/)
{
	char* pEnd;
	if(nLen <= -1)
	{
		nLen = __tcslen(str);
	}
	char Buff[32];
	memset(Buff, 0, sizeof(Buff));
	__tcscpyn(Buff, sizeof(Buff) - 1, str, nLen);
	Basic_LTrim(Buff);
	return strtod(Buff, &pEnd);
}

long __atol(const char* str,int nLen/* = -1*/)
{
	char Buff[32];
	char* pEnd;
	if(nLen == -1)
	{
		nLen = __tcslen(str);
	}
	memset(Buff, 0, sizeof(Buff));
	__tcscpyn(Buff, sizeof(Buff) - 1, str, nLen);
	Basic_LTrim(Buff);
	return strtol(Buff, &pEnd, 10);
}

const TCHAR* __bedigit(const TCHAR* psz, int nLen)
{
	if(psz == NULL )
	{
		return NULL;
	}
	for(int i = nLen - 1; i >= 0; i--)
	{
		if(!_istdigit(psz[i]))
		{
			return &psz[i + 1];
		}
	}
	return psz;
}

__NS_BASIC_END
