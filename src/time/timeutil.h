#pragma once
#ifndef BASIC_TIMEUTIL_H__
#define BASIC_TIMEUTIL_H__

__NS_BASIC_START

typedef unsigned long  ulg;

time_t	dostime2time(unsigned short dosdate, unsigned short dostime);
ulg		dostime(int y, int n, int d, int h, int m, int s);
ulg		unix2dostime(time_t *t);
time_t	dos2unixtime(ulg dostime);

long strtotime(const char* str, size_t len);

#define tcstotime wcstotime

__NS_BASIC_END

#endif //
