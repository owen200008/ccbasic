/***********************************************************************************************
// 文件名:     tlzipfile.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012-2-22 23:02:48
// 内容描述:   支持zip格式数据的读写
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_TLZIPFILE_H
#define BASIC_TLZIPFILE_H

#include "../../inc/basic.h"

__NS_BASIC_START

typedef unsigned int	uint32;
typedef unsigned short	uint16;

const uint32 LOCALHEADERMAGIC    = 0x04034b50;
const uint32 CENTRALHEADERMAGIC  = 0x02014b50;
const uint32 ENDHEADERMAGIC      = 0x06054b50;

// zip结构的定义
#pragma pack(1)
struct	zip_file_data_block
{
	uint32		header_magic;	// LOCALHEADERMAGIC
	uint16		version_unzip;	// 解压所需的pkware版本
	uint16		bit_flag;		// 全局方式位标记 ?
	uint16		zip_type;		// 压缩方式
	uint16		modify_time;	// dos格式的最后修改时间
	uint16		modify_date;	// dos格式的最后修改日期
	uint32		crc;			// crc32校验
	uint32		size_zipped;	// 压缩后尺寸
	uint32		size_unzipped;	// 未压缩尺寸
	uint16		size_filename;	// 文件名长度
	uint16		size_extend;	// 扩展记录长度
	// 文件名
	// 扩展
	// 内容

	zip_file_data_block()
	{
		memset(this, 0, sizeof(zip_file_data_block));
		header_magic = LOCALHEADERMAGIC;
		version_unzip = 0x0A;
	}
};

struct	zip_file_dir_block
{
	uint32		header_magic;			// CENTRALHEADERMAGIC
	uint16		version_zip;			// 压缩使用的pkware版本
	uint16		version_unzip;			// 解压所需的pkware版本
	uint16		bit_flag;				// 全局方式位标记
	uint16		zip_type;				// 压缩方式
	uint16		modify_time;			// dos格式的最后修改时间
	uint16		modify_date;			// dos格式的最后修改日期
	uint32		crc;					// crc32校验
	uint32		size_zipped;			// 压缩后尺寸
	uint32		size_unzipped;			// 未压缩尺寸
	uint16		size_filename;			// 文件名长度
	uint16		size_extend;			// 扩展记录长度
	uint16		size_comment;			// 文件注释长度
	uint16		disk_num_start;			// 磁盘开始号
	uint16		property_internal_file;	// 内部文件属性
	uint32		property_outer_file;	// 外部文件属性
	uint32		local_header_offset;	// 局部头部偏移量
	// 文件名
	// 扩展字段
	// 文件注释
	zip_file_dir_block()
	{
		memset(this, 0, sizeof(zip_file_dir_block));
		header_magic = CENTRALHEADERMAGIC;
		version_zip		= 0x14;
		version_unzip	= 0x0A;;
	}
};

struct _BASIC_DLL_API zip_file_end
{
	uint32		header_magic;				// ENDHEADERMAGIC
	uint16		disk_num_cur;				// 当前磁盘编号
	uint16		disk_num_dir_block_start;	// 目录区开始磁盘编号
	uint16		file_num_cur_disk;			// 本磁盘上记录总数
	uint16		file_num_total;				// 目录区中记录总数
	uint32		size_dir;					// 目录区尺寸大小
	uint32		offset_dir_to_first_disk;	// 目录区对第一张磁盘的偏移量
	uint16		size_comment;				// zip文件注释长度

	// 注释
	zip_file_end()
	{
		memset(this, 0, sizeof(zip_file_end));
		header_magic = ENDHEADERMAGIC;
	}
};
#pragma pack()


const uint32	property_outer_file_dir		= 16;
const uint32	property_outer_file_file	= 32;

//----------------------------------8<---------------------------------
// 以下用于描述读取写入压缩文件后内存中保存的数据结构

// 基本结构
struct zip_base_file_info
{
	char_string	file_name;		// zip文件中的文件名
	char_string	comment;		// 注释
	time_t	time_lastmodify;	// 最后修改时间
	uint16	zip_type;			// 压缩方式

	zip_base_file_info()
	{
		time_lastmodify = zip_type = 0;
	}

	virtual ~zip_base_file_info(){}
	virtual bool add_file(zip_base_file_info* file)
	{
		return false;
	}
	struct is_equal_filename
	{
		is_equal_filename(const char* filename, uint16 len) : __filename(filename), __len(len){}
		bool operator()(zip_base_file_info* info)
		{
			const char* nakename = strrchr(info->file_name.c_str(), '/');
			if (nakename)
				++ nakename;
			else
				nakename = info->file_name.c_str();
			return strncmp(nakename, __filename, __len) == 0 ;
		}
		const char* __filename;
		uint16		__len;
	};

};

//typedef list<zip_base_file_info*>	file_container;
typedef basic_map<char_string, zip_base_file_info*>		file_container;

// 从zip文件中读出的文件信息
struct zip_file_info : public zip_base_file_info
{
	uint32	size_zipped;		// 压缩后文件大小
	uint32	size_unzipped;		// 未压缩的文件大小
	uint32	offset_data_block;	// 数据块偏移
	uint32	offset_dir_block;	// 索引块偏移
	uint32	crc;				// crc校验和

	zip_file_info()
	{
		size_zipped = size_unzipped = offset_data_block = offset_dir_block = 0;
	}
	virtual ~zip_file_info(){}
};

// 目录信息
struct zip_dir_info : public zip_base_file_info
{
	file_container	sub_files;	// 文件列表

	virtual ~zip_dir_info()
	{
		for(file_container::iterator iter = sub_files.begin(); iter != sub_files.end(); ++ iter)
		{
			BASIC_DeleteObject(iter->second);
		}
		sub_files.clear();
	}

	virtual bool add_file(zip_base_file_info* file)
	{
		//sub_files.push_back(file);
		const char* nakename = strrchr(file->file_name.c_str(), '/');
		if (NULL == nakename)
			nakename = file->file_name.c_str();
		else
			++ nakename;

		sub_files[nakename] = file;
		return true;
	}
};

class zipfile_exception : public exception
{
public:
	enum zip_error_code
	{
		noError,
		generic,
		streamEnd,
		needDict,
		errNo,
		streamError,
		dataError,
		memError,
		bufError,
		versionError,
		badFuncParam,
		badZipFile,
		badCrc,
	};
	zipfile_exception(zip_error_code code);
	virtual ~zipfile_exception() throw();
	virtual const char* what() const throw();

protected:
	virtual void _Doraise() const;
	zip_error_code	__errcode;
	char_string		__error;
};

#define	__ERROR_ZIPFILE		0x00001000
#define ZIPFILE_ERROR	(0x80000000 | _ERROR_FILE | __ERROR_ZIPFILE)

#define BASIC_ZIPFILE_GENERIC		(ZIPFILE_ERROR|0x0001)
#define BASIC_ZIPFILE_STREAMEND	(ZIPFILE_ERROR|0x0002)
#define BASIC_ZIPFILE_NEEDDICT		(ZIPFILE_ERROR|0x0003)
#define BASIC_ZIPFILE_ERRNO		(ZIPFILE_ERROR|0x0004)
#define BASIC_ZIPFILE_STREAMERROR	(ZIPFILE_ERROR|0x0005)
#define BASIC_ZIPFILE_DATAERROR	(ZIPFILE_ERROR|0x0006)
#define BASIC_ZIPFILE_MEMERROR		(ZIPFILE_ERROR|0x0007)
#define BASIC_ZIPFILE_BUFERROR		(ZIPFILE_ERROR|0x0008)
#define BASIC_ZIPFILE_VERSIONERROR	(ZIPFILE_ERROR|0x0009)
#define BASIC_ZIPFILE_BADFUNCPARAM	(ZIPFILE_ERROR|0x000A)
#define BASIC_ZIPFILE_BADZIPFILE	(ZIPFILE_ERROR|0x000B)
#define BASIC_ZIPFILE_BADCRC		(ZIPFILE_ERROR|0x000C)


struct _BASIC_DLL_API TLPackFileStatus
{
	time_t			m_mtime;
	uint32			m_zipped;		/*!< 压缩后大小 */
	uint32			m_unzipped;		/*!< 压缩前大小 */
	BYTE			m_attribute;     /*!< 文件的属性。是枚举 TLFileAttribute 的值 */
	char_string		m_comment;
	char_string		m_filename;
	uint32			m_crc;		    /*!< crc校验和 */

	TLPackFileStatus();
	bool IsDirectory() const;
	bool IsFile() const;
};

