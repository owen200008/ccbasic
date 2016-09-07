/***********************************************************************************************
// 文件名:     basic.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 8:18:40
// 内容描述:   BASIC库的头文件
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_BASIC_H
#define BASIC_BASIC_H

//系统环境定义
#include "basic_def.h"

#ifdef __BASICWINDOWS
#ifndef _WIN32_WINNT
//为了支持IPv6,尽量兼容xp的处理
#define _WIN32_WINNT	0x0600
#define NTDDI_VERSION	NTDDI_VISTA
#endif
#include <winsock2.h>		  // 必须在 windows.h 前面
#include <WS2tcpip.h>
#endif

//
//数据类型定义
#include "../types/types.h"
#include "../types/basicobj.h"
#include <stdint.h>
//

//Linux系统相关定义
#if	(defined(__LINUX) || defined(__MAC) || defined(__ANDROID))
#include "../types/linux.h"
#include <pthread.h>
#include <limits.h>
#endif

//
//错误定义
#include "error.h"
//

//
//绑定,用std::function替换
//#include "../misc/fastdelegate.h"
//#include "../misc/fastdelegatebind.h"
//
//
//线程定义
#include "../thread/thread.h"
//
//
//同步对象定义
#include "../thread/mt.h"
//
//单态实现
#include "../misc/singleton.h"
//
//DEBUG宏定义
#include "../debug/debug.h"
//
//异常保护,使用开元stacktrace
//#include "../exception/exception.h"
//
#include "../util/functor.h"
//内存管理
#include "../mem/mem.h"
#include "../mem/smartbuffer.h"
#include "../mem/tlstaticbuffer.h"
//
//数据类型相关
#include "../util/strutil/strutil.h"
#include "../util/strutil/charset.h"
#include "../datastruct/basic_string.h"
//#include "../datastruct/stringex.h"
#include "../datastruct/extern.h"
//#include "../datastruct/tllong.h"
//#include "../datastruct/tlvalue.h"


//智能指针
//#include "../misc/tlrefptr.h"
//扩展的数据类型
//#include "../datastruct/key_value.h"
//#include "../datastruct/tlcoll.h"
//屏蔽字库实现
//#include "../datastruct/pbzk.h"

// 容器
//#include "../util/container.h"
//
//系统信息
//#include "../sys/syserror.h"
//#include "../sys/sysinfo.h"
//
//
//文件处理
#include "../file/filedefine.h"
//#include "../file/filenotify.h"
//#include "../file/fileobj.h"
#include "../file/fileman.h"
//#include "../file/ini/inifile.h"

//xtra
//#include "../algorithm/xtra.h"

//CRC16
//#include "../algorithm/crc16.h"
//#include "../algorithm/crc32.h"
//base64
//#include "../algorithm/base64.h"
//
//#include "../algorithm/algorithm.h"
//#include "../algorithm/md5.h"
//#include "../algorithm/aes/aes.h"
//#include "../algorithm/sha1/sha1.h"
//Time
#include "../time/tltime.h"
//#include "../time/timeutil.h"
//#include "../time/scheduletime.h"
//#include "../time/basicTimeDWord.h"
//日志
#include "../log/log.h"
//
//和数学计算相关的函数
//#include "../algorithm/tlmath.h"
#include <math.h>

// 和颜色处理相关的函数
//#include "../graphics/colorutil.h"

//贪婪加载
//#include "../misc/lazy_init.h"

//通讯定义
//#include "../net/net.h"
//#include "../net/networkprotocal.h"
//
//CRC16
//#include "../algorithm/crc16.h"
//RC5
//#include "../algorithm/rc5/rc5.h"

// 压缩
//#include "../algorithm/zip/tlgzip.h"
//#include "../algorithm/zip/tlzipfile.h"
//#include "../algorithm/zip/zlib.h"

// XML
//#include "../util/xml/xmldom.h"
//#include "../util/xml/tldomparser.h"
//#include "../util/xml/xmloutput.h"

//正则表达式
#if !defined(__MAC) //regex not implement
//#include "../util/regex/cgnuregexp.h"
#endif

//dll加载
//#include "../dll/loaddll.h"
//fl
//#include "../flcommu/basicclient.h"
//#include "../flcommu/servertemplate.h"

//sqlite支持
//#include "../sqlite/sqlite3.h"
//#include "../sqlite/sqlite3db.h"

//#include "../mem/bitstream.h"

#endif 
