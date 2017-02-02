#pragma once
#ifndef BASIC_TIMEUTIL_H__
#define BASIC_TIMEUTIL_H__

__NS_BASIC_START

typedef unsigned long  ulg;

_BASIC_DLL_API time_t	dostime2time(unsigned short dosdate, unsigned short dostime);
_BASIC_DLL_API ulg		dostime(int y, int n, int d, int h, int m, int s);
_BASIC_DLL_API ulg		unix2dostime(time_t *t);
_BASIC_DLL_API time_t	dos2unixtime(ulg dostime);

_BASIC_DLL_API long strtotime(const char* str, size_t len);

#define tcstotime wcstotime

__NS_BASIC_END

#endif //
