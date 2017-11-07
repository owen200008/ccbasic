/***********************************************************************************************
// 文件名:     linux.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:46:12
// 内容描述:   Linux系统相关定义
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_LINUX_H
#define BASIC_LINUX_H

#if	(defined(__LINUX) || defined(__MAC) || defined(__ANDROID))

#include <unistd.h>

static long _daylight = 0;
static long _timezone = -28800;
#define _time32 time
#define _mktime32 mktime

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

#define _tcscpy_s __tcscpyn
#define strncpy_s __tcscpyn
//
#define _stricmp    strcasecmp
#define stricmp     strcasecmp
#define strnicmp    strncasecmp
#define _strnicmp   strncasecmp
#define _strdup     strdup
#define _stricoll   strcoll
#define _access     access
#define _vsnprintf  vsnprintf
#define _snprintf   snprintf
#define _stprintf_s   snprintf
#define sprintf_s   snprintf
#define _stat       stat
#define _tzset      tzset
#define _utime      utime
#define _utimbuf    utimbuf
#define _getcwd     getcwd
#define _fstat      fstat
#define _chmod      chmod
#define _mkdir      mkdir

#define _localtime32	localtime
#define _mktime32		mktime
#define _mktime64		mktime
#define _time32			time


#define __iscsym(_c)    (isalnum(_c) || ((_c) == '_'))
//#define wcslen        strlen
// #define _strupr      strupr
// #define _strlwr      strlwr
// #define _strrev      strrev
char * __cdecl _strupr (   char * string    );
char * __cdecl _strlwr (   char * string    );
char * __cdecl _strrev (   char * string    );

#ifndef wsprintf
 #define wsprintf   sprintf
#endif
#define wsprintfA   sprintf
//

#ifndef _tstof
#define _tstof atof
#endif

#ifndef _tstoi
#define _tstoi atoi
#endif

#ifndef _tstoi64
#define _tstoi64 _atoi64
#endif
//////////////////////////////////////////////////////////////////////////////////////////////
//for file notify
#define FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001   
#define FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002   
#define FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004   
#define FILE_NOTIFY_CHANGE_SIZE         0x00000008   
#define FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010   
#define FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020   
#define FILE_NOTIFY_CHANGE_CREATION     0x00000040   
#define FILE_NOTIFY_CHANGE_SECURITY     0x00000100   

#define FILE_ACTION_ADDED                   0x00000001   
#define FILE_ACTION_REMOVED                 0x00000002   
#define FILE_ACTION_MODIFIED                0x00000003   
#define FILE_ACTION_RENAMED_OLD_NAME        0x00000004   
#define FILE_ACTION_RENAMED_NEW_NAME        0x00000005   

#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))

//////////////////////////////////////////////////////////////////////////////////////////////
//目前只有ctime使用到，保持和windows一样的方式64位
#ifndef _TIME64_T_DEFINED
typedef LONGLONG __time64_t;     /* 64-bit time value */
#define _TIME64_T_DEFINED
#endif

#include <stdarg.h>
#include <stdlib.h>

#ifdef __GNUC__
#include <functional>
#endif

#endif	// __LINUX __MAC

#endif 