class _BASIC_DLL_API CBasicPackFileVisitor
{
public:
	//! breif 如果返回false,则停止遍历
	virtual bool	Visit(const TLPackFileStatus& status) = 0;
};

#pragma warning (push)
#pragma warning (disable: 4251)

class _BASIC_DLL_API CBasicZipFile : public CBasicFileObj
{
public:
	CBasicZipFile(void);
	virtual ~CBasicZipFile(void);
public:
	// 文件操作
	//! \brief 打开一个zip文件。这个操作过程中应该已经读取所有的文件索引
	virtual long	OpenZipFile(const char* lpszFileName, DWORD dwOpenFlags);
	//! \brief 关闭
	virtual void	Close();
	//! \breif 写盘
	virtual long	Flush();

	//!子文件的操作
	//! \brief 返回内存文件格式的CWBasicFileObj对象
	CBasicFileObj*	GetPackFile(const char* filename);
	//! \brief 得到文件信息
	bool	GetPackFileStatus(const char* filename, TLPackFileStatus& status);
	//! \brief 释放CWBasicFileObj对象,fileobj由GetFile得到
	void	ReleasePackFile(CBasicFileObj* fileobj);


	//! \brief 增加一个文件,如果dir为空，则直接取fileobj的文件名.
	// 如果该文件名已经存在则替换已经存在的文件。
	bool	AddPackFile(CBasicStaticBuffer* buffer, const char* filename , const char* comment = NULL);

