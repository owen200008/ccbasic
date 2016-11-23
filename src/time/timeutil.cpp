#include "../inc/basic.h"
#include "timelib/timelib.h"

__NS_BASIC_START

time_t	dostime2time(unsigned short dosdate, unsigned short dostime)
{
	struct tm atm;
	atm.tm_sec = (dostime & ~0xFFE0) << 1;
	atm.tm_min = (dostime & ~0xF800) >> 5;
	atm.tm_hour = dostime >> 11;

	atm.tm_mday = dosdate & ~0xFFE0;
	atm.tm_mon = ((dosdate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (dosdate >> 9) + 80;
	atm.tm_isdst = -1;
	return mktime(&atm);
}

ulg dostime(int y, int n, int d, int h, int m, int s)
{
	return y < 1980 ? dostime(1980, 1, 1, 0, 0, 0) : 
		(((ulg)y - 1980) << 25) | ((ulg)n << 21) | ((ulg)d << 16) | 
		((ulg)h << 11) | ((ulg)m << 5) | ((ulg)s >> 1);
}


ulg unix2dostime(time_t *t)
{
	time_t t_even = (time_t)(((unsigned long)(*t) + 1) & (~1));
#ifdef __BASICWINDOWS	
	struct tm *s = localtime(&t_even);
	if (s == (struct tm *)NULL) 
	{
		t_even = (time_t)((time(NULL) + 1) & (~1));
		s = localtime(&t_even);
	}
#else
	struct tm ti;
	struct tm *s = localtime_r(&t_even, &ti);
	if (s == (struct tm *)NULL) 
	{
		t_even = (__time32_t)(((unsigned long)time(NULL) + 1) & (~1));
		s = localtime_r(&t_even, &ti);
	}
#endif	
	return dostime(s->tm_year + 1900, s->tm_mon + 1, s->tm_mday, s->tm_hour, s->tm_min, s->tm_sec);
}

time_t dos2unixtime(ulg dostime)
{
	time_t clock = time(NULL);
	struct tm *t = localtime(&clock);
	t->tm_isdst = -1;
	t->tm_sec  = (((int)dostime) <<  1) & 0x3e;
	t->tm_min  = (((int)dostime) >>  5) & 0x3f;
	t->tm_hour = (((int)dostime) >> 11) & 0x1f;
	t->tm_mday = (int)(dostime >> 16) & 0x1f;
	t->tm_mon  = ((int)(dostime >> 21) & 0x0f) - 1;
	t->tm_year = ((int)(dostime >> 25) & 0x7f) + 80;

	return mktime(t);
}

long strtotime(const char* str, size_t len)
{
    if (len <= 0)
    {
        len = strlen(str);
    }
    timelib_time* parsed_time = timelib_strtotime((char*)str, len, NULL, timelib_builtin_db());
    timelib_update_ts(parsed_time, NULL);
    int error = 0;
    long t = timelib_date_to_int(parsed_time, &error);
    timelib_time_dtor(parsed_time);
    if (0 == error)
    {
        return t;
    }
    return 0;
}

//__time32_t	strtotime(const char* str, size_t len)
//{
//	if (NullLen == len)
//		len = strlen(str);
//	timelib_time* parsed_time = timelib_strtotime((char*)str, len, NULL, timelib_builtin_db());
//	timelib_update_ts(parsed_time, NULL);
//	int error = 0;
//	__time32_t t = timelib_date_to_int(parsed_time, &error);
//	timelib_time_dtor(parsed_time);
//	if (0 == error)
//	{
//		return t;
//	}
//	return 0;
//}

//__time32_t wcstotime(const WCHAR* str, size_t len)
//{
//	char_string strtime = Basic_WideStringToMultiString(str, len);
//	return strtotime(strtime.c_str(), strtime.length());
//}

__NS_BASIC_END

