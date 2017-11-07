/***********************************************************************************************
// 文件名:     filedefine.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:42:04
// 内容描述:   定义文件系统的一些结构和错误信息
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_FILEDEFINE_H
#define BASIC_FILEDEFINE_H

#pragma once
#pragma warning(disable : 4996)
#pragma	pack(1)
__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
struct TLFileStatusW;

/////////////////////////////////////////////////////////////////////////////////////////////
//! 文件的状态信息
/*!
*  用在 CWBasicFileObj::GetStatus 函数取得文件状态信息
*/
struct TLFileStatusW
{
	time_t m_ctime;          /*!< 文件的创建日期 */
	time_t m_mtime;          /*!< 文件的最后修改日期 */
	time_t m_atime;          /*!< 文件的最后访问日期 */
	long   m_size;           /*!< 文件长度（字节） */
	BYTE   m_attribute;      /*!< 文件的属性。是枚举 TLFileAttribute 的值 */
	BYTE   m_padding;        /*!< 结构对齐 */
	TCHAR  m_szFullName[MAX_PATH]; /*!< 文件的全路径 */

	TLFileStatusW()
	{
		memset(this, 0, sizeof(TLFileStatusW));
	}

};

struct TLFileStatus
{
	time_t m_ctime;          /*!< 文件的创建日期 */
	time_t m_mtime;          /*!< 文件的最后修改日期 */
	time_t m_atime;          /*!< 文件的最后访问日期 */
	long   m_size;           /*!< 文件长度（字节） */
	BYTE   m_attribute;      /*!< 文件的属性。是枚举 TLFileAttribute 的值 */
	BYTE   m_padding;        /*!< 结构对齐 */
	char  m_szFullName[MAX_PATH]; /*!< 文件的全路径 */

	TLFileStatus()
	{
		memset(this, 0, sizeof(TLFileStatus));
	}

};

//! 文件的属性
enum TLFileAttribute 
{
	_basic_fa_normal =    0x00,		/*!< 正常 */
	_basic_fa_readOnly =  0x01,		/*!< 只读 */
	_basic_fa_hidden =    0x02,		/*!< 隐藏 */
	_basic_fa_sys    =    0x04,		/*!< 系统 */
	_basic_fa_volume =    0x08,		/*!<  */
	_basic_fa_directory = 0x10,		/*!< 目录 */
	_basic_fa_archive =   0x20		/*!< 归档 */
};

//seek
#define BASIC_FILE_BEGIN           0				//文件头
#define BASIC_FILE_CURRENT         1				//当前位置
#define BASIC_FILE_END             2				//文件尾

//////////////////////////////////////////////////////////////////////////////
//返回错误定义
#define FILE_ERROR							(0xE0000000 | _ERROR_FILE)	    //文件错误
#define FATFILE_ERROR						(0xF0000000 | _ERROR_FILE)		//FATFILE文件错误

#define BASIC_FILE_OK						0								//成功，没有错误

#define BASIC_FILE_GENERIC_ERROR			(FILE_ERROR | 0x0001)			//一般性错误，即未知的错误
#define BASIC_FILE_NOT_FOUND				(FILE_ERROR | 0x0002)			//文件不存在
#define BASIC_FILE_BAD_PATH					(FILE_ERROR | 0x0003)			//路径不合法
#define BASIC_FILE_TOO_MANY_OPEN			(FILE_ERROR | 0x0004)			//打开的文件太多
#define BASIC_FILE_ACCESS_DENIED			(FILE_ERROR | 0x0005)			//禁止访问
#define BASIC_FILE_INVALID					(FILE_ERROR | 0x0006)			//文件不合法
#define BASIC_FILE_REMOVE_CUR_DIR			(FILE_ERROR | 0x0007)			//删除当前的目录
#define BASIC_FILE_DIR_FULL					(FILE_ERROR | 0x0008)			//目录满了
#define BASIC_FILE_BAD_SEEK					(FILE_ERROR | 0x0009)			//移动文件指针失败
#define BASIC_FILE_HARD_IO					(FILE_ERROR | 0x000a)			//IO 错误
#define BASIC_FILE_SHARING_VIOLATION		(FILE_ERROR | 0x000b)			//违反共享
#define BASIC_FILE_LOCK_VIOLATION			(FILE_ERROR | 0x000c)			//违反加锁
#define BASIC_FILE_DISK_FULL				(FILE_ERROR | 0x000d)			//磁盘满
#define BASIC_FILE_END_OF_FILE				(FILE_ERROR | 0x000e)			//文件结束
#define BASIC_FILE_BAD_NET_PATH				(FILE_ERROR | 0x000f)			//网络路径不合法
#define BASIC_FILE_ALREADY_EXISTS			(FILE_ERROR | 0x0010)			//文件已存在
#define BASIC_FILE_NET_ACCESS_DENIED		(FILE_ERROR | 0x0011)			//网络访问错误
#define BASIC_FILE_WRITE_FAULT				(FILE_ERROR | 0x0012)			//写文件错误
#define BASIC_FILE_NET_WRITE_FAULT			(FILE_ERROR | 0x0013)			//写网络文件错误
#define BASIC_FILE_NOT_SUPPORT				(FILE_ERROR | 0x0014)			//不支持
#define BASIC_FILE_TOO_LARGE				(FILE_ERROR | 0x0015)			//文件太大，只支持 2G 
#define BASIC_FILE_ALREADY_OEPN				(FILE_ERROR | 0x0016)			//文件已经打开
#define BASIC_FILE_NOT_OEPN					(FILE_ERROR | 0x0017)			//文件没有打开
#define BASIC_FILE_WRITE_DENIED				(FILE_ERROR | 0x0018)			//文件禁止写，以只读方式打开，不能写和改变文件长度
#define BASIC_FILE_SETLEN_DENIED			(FILE_ERROR | 0x0019)			//文件禁止改变长度。内存文件，以PF_MEM_ONLY方式打开。
#define BASIC_FILE_NO_BUFFER				(FILE_ERROR | 0x001a)			//没有合法的缓冲区
#define BASIC_FILE_MKDIR_ERROR				(FILE_ERROR | 0x001b)			//创建目录失败
#define BASIC_FILE_NO_MEMORY				(FILE_ERROR | 0x001c)			//内存不足

#define BASIC_FILE_BAD_PARAM				(FILE_ERROR | 0x001d)			//函数调用参数错误
#define BASIC_FILE_BAD_RESOURCE				(FILE_ERROR | 0x001e)			//和资源相关的错误
#define BASIC_FILE_BAD_PROCESS				(FILE_ERROR | 0x001f)			//和进程、函数调用、子进程相关的错误
#define BASIC_FILE_NO_EXEC					(FILE_ERROR | 0x0020)			//无法执行
#define BASIC_FILE_BAD_PIPE					(FILE_ERROR | 0x0021)			//和管道相关的错误

__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack()
#endif 