	//! \brief 增加一个文件,如果dir为空，则直接取fileobj的文件名.
	// 如果该文件名已经存在则替换已经存在的文件。
	// 不使用CWBasicStaticBuffer的版本
	bool	AddPackFile(const void* pBuffer, size_t length, const char* filename , const char* comment = NULL);

	//! \brief 删除一个文件
	bool	DeletePackFile(const char* filepath);
	//! \brief 遍历文件名
	void	VisitPackFile(CBasicPackFileVisitor* pVisitor, const char* filespec, bool bSubdir = true);

	//! zip操作
	// 其它zip文件的操作
	//! \brief 增加本地目录上的文件到压缩包。filename如果是目录则将目录下的文件也加入压缩包
	bool	AddPackFile(const char* filename, const char* path = 0, const char* comment = 0);
	bool	AddPathToFile(const char* path);
	//! \breif 解压到目录
	bool	ExtractTo(const char* path);
	//! \brief 设置文件注释
	bool	SetComment(const char* comment);
	//! \brief 读取注释
	const char*	GetComment() const;

	//! \brief 设置压缩的level
	void	SetZipLevel(int level);

	uint16	GetFileNumTotal() const {return m_end.file_num_total;}
protected:
	virtual long	buildZipIndex();
	CBasicFileObj*		getFileObj(const zip_file_info* info);
	bool			extractToFile(const char* dir, const zip_base_file_info* info);
	bool			setFileTime(const char* filename, time_t mtime);
		
	zip_file_info*	__addfile(CBasicStaticBuffer* dataBuffer, file_container* dir, CBasicSmartBuffer& bufIndex, zip_file_info* fileinfo, const char* filepath);
	zip_dir_info*	__addpath(file_container* dir, CBasicSmartBuffer& bufIndex, zip_dir_info* dirinfo, const char* filepath);
	bool			__adddir(const char* dirname, CBasicSmartBuffer& bufIndex, char_string path, file_container* dir);
	void			removeIndexFromDirBuffer(CBasicSmartBuffer& bufIndex, zip_file_info* fileinfo);
	bool			repairFile();

	static	zip_base_file_info*	getFileInfo(const char* filename, file_container* dir);
	static	zip_base_file_info* deleteFileInfo(const char* file, file_container* dir);
	static	void		getFileStatus(const zip_base_file_info* info, TLPackFileStatus& status);
	static	bool		addFile(const char *filename, zip_base_file_info* file, file_container* dir);
	static  char_string getRootPath(const char* fileame);
	static  const char* getFileName(const char* filename);
	static  const char* stepPath(const char* filename);



	class VisitorSpecFile;
	class ZipExtractor;
	class ShortenOffset;
	friend class ZipExtractor;
protected:
	file_container	m_Files;
	char_string		m_strComment;
	int				m_ziplevel;
	zip_file_end	m_end;
	bool			m_bNeedRepair;
};

#pragma warning (pop)

__NS_BASIC_END

#endif 

