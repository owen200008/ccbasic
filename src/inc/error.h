#ifndef BASIC_ERROR_H
#define BASIC_ERROR_H

//basic项目错误代码定义
//高16位定义为模块分类，低16位定义为模块内部错误号

//系统类错误
#define _ERROR_SYS				0x00010000
//

//文件类错误
#define _ERROR_FILE				0x00020000
//

//内存类错误
#define _ERROR_MEM				0x00030000
//
//
//线程类错误
#define _ERROR_THREAD			0x00050000
#define _ERROR_THREAD_INVALID	(_ERROR_THREAD|1)		//无效线程
//

//
//日志类错误
#define _ERROR_LOG				0x00060000
#define _ERROR_LOG_OPEN			(_ERROR_LOG|1)			//打开日志失败
//

//网络类错误
#define _ERROR_NET				0x00070000

//XML配置文件类错误
#define _ERROR_XCON				0x00080000

//串行化的错误代码
#define _ERROR_ARC				0x00090000

//树的错误代码
#define _ERROR_TREE				0x000A0000

#endif //BASIC_ERROR_H
