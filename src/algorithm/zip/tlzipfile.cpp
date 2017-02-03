#include "../../inc/basic.h"
#include <iostream>
using namespace std;
using namespace basiclib;
// functor to print dir tree
struct print_tree
{
	print_tree(file_container* container, int deep) : __container(container), __deep(deep){}

	int	operator()(const pair<char_string, zip_base_file_info*>& p)
	{
		zip_base_file_info* info = p.second;
		cout.width(4 * __deep + 2);
		cout.fill(' ');
		cout<<"+-";
		cout.width(0);
		cout<<p.first<<endl;

		zip_dir_info* dir = dynamic_cast<zip_dir_info*>(info);
		if (dir)
		{
			for_each(dir->sub_files.begin(), dir->sub_files.end(), print_tree(&(dir->sub_files), __deep + 1));
		}
		return 0;
	}

protected:
	file_container*	__container;
	int				__deep;
};


__NS_BASIC_START

zipfile_exception::zipfile_exception(zip_error_code code)
	: exception(), __errcode(code)
{
	switch(code)
	{
	case noError:
		__error = "no error";
		break;
	case generic:
		__error = "generic error";
		break;
	case streamEnd:
		__error = "stream end";
		break;
	case needDict:
		__error = "need dictionary";
	case errNo:
		__error = "error no";
		break;
	case streamError:
		__error = "error stream error";
		break;
	case dataError:
		__error = "data error";
		break;
	case memError:
		__error = "memory error";
		break;
	case bufError:
		__error = "buffer error";
		break;
	case versionError:
		__error = "version error";
		break;
	case badFuncParam:
		__error = "bad function param";
		break;
	case badZipFile:
		__error = "bad zip file";
		break;
	case badCrc:
		__error = "crc error";
		break;
	default:
		__error = "unknown error";
		break;
	}
}

void zipfile_exception::_Doraise() const
{
	throw (*this);
}

zipfile_exception::~zipfile_exception() throw()
{}

const char* zipfile_exception::what() const throw()
{
	return __error.c_str();
}


TLPackFileStatus::TLPackFileStatus()
{
	m_mtime = 0;
	m_zipped = m_unzipped = m_attribute = 0;
}

bool TLPackFileStatus::IsDirectory() const
{
	return _basic_fa_directory == (m_attribute & _basic_fa_directory);
}
bool TLPackFileStatus::IsFile() const
{
	return 0 == (m_attribute & _basic_fa_directory);
}
/////////////////////////////////////////////////////////////////////////////
// CBasicZipFile
CBasicZipFile::CBasicZipFile(void)
{
	m_ziplevel = 9;
	m_bNeedRepair = false;
}

CBasicZipFile::~CBasicZipFile(void)
{
}

void CBasicZipFile::SetZipLevel(int level)
{
	m_ziplevel = level;
}

long CBasicZipFile::OpenZipFile(const char* lpszFileName, DWORD dwOpenFlags)
{
	long lError = CBasicFileObj::Open(lpszFileName, dwOpenFlags);
	if (BASIC_FILE_OK != lError)
		return lError;

	return buildZipIndex();
}

void CBasicZipFile::Close()
{
	for(file_container::iterator iter = m_Files.begin(); iter != m_Files.end(); ++ iter)
	{
		BASIC_DeleteObject(iter->second);
	}
	m_Files.clear();

	if (m_bNeedRepair)
		repairFile();
	else
		CBasicFileObj::Close();
}

long CBasicZipFile::Flush()
{
	return CBasicFileObj::Flush();
}

long CBasicZipFile::buildZipIndex()
{
	long file_length = GetLength();
	if (file_length < 0)	// 发生错误
		return file_length;

	// 判断文件长度是否合法
	if (file_length < sizeof(zip_file_end))
	{
		if (!IsReadOnly() && file_length == 0)
		{	// 如果是写文件，允许文件长度为0
			return BASIC_FILE_OK;
		}
		else
		{
			Close();
			return BASIC_ZIPFILE_BADZIPFILE;
		}
	}

	uint16 buffer_size = 65535;	// uint16
	if (file_length < buffer_size)
		buffer_size = (uint16)file_length - sizeof(zip_file_data_block) - sizeof(zip_file_dir_block);

	long remain_length = file_length - buffer_size;
	Seek( -(long)buffer_size, BASIC_FILE_END);
	
	CBasicStaticBuffer buffer;
	char* end = (char*)buffer.Alloc(buffer_size);
	Read(end, buffer_size);

	char* flag = end + buffer_size - sizeof(zip_file_end) + 1;

	while(end <= flag && *(uint32*)flag != ENDHEADERMAGIC)
		-- flag;
	if (flag < end)
	{
		Close();
		return BASIC_ZIPFILE_BADZIPFILE;
	}
	
	zip_file_end* file_end = (zip_file_end*)flag;
	m_end = *file_end;
	m_strComment = char_string((char*)(file_end + 1), file_end->size_comment);
	// 偏移到目录区
	Seek(file_end->offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	CBasicStaticBuffer sb;
	uint32 extend_length = 0;
	zip_file_dir_block	dir_block;
	uint32 diroffset = 0;
	for (int i = 0; i < file_end->file_num_total; ++ i)
	{
		Read(&dir_block, sizeof(zip_file_dir_block));
		
		if (dir_block.header_magic != CENTRALHEADERMAGIC)
		{
			Close();
			return BASIC_ZIPFILE_BADZIPFILE;
		}
		extend_length = dir_block.size_filename + dir_block.size_extend + dir_block.size_comment;
		char* file = (char*)sb.Alloc(extend_length);
		Read(file, extend_length);
		

		time_t lastmodify = dostime2time(dir_block.modify_date, dir_block.modify_time);

		char* extend = file + dir_block.size_filename;
		char* comment = extend + dir_block.size_extend;
		
		if (dir_block.property_outer_file & property_outer_file_dir)	// is a directory
		{	// 目录
			zip_dir_info* info = Basic_NewObject<zip_dir_info>();
			info->file_name = char_string(file, dir_block.size_filename - 1);	// 目录去除掉最后一个'/'
			info->comment = char_string(comment, dir_block.size_comment);
			info->time_lastmodify = lastmodify;
			info->zip_type = dir_block.zip_type;
			if (!addFile(info->file_name.c_str(), info, &m_Files))
				BASIC_DeleteObject(info);
		}
		else
		{	// 文件
			zip_file_info* info = Basic_NewObject<zip_file_info>();
			info->file_name = char_string(file, dir_block.size_filename);
			info->comment = char_string(comment, dir_block.size_comment);
			info->time_lastmodify = lastmodify;
			info->zip_type = dir_block.zip_type;
			info->size_zipped = dir_block.size_zipped;
			info->size_unzipped = dir_block.size_unzipped;
			info->offset_data_block = dir_block.local_header_offset;
			info->offset_dir_block = diroffset;
			info->crc = dir_block.crc;

			if (!addFile(info->file_name.c_str(), info, &m_Files))
				BASIC_DeleteObject(info);
		}
		diroffset += sizeof(zip_file_dir_block) + extend_length;
	}

	return BASIC_FILE_OK;
}

bool CBasicZipFile::addFile(const char *filename, zip_base_file_info* file, file_container* dir)
{
	const char* sub = strchr(filename, '/');
	if (sub)	// is dir
	{
		if (*(++sub) != 0)	// not end
		{
			char_string key(filename, sub - filename - 1);
			file_container::iterator iter = dir->lower_bound(key);
			if (iter != dir->end() && iter->first == key)
			{
				zip_dir_info* subfile = dynamic_cast<zip_dir_info*>(iter->second);
				if (subfile)
					return addFile(sub, file, &subfile->sub_files);
				else
					return false;
			}
			else
			{
				return false;
			}
		}
	}
	(*dir)[filename] = file;
	return true;
}

zip_base_file_info*	CBasicZipFile::getFileInfo(const char* filename, file_container* dir)
{
	char_string key;
	const char* sub = strchr(filename, '/');
	if (sub)	// is dir
	{
		key = char_string(filename, sub - filename);
	}
	else
	{
		key = filename;
	}

	zip_base_file_info* info = NULL;
	file_container::iterator iter = dir->lower_bound(key);
	if (iter != dir->end() && iter->first == key)
	{
		if (sub && *(++sub) != '\0')
		{
			zip_dir_info* subfile = dynamic_cast<zip_dir_info*>(iter->second);
			if (subfile)
				info = getFileInfo(sub, &subfile->sub_files);
		}
		else
		{
			info = iter->second;
		}
	}
	return info;
}

CBasicFileObj* CBasicZipFile::getFileObj(const zip_file_info* info)
{
	Seek(info->offset_data_block, BASIC_FILE_BEGIN);

	zip_file_data_block file_block;
	if (sizeof(zip_file_data_block) != Read(&file_block, sizeof(zip_file_data_block))
		|| file_block.header_magic != LOCALHEADERMAGIC)
		return NULL;

	CBasicFileObj* pFileObj = NULL;
	do 
	{
		if (info->size_zipped > 0)
		{
			CBasicStaticBuffer reader;
			char* zipped = (char*)reader.Alloc(info->size_zipped);
			Seek(file_block.size_filename + file_block.size_extend,	BASIC_FILE_CURRENT);
			if (info->size_zipped != Read(zipped, info->size_zipped))
				return NULL;

			if (info->zip_type != 0)
			{
				CBasicStaticBuffer buffer;
				char* unzipped = (char*)buffer.Alloc(file_block.size_unzipped);

				int err;
				z_stream stream;

				stream.next_in = (Bytef*)zipped;
				stream.avail_in = (uInt)file_block.size_zipped;

				stream.next_out = (Bytef*)unzipped;
				stream.avail_out = (uInt)file_block.size_unzipped;

				stream.zalloc = (alloc_func)0;
				stream.zfree = (free_func)0;
				stream.opaque = (voidpf)0;

				err = inflateInit2(&stream, -MAX_WBITS);
				if (err != Z_OK) 
				{
					return NULL;
				}

				err = inflate(&stream, Z_FINISH);
				if (err != Z_STREAM_END)
				{
					inflateEnd(&stream);
					if (err != Z_OK)
					{
						return NULL;
					}
				}
				else
					err = inflateEnd(&stream);

				uLong crc = crc32(0, (Bytef*)unzipped, file_block.size_unzipped);
				if (crc != file_block.crc)
				{
					return NULL;
				}

				pFileObj = new CBasicFileObj((void*)unzipped, file_block.size_unzipped);
			}
			else
			{
				pFileObj = new CBasicFileObj((void*)zipped, file_block.size_zipped);
			}
		}
		else
		{
			pFileObj = new CBasicFileObj((void*)NULL, 0);
		}
	}while(false);
	return pFileObj;
}

void CBasicZipFile::getFileStatus(const zip_base_file_info* info, TLPackFileStatus& status)
{
	status.m_filename	= info->file_name;
	status.m_mtime		= info->time_lastmodify;
	status.m_comment	= info->comment;
	const zip_file_info* fileinfo = dynamic_cast<const zip_file_info*>(info);
	if (fileinfo)
	{
		status.m_zipped		= fileinfo->size_zipped;
		status.m_unzipped	= fileinfo->size_unzipped;
		status.m_crc		= fileinfo->crc;
	}
	else
	{
		status.m_attribute = _basic_fa_directory;
	}
}

CBasicFileObj*	CBasicZipFile::GetPackFile(const char* filename)
{
	zip_file_info* fileinfo = dynamic_cast<zip_file_info*>(getFileInfo(filename, &m_Files));
	if (fileinfo)
	{
		return getFileObj(fileinfo);
	}
	return NULL;
}

bool CBasicZipFile::GetPackFileStatus(const char* filename, TLPackFileStatus& status)
{
	zip_base_file_info* info = getFileInfo(filename, &m_Files);
	if (NULL == info)
		return false;
	
	getFileStatus(info, status);
	return true;
}

class CBasicZipFile::ZipExtractor
{
public:
	ZipExtractor(const char* path, CBasicZipFile* file) : __path(path), __file(file)
	{}

	bool operator()(const pair<char_string, zip_base_file_info*>& p)
	{
		return __file->extractToFile(__path, p.second);
	}

protected:
	const char*			__path;
	CBasicZipFile*			__file;
};

bool CBasicZipFile::ExtractTo(const char* path)
{
	const char* dir = path;

	for_each(m_Files.begin(), m_Files.end(), ZipExtractor(dir, this));
	return true;
}

bool CBasicZipFile::extractToFile(const char* path, const zip_base_file_info* info)
{
	char_string filename = path + info->file_name;
	const zip_file_info* file = dynamic_cast<const zip_file_info*>(info);
	const zip_dir_info* dir = dynamic_cast<const zip_dir_info*>(info);
	if (file)
	{
		CBasicFileObj* pFile = getFileObj(file);
		if (pFile)
		{
			const char* fullname = filename.c_str();
			Basic_mkdir(fullname);
			if (!pFile->IsOpen())
			{
				CBasicFileObj f;
				f.Open(fullname, PF_DISK_FILE|PF_CREATE_ONLY);
			}
			else
			{
				pFile->CopyTo(fullname);
			}
			ReleasePackFile(pFile);
			setFileTime(fullname, info->time_lastmodify);
		}
	}
	else if (dir)
	{
		filename += '/';

		const char* fullname = filename.c_str();
		Basic_mkdir(fullname);
		setFileTime(fullname, info->time_lastmodify);
		for_each(dir->sub_files.begin(), dir->sub_files.end(), ZipExtractor(path, this));
	}
	return true;
}

bool CBasicZipFile::setFileTime(const char* filename, time_t mtime)
{
	TLFileStatus status;
	if (BASIC_FILE_OK == Basic_GetFileStatus(filename, status))
	{
		status.m_mtime = mtime;
		return BASIC_FILE_OK == Basic_SetFileStatus(filename, status);
	}
	return false;
}

void CBasicZipFile::ReleasePackFile(CBasicFileObj* fileobj)
{
	delete fileobj;
}

bool CBasicZipFile::SetComment(const char* comment)
{
	if (IsReadOnly())
		return false;
	m_strComment = comment;
	m_end.size_comment = m_strComment.length();

	Seek(m_end.offset_dir_to_first_disk + m_end.size_dir, BASIC_FILE_BEGIN);
	Write(&m_end, sizeof(zip_file_end));
	Write(comment, m_end.size_comment);
	return true;
}

const char*	CBasicZipFile::GetComment() const
{
	return m_strComment.c_str();
}

class CBasicZipFile::VisitorSpecFile
{
public:
	typedef pair<char_string, zip_base_file_info*> argument_type;
	VisitorSpecFile(CBasicPackFileVisitor* visitor, const char* spec, bool subdir)
		: __visitor(visitor), __spec(spec), __subdir(subdir)
	{}

	bool operator()(const pair<char_string, zip_base_file_info*>& p) const
	{	
		zip_dir_info* dir = dynamic_cast<zip_dir_info*>(p.second);
		if (dir)
		{
			if (__subdir)
			{
				return (dir->sub_files.end() == find_if(dir->sub_files.begin(), dir->sub_files.end(), std::not1(*this)));
			}
		}

		const char* filename = strrchr(p.second->file_name.c_str(), '/');
		filename = (NULL == filename) ? p.second->file_name.c_str() : filename + 1;
		if (Basic_StringMatchSpec(filename, __spec))
		{
			TLPackFileStatus status;
			CBasicZipFile::getFileStatus(p.second, status);
			return __visitor->Visit(status);
		}			

		return true;
	}
protected:
	CBasicPackFileVisitor*	__visitor;
	const char*			__spec;
	bool				__subdir;
};

void CBasicZipFile::VisitPackFile(CBasicPackFileVisitor* pVisitor, const char* filespec, bool bSubdir)
{
	file_container* container = NULL;
	while (*filespec == '/')
	{
		++ filespec;
	}
	const char* pathsplit = strrchr(filespec, '/');
	if (pathsplit)
	{
		zip_dir_info* dir  = dynamic_cast<zip_dir_info*>(getFileInfo(char_string(filespec, pathsplit - filespec).c_str(), &m_Files));
		if (dir)
		{
			container = &dir->sub_files;
			++ pathsplit;
		}
	}
	else
	{
		container = &m_Files;
		pathsplit = filespec;
	}
	if (container)
	{
		find_if(container->begin(), container->end(), std::not1(VisitorSpecFile(pVisitor, pathsplit, bSubdir)));
	}
}

zip_dir_info*  CBasicZipFile::__addpath(file_container* dir, CBasicSmartBuffer& bufIndex, zip_dir_info* dirinfo, const char* filepath)
{
	char_string filename = dirinfo->file_name + '/';
	// write file data
	zip_file_data_block block;
	ulg dost = unix2dostime(&dirinfo->time_lastmodify);
	block.modify_date = HIWORD(dost);
	block.modify_time = LOWORD(dost);
	block.size_filename = filename.length();
	Write(&block, sizeof(zip_file_data_block));
	Write(filename.c_str(), block.size_filename);

	// write index data
	zip_file_dir_block dirblock;
	dirblock.modify_date = block.modify_date;
	dirblock.modify_time = block.modify_time;
	dirblock.property_outer_file = property_outer_file_dir;	// path
	dirblock.local_header_offset = m_end.offset_dir_to_first_disk;
	dirblock.size_filename = block.size_filename;
	dirblock.size_comment = dirinfo->comment.length();
	bufIndex.AppendData((char*)&dirblock, sizeof(zip_file_dir_block));
	bufIndex.AppendData(filename.c_str(), dirblock.size_filename);
	bufIndex.AppendData(dirinfo->comment.c_str(), dirblock.size_comment);

	m_end.offset_dir_to_first_disk += sizeof(zip_file_data_block) + block.size_filename;
	++ m_end.file_num_total;
	return dirinfo;
}

zip_file_info* CBasicZipFile::__addfile(CBasicStaticBuffer* dataBuffer, file_container* dir, CBasicSmartBuffer& bufIndex, zip_file_info* fileinfo, const char* filepath)
{
	zip_file_data_block block;
	ulg dost = unix2dostime(&fileinfo->time_lastmodify);
	block.modify_date = HIWORD(dost);
	block.modify_time = LOWORD(dost);
	block.size_filename = fileinfo->file_name.length();
	block.zip_type = 0x08;
	block.size_unzipped = dataBuffer->GetLength();

	zip_file_dir_block dirblock;
	dirblock.modify_date = block.modify_date;
	dirblock.modify_time = block.modify_time;
	dirblock.size_filename = block.size_filename;
	dirblock.zip_type = block.zip_type;
	dirblock.size_comment = fileinfo->comment.length();
	dirblock.size_unzipped = block.size_unzipped;
	dirblock.property_outer_file = property_outer_file_file;
	dirblock.local_header_offset = m_end.offset_dir_to_first_disk;
	fileinfo->offset_data_block = dirblock.local_header_offset;
	
	uint32 datalen = dataBuffer->GetLength();
	bool rval = false;
	CBasicStaticBuffer zippedBuffer;
	if (block.size_unzipped > 0)
	{
		
		uInt outlen = (uInt)(block.size_unzipped * 1.1);
		char* zipped = (char*)zippedBuffer.Alloc(outlen);

		z_stream stream;
		int err;

		stream.next_in = (Bytef*)dataBuffer->GetBuffer();
		stream.avail_in = (uInt)block.size_unzipped;

		block.crc = dirblock.crc = crc32(0, stream.next_in, block.size_unzipped);

		stream.next_out = (Bytef*)zipped;
		stream.avail_out = outlen;

		stream.zalloc = 0;
		stream.zfree = 0;
		stream.opaque = (voidpf)0;


		err = deflateInit2(&stream, m_ziplevel,
			Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

		if (err == Z_OK)
		{
			err = deflate(&stream, Z_FINISH);
			if (err == Z_STREAM_END)
			{
				rval = (deflateEnd(&stream) == Z_OK);
				if (rval)
				{
					if (stream.total_out < block.size_unzipped)	// 压缩了比不压缩还大，还不如不压缩
					{
						block.size_zipped = dirblock.size_zipped = fileinfo->size_zipped = stream.total_out;
						rval = true;
					}
					else
					{
						rval = false;
					}
				}
			}
			else
			{
				deflateEnd(&stream);
			}
		}
	}

	if (rval)
	{
		datalen = block.size_zipped;
		dataBuffer = &zippedBuffer;

		switch(m_ziplevel)
		{
		case Z_BEST_SPEED:
			block.bit_flag = 1 << 3;
			break;
		case Z_BEST_COMPRESSION:
			block.bit_flag = 1 << 1;
			break;
		}
	}
	else
	{
		block.zip_type = dirblock.zip_type = 0;
		block.size_zipped = block.size_unzipped;
		dirblock.size_zipped = dirblock.size_unzipped;
	}
	Write(&block, sizeof(zip_file_data_block));
	Write(fileinfo->file_name.c_str(), block.size_filename);
	Write(dataBuffer->GetBuffer(), datalen);

	m_end.offset_dir_to_first_disk += sizeof(zip_file_data_block) + block.size_filename + datalen;
	++ m_end.file_num_total;
	
	fileinfo->size_zipped = block.size_zipped;
	fileinfo->zip_type = block.zip_type;
	fileinfo->offset_dir_block = bufIndex.GetDataLength();
	bufIndex.AppendData((const char*)&dirblock, sizeof(zip_file_dir_block));
	bufIndex.AppendData(fileinfo->file_name.c_str(), dirblock.size_filename);
	bufIndex.AppendData(fileinfo->comment.c_str(), dirblock.size_comment);

	return fileinfo;

}

bool CBasicZipFile::AddPathToFile(const char* path)
{
	if (IsReadOnly())
		return false;
	bool ret = false;

	CBasicSmartBuffer bufIndex;
	bufIndex.SetDataLength(m_end.size_dir);
	Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);
	Read(bufIndex.GetDataBuffer(), m_end.size_dir);

	Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	file_container* dir = &m_Files;
	__adddir(path, bufIndex, "", dir);

	m_end.size_dir = bufIndex.GetDataLength();
	Write(bufIndex.GetDataBuffer(), m_end.size_dir);
	Write(&m_end, sizeof(zip_file_end));
	Write(m_strComment.c_str(), m_end.size_comment);
	ret = true;
	return ret;
}
bool CBasicZipFile::AddPackFile(const char* filename, const char* path, const char* comment)
{
	if (IsReadOnly())
		return false;
	bool ret = false;
	TLFileStatus status;
	if (BASIC_FILE_OK != Basic_GetFileStatus(filename, status))
		return ret;

	const char* cfilename = filename;

	CBasicSmartBuffer bufIndex;
	bufIndex.SetDataLength(m_end.size_dir);
	Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);
	Read(bufIndex.GetDataBuffer(), m_end.size_dir);

	Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	// first mkdir
	file_container* dir = &m_Files;
	if (!IsStringEmpty(path))
	{
		char_string fullpath;
		char_string filepath = getRootPath(path);

		const char* p = stepPath(path);
		while(!filepath.empty())
		{
			fullpath += filepath;
			zip_dir_info* dirinfo = dynamic_cast<zip_dir_info*>(getFileInfo(filepath.c_str(), dir));
			if (NULL == dirinfo)
			{
				dirinfo = Basic_NewObject<zip_dir_info>();
				dirinfo->file_name = fullpath;
				dirinfo->time_lastmodify = time(NULL);
				(*dir)[filepath] = dirinfo;

				__addpath(dir, bufIndex, dirinfo, filepath.c_str());
			}
			fullpath += "/";
			dir = &(dirinfo->sub_files);
			filepath = getRootPath(p);
			p = stepPath(p);
		}
	}
	char_string filepath = getFileName(cfilename);
	if (status.m_attribute & _basic_fa_directory)
	{
		zip_base_file_info* info = getFileInfo(filepath.c_str(), dir);
		zip_file_info* fileinfo = dynamic_cast<zip_file_info*>(info);
		if (fileinfo)
			return false;

		zip_dir_info* dirinfo = dynamic_cast<zip_dir_info*>(info);
		if (NULL == dirinfo)
		{
			dirinfo = Basic_NewObject<zip_dir_info>();
			if (IsStringEmpty(path))
				dirinfo->file_name = filepath;
			else
				dirinfo->file_name = path + filepath;
			dirinfo->time_lastmodify = status.m_mtime;
			(*dir)[filepath] = dirinfo;

			__addpath(dir, bufIndex, dirinfo, filepath.c_str());
		}
		__adddir(filename, bufIndex, dirinfo->file_name + '/', &dirinfo->sub_files);
	}
	else
	{
		zip_base_file_info* info = getFileInfo(filepath.c_str(), dir);
		zip_dir_info* dirinfo = dynamic_cast<zip_dir_info*>(info);
		if (dirinfo)
			return false;

		CBasicFileObj file;
		if (BASIC_FILE_OK == file.Open(filename, PF_DISK_FILE|PF_READ_ONLY))
		{
			long len = file.GetLength();
			CBasicStaticBuffer buffer;
			char* buf = (char*)buffer.Alloc(len);
			file.Read(buf, len);
			file.Close();

			zip_file_info* fileinfo = dynamic_cast<zip_file_info*>(info);
			if (fileinfo)
			{
				removeIndexFromDirBuffer(bufIndex, fileinfo);
			}
			else
			{
				fileinfo = Basic_NewObject<zip_file_info>();
				if (IsStringEmpty(path))
					fileinfo->file_name = filepath;
				else
					fileinfo->file_name = path + filepath;
			}
			fileinfo->time_lastmodify = status.m_mtime;
			fileinfo->size_unzipped = len;
			if (comment)
				fileinfo->comment = comment;
			else
				fileinfo->comment.clear();


			(*dir)[filepath] = fileinfo;

			__addfile(&buffer, dir, bufIndex, fileinfo, filepath.c_str());
		}
	}
	
	m_end.size_dir = bufIndex.GetDataLength();
	Write(bufIndex.GetDataBuffer(), m_end.size_dir);
	Write(&m_end, sizeof(zip_file_end));
	Write(m_strComment.c_str(), m_end.size_comment);
	ret = true;
	return ret;
}

bool CBasicZipFile::__adddir(const char* dirname, CBasicSmartBuffer& bufIndex, char_string path, file_container* dir)
{
#ifdef __BASICWINDOWS
	tstring_s search = tstring_s(dirname) + "\\*.*";
#else
	tstring_s search = tstring_s(dirname) + "/*";
#endif
	char_string cfilename;
	CBasicFileFind fd;
	long lRet = fd.FindFile(search.c_str());
	while(BASIC_FILE_OK == lRet)
	{
		lRet = fd.FindNextFile();
		if (fd.IsDots())
			continue;

		cfilename = fd.GetFileName();

		if (fd.IsDirectory())
		{
			zip_base_file_info* info = getFileInfo(cfilename.c_str(), dir);
			zip_file_info* fileinfo = dynamic_cast<zip_file_info*>(info);
			if (fileinfo)
				continue;

			zip_dir_info* dirinfo = dynamic_cast<zip_dir_info*>(info);
			if (NULL == dirinfo)
			{
				dirinfo = Basic_NewObject<zip_dir_info>();
				dirinfo->file_name = path + cfilename;
				dirinfo->time_lastmodify = fd.GetLastWriteTime();
				(*dir)[cfilename] = dirinfo;
				__addpath(dir, bufIndex, dirinfo, path.c_str());	
			}
			__adddir(fd.GetFilePath().c_str(), bufIndex, dirinfo->file_name + '/', &dirinfo->sub_files);
		}
		else
		{
			zip_base_file_info* info = getFileInfo(cfilename.c_str(), dir);
			zip_dir_info* dirinfo = dynamic_cast<zip_dir_info*>(info);
			if (dirinfo)
				continue;

			CBasicFileObj file;
			if (BASIC_FILE_OK == file.Open(fd.GetFilePath().c_str(), PF_DISK_FILE|PF_READ_ONLY))
			{
				long len = file.GetLength();
				CBasicStaticBuffer buffer;
				char* buf = (char*)buffer.Alloc(len);
				file.Read(buf, len);
				file.Close();

				zip_file_info* fileinfo = dynamic_cast<zip_file_info*>(info);
				if (fileinfo)
				{
					removeIndexFromDirBuffer(bufIndex, fileinfo);
				}
				else
				{
					fileinfo = Basic_NewObject<zip_file_info>();
					fileinfo->file_name = path + cfilename;
				}
				fileinfo->time_lastmodify = fd.GetLastWriteTime();
				fileinfo->size_unzipped = len;
				(*dir)[cfilename] = fileinfo;

				__addfile(&buffer, dir, bufIndex, fileinfo, path.c_str());

			}
		}
	}
	return true;
}


class CBasicZipFile::ShortenOffset
{
public:
	ShortenOffset(uint32 diroffset, uint32 dirlen) : __diroffset(diroffset), __dirlen(dirlen){}

	int	operator()(const pair<char_string, zip_base_file_info*>& p)
	{
		zip_file_info* fileinfo = dynamic_cast<zip_file_info*>(p.second);
		zip_dir_info* dirinfo = NULL;
		if (fileinfo && fileinfo->offset_dir_block > __diroffset)
		{
			fileinfo->offset_dir_block -= __dirlen;
		}
		else if (NULL != (dirinfo = dynamic_cast<zip_dir_info*>(p.second)))
		{
			for_each(dirinfo->sub_files.begin(), dirinfo->sub_files.end(), *this);
		}
		return 0;
	}
protected:
	uint32	__diroffset;
	uint32	__dirlen;
};

bool CBasicZipFile::AddPackFile(const void* pBuffer, size_t length, const char* filename , const char* comment)
{
	CBasicStaticBuffer buffer;
	buffer.Attach((void*)pBuffer, length);
	bool bReturn = AddPackFile(&buffer, filename, comment);
	buffer.Detach();
	return bReturn;
}

bool CBasicZipFile::AddPackFile(CBasicStaticBuffer* buffer, const char* filename, const char* comment)
{
	if (IsReadOnly() || IsStringEmpty(filename))
		return false;
	CBasicSmartBuffer bufIndex;
	bufIndex.SetDataLength(m_end.size_dir);
	if (m_end.size_dir > 0)
	{
		Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);
		Read(bufIndex.GetDataBuffer(), m_end.size_dir);
	}

	Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	// first mkdir
	file_container* dir = &m_Files;
	if (!IsStringEmpty(filename))
	{
		char_string fullpath;
		char_string filepath = getRootPath(filename);

		const char* p = stepPath(filename);
		while(!filepath.empty())
		{
			fullpath += filepath;
			zip_dir_info* dirinfo = dynamic_cast<zip_dir_info*>(getFileInfo(filepath.c_str(), dir));
			if (NULL == dirinfo)
			{
				dirinfo = Basic_NewObject<zip_dir_info>();
				dirinfo->file_name = fullpath;
				dirinfo->time_lastmodify = time(NULL);
				(*dir)[filepath] = dirinfo;

				__addpath(dir, bufIndex, dirinfo, filepath.c_str());
			}
			fullpath += "/";
			dir = &(dirinfo->sub_files);
			filepath = getRootPath(p);
			p = stepPath(p);
		}
	}

	const char* file = getFileName(filename);
	zip_base_file_info* info = getFileInfo(file, dir);

	zip_dir_info* dirinfo = dynamic_cast<zip_dir_info*>(info);
	if (dirinfo)
		return false;

	zip_file_info* fileinfo = dynamic_cast<zip_file_info*>(info);
	if (NULL == fileinfo)
	{
		fileinfo = Basic_NewObject<zip_file_info>();
		fileinfo->file_name = filename;
		(*dir)[file] = fileinfo;
	}
	else
	{
		removeIndexFromDirBuffer(bufIndex, fileinfo);
	}
	fileinfo->time_lastmodify = time(NULL);
	fileinfo->size_unzipped = buffer->GetLength();
	if (comment)
		fileinfo->comment = comment;
	else
		fileinfo->comment.clear();


	__addfile(buffer, dir, bufIndex, fileinfo, filename);

	m_end.size_dir = bufIndex.GetDataLength();
	Write(bufIndex.GetDataBuffer(), m_end.size_dir);
	Write(&m_end, sizeof(zip_file_end));
	Write(m_strComment.c_str(), m_end.size_comment);
	SetLength(GetPosition());
	return true;
}

void CBasicZipFile::removeIndexFromDirBuffer(CBasicSmartBuffer& bufIndex, zip_file_info* fileinfo)
{
	uint32 diroffset = 0;
	uint32 dirlen = 0;

	diroffset = fileinfo->offset_dir_block;
	dirlen = sizeof(zip_file_dir_block) + fileinfo->file_name.length() + fileinfo->comment.length();

	long len = 0;
	char* p = bufIndex.GetDataBuffer(len);

	memmove(p + diroffset , p + diroffset + dirlen, len - diroffset - dirlen);
	bufIndex.SetDataLength(len - dirlen);

	for_each(m_Files.begin(), m_Files.end(), ShortenOffset(diroffset, dirlen));

	m_end.size_dir -= dirlen;
	-- m_end.file_num_total;

	fileinfo->offset_dir_block = 0;
	fileinfo->offset_data_block= 0;
	m_bNeedRepair = true;
}

bool CBasicZipFile::DeletePackFile(const char* filepath)
{
	if (IsReadOnly())
		return false;

	if (!getFileInfo(filepath, &m_Files))
		return false;
	
	CBasicStaticBuffer bufIndex;
	char* buf = (char*)bufIndex.Alloc(m_end.size_dir);
	Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);
	Read(buf, m_end.size_dir);
	Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	m_end.size_dir = 0;
	uint16 write = 0;
	size_t len = strlen(filepath);
	for (uint16 i = 0; i < m_end.file_num_total; ++ i)
	{
		zip_file_dir_block* block = (zip_file_dir_block*)buf;
		long dirlen = sizeof(zip_file_dir_block) + block->size_filename + block->size_extend + block->size_comment;
		char* pfile = (char*)(block + 1);
		if (block->size_filename < len
			|| strncmp(pfile, filepath, len) != 0
			|| (block->size_filename > len && pfile[len] != '/'))
		{
			Write(buf, dirlen);
			m_end.size_dir += dirlen;
			++ write;
		}
		buf += dirlen;
	}

	m_end.file_num_total = write;

	Write(&m_end, sizeof(zip_file_end));
	Write(m_strComment.c_str(), m_end.size_comment);

	SetLength(GetPosition());

	m_bNeedRepair = true;
		

	return ReOpen() == BASIC_FILE_OK;
}

zip_base_file_info* CBasicZipFile::deleteFileInfo(const char* file, file_container* dir)
{
	zip_base_file_info* info = NULL;
	file_container::iterator iter = dir->find(file);
	if (iter != dir->end())
	{
		info = iter->second;
		dir->erase(iter);
	}
	return info;
}

char_string CBasicZipFile::getRootPath(const char* filename)
{
	const char* pathsplit = strchr(filename, '/');
	if (pathsplit)
		return char_string(filename, pathsplit - filename);
	
	return char_string();
}

const char* CBasicZipFile::getFileName(const char* filename)
{
	const char* pathsplit = strrchr(filename, '/');
	if (pathsplit)
		return pathsplit + 1;
	pathsplit = strrchr(filename, '\\');
	if (pathsplit)
		return  pathsplit + 1;
	return filename;
}

const char* CBasicZipFile::stepPath(const char* filename)
{
	const char* pathsplit = strchr(filename, '/');
	if (pathsplit)
		return pathsplit + 1;
	pathsplit =strchr(filename, '\\');
	if (pathsplit)
		return pathsplit + 1;
	return NullAString;
}

bool CBasicZipFile::repairFile()
{
	bool ret = false;
	tstring_s strFilePath = GetFilePath();
	tstring_s str = strFilePath + ".repair";
	CBasicFileObj file;
	if (BASIC_FILE_OK == file.Open(str.c_str(), CBasicFileObj::GetFileOpenType()))
	{
		CBasicStaticBuffer bufIndex;
		char* p = (char*)bufIndex.Alloc(m_end.size_dir);
		Seek(m_end.offset_dir_to_first_disk, BASIC_FILE_BEGIN);
		Read(p, m_end.size_dir);

		uint32 local_header_offset = 0;

		uint32 extendlen = 0;
		CBasicStaticBuffer tempBuf;
		char* extend = NULL;
		zip_file_data_block datablock;
		zip_file_dir_block* dirblock = (zip_file_dir_block*)p;
		for(int i = 0; i < m_end.file_num_total; ++ i)
		{
			// data block
			Seek(dirblock->local_header_offset, BASIC_FILE_BEGIN);
			Read(&datablock, sizeof(zip_file_data_block));
			file.Write(&datablock, sizeof(zip_file_data_block));

			extendlen = datablock.size_filename + datablock.size_extend;
			if (datablock.zip_type == 0)
				extendlen += datablock.size_unzipped;
			else
				extendlen += datablock.size_zipped;

			extend = (char*)tempBuf.Alloc(extendlen);
			Read(extend, extendlen);
			file.Write(extend, extendlen);

			dirblock->local_header_offset = local_header_offset;
			local_header_offset += sizeof(zip_file_data_block) + extendlen;

			dirblock = (zip_file_dir_block*)((char*)(dirblock + 1) + dirblock->size_filename + dirblock->size_extend + dirblock->size_comment);
		}

		m_end.offset_dir_to_first_disk = local_header_offset;
		file.Write(bufIndex.GetBuffer(), m_end.size_dir);
		file.Write(&m_end, sizeof(zip_file_end));
		file.Write(m_strComment.c_str(), m_strComment.length());

		CBasicFileObj::Close();

		ret = file.CopyTo(strFilePath.c_str()) == BASIC_FILE_OK;
		file.Close();
		Basic_DeleteFile(str.c_str());
		m_bNeedRepair = false;
		ret = true;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct basicfile_data_block
{
	Net_UChar		m_cSignFileBegin;//标识符1
	Net_UShort		bit_flag;		// 全局方式位标记 ?
	Net_UChar		zip_type;		// 压缩方式
	Net_UChar		m_cSignFileMid;//标识符2
	Net_UInt		crc;			// crc32校验
	Net_UInt		size_zipped;	// 压缩后尺寸
	Net_UInt		size_unzipped;	// 未压缩尺寸
	Net_UShort		size_filename;	// 文件名长度
	Net_UChar		m_cSignFileEnd;//标识符3
	// 文件名
	// 扩展
	// 内容
	basicfile_data_block()
	{
		memset(this, 0, sizeof(basicfile_data_block));
		m_cSignFileBegin = 0x01;
		m_cSignFileMid = 0x02;
		m_cSignFileEnd = 0x03;
	}
	bool IsBasicFileDataBlock()
	{
		return m_cSignFileBegin == 0x01 && m_cSignFileMid == 0x02 && m_cSignFileEnd == 0x03;
	}
};

struct basicfile_dir_block
{
	Net_UChar	m_cSignFileBegin;//标识符1
	Net_UShort	bit_flag;				// 全局方式位标记
	Net_UChar	zip_type;				// 压缩方式
	Net_UChar	m_cSignFileMid;//标识符1
	Net_UInt	crc;					// crc32校验
	Net_UInt	size_zipped;			// 压缩后尺寸
	Net_UInt	size_unzipped;			// 未压缩尺寸
	Net_UShort	size_filename;			// 文件名长度
	Net_UShort	size_extend;			// 扩展记录长度
	Net_UShort	property_internal_file;	// 内部文件属性
	Net_UInt	property_outer_file;	// 外部文件属性
	Net_UInt	local_header_offset;	// 局部头部偏移量
	Net_UChar	m_cSignFileEnd;//标识符3
	// 文件名
	// 扩展字段
	// 文件注释
	basicfile_dir_block()
	{
		memset(this, 0, sizeof(basicfile_dir_block));
		m_cSignFileBegin = 0x02;
		m_cSignFileMid = 0x03;
		m_cSignFileEnd = 0x04;
	}
	bool IsBasicFileDirBlock()
	{
		return m_cSignFileBegin == 0x02 && m_cSignFileMid == 0x03 && m_cSignFileEnd == 0x04;
	}
};

struct basicfile_end
{
	Net_UChar	m_cSignFileBegin;//标识符1
	Net_UChar	m_cSignFileMid;//标识符1
	Net_UShort	file_num_total;				// 目录区中记录总数
	Net_UInt	size_dir;					// 目录区尺寸大小
	Net_UInt	offset_dir_to_first_disk;	// 目录区对第一张磁盘的偏移量
	Net_UChar	m_cSignFileEnd;//标识符3

	// 注释
	basicfile_end()
	{
		memset(this, 0, sizeof(basicfile_end));
		m_cSignFileBegin = 0x03;
		m_cSignFileMid = 0x04;
		m_cSignFileEnd = 0x05;
	}
	bool IsBasicFileEndBlock()
	{
		return m_cSignFileBegin == 0x03 && m_cSignFileMid == 0x04 && m_cSignFileEnd == 0x05;
	}
};

#pragma pack()

// 目录信息
struct basicdir_info : public basicbase_file_info
{
	basicfile_container	sub_files;	// 文件列表

	virtual ~basicdir_info()
	{
		for (basicfile_container::iterator iter = sub_files.begin(); iter != sub_files.end(); ++iter)
		{
			basiclib::BASIC_DeleteObject(iter->second);
		}
		sub_files.clear();
	}

	virtual bool add_file(basicbase_file_info* file)
	{
		//sub_files.push_back(file);
		const char* nakename = strrchr(file->file_name.c_str(), '/');
		if (NULL == nakename)
			nakename = file->file_name.c_str();
		else
			++nakename;

		sub_files[nakename] = file;
		return true;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Net_Int k[4] = { 0x1F0DA000, 0x11223344, 0x12345678, 0x38682BAF };
static int g_defaultZipLevel = 9;


basicbase_file_info* getFileInfo(const char* filename, basicfile_container* dir)
{
	basiclib::CBasicString key;
	const char* sub = strchr(filename, '/');
	if (sub)	// is dir
	{
		key = basiclib::CBasicString(filename, sub - filename);
	}
	else
	{
		key = filename;
	}

	basicbase_file_info* info = NULL;
	basicfile_container::iterator iter = dir->lower_bound(key);
	if (iter != dir->end() && iter->first == key)
	{
		if (sub && *(++sub) != '\0')
		{
			basicdir_info* subfile = dynamic_cast<basicdir_info*>(iter->second);
			if (subfile)
				info = getFileInfo(sub, &subfile->sub_files);
		}
		else
		{
			info = iter->second;
		}
	}
	return info;
}

basicfile_info*	__addfile(CBasicCombinFile* pFile, basicfile_end& m_end, basiclib::CBasicStaticBuffer* dataBuffer, basicfile_container* dir, basiclib::CBasicSmartBuffer& bufIndex, basicfile_info* fileinfo, const char* filepath)
{
	basicfile_data_block block;
	block.size_filename = fileinfo->file_name.length();
	block.zip_type = 0x01;
	block.size_unzipped = dataBuffer->GetLength();

	basicfile_dir_block dirblock;
	dirblock.size_filename = block.size_filename;
	dirblock.zip_type = block.zip_type;
	dirblock.size_unzipped = block.size_unzipped;
	dirblock.property_outer_file = property_outer_file_file;
	dirblock.local_header_offset = m_end.offset_dir_to_first_disk;
	fileinfo->offset_data_block = dirblock.local_header_offset;

	Net_UInt datalen = dataBuffer->GetLength();
	bool rval = false;
	basiclib::CBasicStaticBuffer zippedBuffer;
	if (block.size_unzipped > 0)
	{
		uInt outlen = (uInt)(block.size_unzipped * 1.1);
		char* zipped = (char*)zippedBuffer.Alloc(outlen);

		z_stream stream;
		int err;

		stream.next_in = (Bytef*)dataBuffer->GetBuffer();
		stream.avail_in = (uInt)block.size_unzipped;

		block.crc = dirblock.crc = crc32(0, stream.next_in, block.size_unzipped);

		stream.next_out = (Bytef*)zipped;
		stream.avail_out = outlen;

		stream.zalloc = 0;
		stream.zfree = 0;
		stream.opaque = (voidpf)0;

		err = deflateInit2(&stream, g_defaultZipLevel,
			Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

		if (err == Z_OK)
		{
			err = deflate(&stream, Z_FINISH);
			if (err == Z_STREAM_END)
			{
				rval = (deflateEnd(&stream) == Z_OK);
				if (rval)
				{
					if (stream.total_out < block.size_unzipped)	// 压缩了比不压缩还大，还不如不压缩
					{
						block.size_zipped = dirblock.size_zipped = fileinfo->size_zipped = stream.total_out;
						rval = true;
					}
					else
					{
						rval = false;
					}
				}
			}
			else
			{
				deflateEnd(&stream);
			}
		}
	}

	if (rval)
	{
		datalen = block.size_zipped;
		dataBuffer = &zippedBuffer;

		switch (g_defaultZipLevel)
		{
		case Z_BEST_SPEED:
			block.bit_flag = 1 << 3;
			break;
		case Z_BEST_COMPRESSION:
			block.bit_flag = 1 << 1;
			break;
		}
	}
	else
	{
		block.zip_type = dirblock.zip_type = 0;
		block.size_zipped = block.size_unzipped;
		dirblock.size_zipped = dirblock.size_unzipped;
	}
	
	int nNumber = datalen / 4;
	if (nNumber >= 4)
	{
		//经过一次加密处理
		block.zip_type |= 0x02;
		dirblock.zip_type |= 0x02;
		Baisc_XTRA((int*)dataBuffer->GetBuffer(), nNumber, k);
	}
	
	pFile->Write(&block, sizeof(basicfile_data_block));
	pFile->Write(fileinfo->file_name.c_str(), block.size_filename);

	pFile->Write(dataBuffer->GetBuffer(), datalen);

	m_end.offset_dir_to_first_disk += sizeof(basicfile_data_block)+block.size_filename + datalen;
	++m_end.file_num_total;

	fileinfo->size_zipped = block.size_zipped;
	fileinfo->zip_type = block.zip_type;
	fileinfo->offset_dir_block = bufIndex.GetDataLength();
	bufIndex.AppendData((const char*)&dirblock, sizeof(basicfile_dir_block));
	bufIndex.AppendData(fileinfo->file_name.c_str(), dirblock.size_filename);

	return fileinfo;
}

basicdir_info* __addpath(CBasicCombinFile* pFile, basicfile_end& m_end, basicfile_container* dir, basiclib::CBasicSmartBuffer& bufIndex, basicdir_info* dirinfo, const char* filepath)
{
	basiclib::CBasicString filename = dirinfo->file_name;
	filename += '/';
	// write file data
	basicfile_data_block block;

	block.size_filename = filename.length();
	pFile->Write(&block, sizeof(basicfile_data_block));
	pFile->Write(filename.c_str(), block.size_filename);

	// write index data
	basicfile_dir_block dirblock;
	dirblock.property_outer_file = property_outer_file_dir;	// path
	dirblock.local_header_offset = m_end.offset_dir_to_first_disk;
	dirblock.size_filename = block.size_filename;
	bufIndex.AppendData((char*)&dirblock, sizeof(basicfile_dir_block));
	bufIndex.AppendData(filename.c_str(), dirblock.size_filename);

	m_end.offset_dir_to_first_disk += sizeof(basicfile_data_block)+block.size_filename;
	++m_end.file_num_total;
	return dirinfo;
}


class CBasicCombinFile::ShortenOffset
{
public:
	ShortenOffset(Net_UInt diroffset, Net_UInt dirlen) : __diroffset(diroffset), __dirlen(dirlen){}

	int	operator()(const pair<basiclib::CBasicString, basicbase_file_info*>& p)
	{
		basicfile_info* fileinfo = dynamic_cast<basicfile_info*>(p.second);
		basicdir_info* dirinfo = NULL;
		if (fileinfo && fileinfo->offset_dir_block > __diroffset)
		{
			fileinfo->offset_dir_block -= __dirlen;
		}
		else if (NULL != (dirinfo = dynamic_cast<basicdir_info*>(p.second)))
		{
			for_each(dirinfo->sub_files.begin(), dirinfo->sub_files.end(), *this);
		}
		return 0;
	}
protected:
	Net_UInt	__diroffset;
	Net_UInt	__dirlen;
};


void removeIndexFromDirBuffer(CBasicCombinFile* pFile, basicfile_end& m_end, basiclib::CBasicSmartBuffer& bufIndex, basicfile_info* fileinfo)
{
	Net_UInt diroffset = 0;
	Net_UInt dirlen = 0;

	diroffset = fileinfo->offset_dir_block;
	dirlen = sizeof(basicfile_dir_block)+fileinfo->file_name.length();

	long len = 0;
	char* p = bufIndex.GetDataBuffer(len);

	memmove(p + diroffset, p + diroffset + dirlen, len - diroffset - dirlen);
	bufIndex.SetDataLength(len - dirlen);

	for_each(pFile->m_Files.begin(), pFile->m_Files.end(), CBasicCombinFile::ShortenOffset(diroffset, dirlen));

	m_end.size_dir -= dirlen;
	--m_end.file_num_total;

	fileinfo->offset_dir_block = 0;
	fileinfo->offset_data_block = 0;
	pFile->m_bNeedRepair = true;
}

bool __adddir(CBasicCombinFile* pFile, basicfile_end& m_end, const char* dirname, basiclib::CBasicSmartBuffer& bufIndex, basiclib::CBasicString& path, basicfile_container* dir)
{
#ifdef __BASICWINDOWS
	basiclib::CBasicString search = basiclib::CBasicString(dirname) + "\\*.*";
#else
	basiclib::CBasicString search = basiclib::CBasicString(dirname) + "/*";
#endif
	basiclib::CBasicString cfilename;
	basiclib::CBasicFileFind fd;
	long lRet = fd.FindFile(search.c_str());
	while (BASIC_FILE_OK == lRet)
	{
		lRet = fd.FindNextFile();
		if (fd.IsDots())
			continue;

		cfilename = fd.GetFileName();

		if (fd.IsDirectory())
		{
			basicbase_file_info* info = getFileInfo(cfilename.c_str(), dir);
			basicfile_info* fileinfo = dynamic_cast<basicfile_info*>(info);
			if (fileinfo)
				continue;

			basicdir_info* dirinfo = dynamic_cast<basicdir_info*>(info);
			if (NULL == dirinfo)
			{
				dirinfo = basiclib::Basic_NewObject<basicdir_info>();
				dirinfo->file_name = path + cfilename;
				(*dir)[cfilename] = dirinfo;
				__addpath(pFile, m_end, dir, bufIndex, dirinfo, path.c_str());
			}
			basiclib::CBasicString strPathEnter = dirinfo->file_name;
			strPathEnter += '/';
			__adddir(pFile, m_end, fd.GetFilePath().c_str(), bufIndex, strPathEnter, &dirinfo->sub_files);
		}
		else
		{
			basicbase_file_info* info = getFileInfo(cfilename.c_str(), dir);
			basicdir_info* dirinfo = dynamic_cast<basicdir_info*>(info);
			if (dirinfo)
				continue;

			CBasicFileObj file;
			if (BASIC_FILE_OK == file.Open(fd.GetFilePath().c_str(), PF_DISK_FILE | PF_READ_ONLY))
			{
				long len = file.GetLength();
				basiclib::CBasicStaticBuffer buffer;
				char* buf = (char*)buffer.Alloc(len);
				file.Read(buf, len);
				file.Close();

				basicfile_info* fileinfo = dynamic_cast<basicfile_info*>(info);
				if (fileinfo)
				{
					removeIndexFromDirBuffer(pFile, m_end, bufIndex, fileinfo);
				}
				else
				{
					fileinfo = basiclib::Basic_NewObject<basicfile_info>();
					fileinfo->file_name = path + cfilename;
				}
				fileinfo->size_unzipped = len;
				(*dir)[cfilename] = fileinfo;

				__addfile(pFile, m_end, &buffer, dir, bufIndex, fileinfo, path.c_str());
			}
		}
	}
	return true;
}

bool __adddirComBinFile(CBasicCombinFile* pFile, basicfile_end& m_end, CBasicCombinFile* pComBinFile, basicfile_container* pDstDirName, basiclib::CBasicSmartBuffer& bufIndex, basiclib::CBasicString& path, basicfile_container* dir)
{
	for (auto& dstChildDir : *pDstDirName)
	{
		basicbase_file_info* pDstBaseInfo = dstChildDir.second;
		basicdir_info* pDstDirInfo = dynamic_cast<basicdir_info*>(pDstBaseInfo);
		if (pDstDirInfo)
		{
			basicbase_file_info* pSrcInfo = getFileInfo(dstChildDir.first.c_str(), dir);
			basicfile_info* pSrcFileinfo = dynamic_cast<basicfile_info*>(pSrcInfo);
			if (pSrcFileinfo)
				continue;

			basicdir_info* pSrcDirInfo = dynamic_cast<basicdir_info*>(pSrcInfo);
			if (NULL == pSrcDirInfo)
			{
				pSrcDirInfo = basiclib::Basic_NewObject<basicdir_info>();
				pSrcDirInfo->file_name = path + dstChildDir.first.c_str();
				(*dir)[dstChildDir.first.c_str()] = pSrcDirInfo;
				__addpath(pFile, m_end, dir, bufIndex, pSrcDirInfo, path.c_str());
			}
			basiclib::CBasicString strPathEnter = pSrcDirInfo->file_name;
			strPathEnter += '/';
			__adddirComBinFile(pFile, m_end, pComBinFile, &(pDstDirInfo->sub_files), bufIndex, strPathEnter, &pSrcDirInfo->sub_files);
			continue;
		}
		else
		{
			basicfile_info* pDstFileInfo = dynamic_cast<basicfile_info*>(pDstBaseInfo);
			if (pDstFileInfo)
			{
				basicbase_file_info* pSrcInfo = getFileInfo(dstChildDir.first.c_str(), dir);
				basicdir_info* pSrcDirinfo = dynamic_cast<basicdir_info*>(pSrcInfo);
				if (pSrcDirinfo)
					continue;

				//获取数据
				basiclib::CBasicSmartBuffer smResFile;
				pComBinFile->ReadFileFromFileInfo(pDstFileInfo, smResFile);

				basicfile_info* fileinfo = dynamic_cast<basicfile_info*>(pSrcInfo);
				if (fileinfo)
				{
					removeIndexFromDirBuffer(pFile, m_end, bufIndex, fileinfo);
				}
				else
				{
					fileinfo = basiclib::Basic_NewObject<basicfile_info>();
					fileinfo->file_name = path + dstChildDir.first;
				}
				fileinfo->size_unzipped = smResFile.GetDataLength();
				(*dir)[dstChildDir.first.c_str()] = fileinfo;

				CBasicStaticBuffer staticBuffer;
				staticBuffer.Attach(smResFile.GetDataBuffer(), smResFile.GetDataLength());
				__addfile(pFile, m_end, &staticBuffer, dir, bufIndex, fileinfo, path.c_str());
				staticBuffer.Detach();

				continue;
			}
		}
		
		ASSERT(0);
	}

	return true;
}


// CBasicZipFile
CBasicCombinFile::CBasicCombinFile(void)
{
	m_bNeedRepair = false;
	m_pEnd = new basicfile_end();
}

CBasicCombinFile::~CBasicCombinFile(void)
{
	if (m_pEnd)
	{
		delete m_pEnd;
		m_pEnd = NULL;
	}
}

// 文件操作
//! \brief 打开一个zip文件。这个操作过程中应该已经读取所有的文件索引
long CBasicCombinFile::OpenCombinFile(const char* lpszFileName, DWORD dwOpenFlags)
{
	long lError = CBasicFileObj::Open(lpszFileName, dwOpenFlags);
	if (BASIC_FILE_OK != lError)
		return lError;

	return buildZipIndex();
}

void CBasicCombinFile::Close()
{
	for (basicfile_container::iterator iter = m_Files.begin(); iter != m_Files.end(); ++iter)
	{
		basiclib::BASIC_DeleteObject(iter->second);
	}
	m_Files.clear();

	if (m_bNeedRepair)
		repairFile();
	else
		CBasicFileObj::Close();
}
Net_UShort CBasicCombinFile::GetFileNumTotal() const
{
	return m_pEnd->file_num_total;
}

long CBasicCombinFile::Flush()
{
	return CBasicFileObj::Flush();
}

//!子文件的操作
bool CBasicCombinFile::GetPackFile(const char* filename, basiclib::CBasicSmartBuffer& smBuf)
{
	basicfile_info* fileinfo = dynamic_cast<basicfile_info*>(getFileInfo(filename, &m_Files));
	if (fileinfo)
	{
		return ReadFileFromFileInfo(fileinfo, smBuf);
	}
	return false;
}

//checkfunc
bool CBasicCombinFile::CheckFuncImpl(basicfile_container& files)
{
	basiclib::CBasicSmartBuffer smBuf;
	for (auto& childfile : files)
	{
		basicbase_file_info* pbaseInfo = childfile.second;
		basicdir_info* dirinfo = dynamic_cast<basicdir_info*>(pbaseInfo);
		if (dirinfo)
		{
			if (!CheckFuncImpl(dirinfo->sub_files))
			{
				return false;
			}
		}
		else
		{
			basicfile_info* fileinfo = dynamic_cast<basicfile_info*>(pbaseInfo);
			if (fileinfo)
			{
				if (!ReadFileFromFileInfo(fileinfo, smBuf))
				{
					return false;
				}
				smBuf.SetDataLength(0);
			}
		}
	}
	return true;
}
//check所有的文件是否正确
bool CBasicCombinFile::CheckFileIsValid()
{
	return CheckFuncImpl(m_Files);
}

//readfromfile
bool CBasicCombinFile::ReadFileFromFileInfo(const basicfile_info* info, basiclib::CBasicSmartBuffer& smBuf)
{
	Seek(info->offset_data_block, BASIC_FILE_BEGIN);

	basicfile_data_block file_block;
	if (sizeof(basicfile_data_block) != Read(&file_block, sizeof(basicfile_data_block))
		|| !file_block.IsBasicFileDataBlock())
	{
		ASSERT(0);
		return false;
	}


	do
	{
		if (info->size_zipped > 0)
		{
			basiclib::CBasicStaticBuffer reader;
			char* zipped = (char*)reader.Alloc(info->size_zipped);
			Seek(file_block.size_filename, BASIC_FILE_CURRENT);
			if (info->size_zipped != Read(zipped, info->size_zipped))
				return false;

			if (info->zip_type & 0x02)
			{
				int nNumber = file_block.size_zipped / 4;
				if (nNumber >= 4)
				{
					Baisc_XTRA((int*)zipped, -nNumber, k);
				}
				else
				{
					ASSERT(0);
				}
			}
			if (info->zip_type & 0x01)
			{
				basiclib::CBasicStaticBuffer buffer;
				char* unzipped = (char*)buffer.Alloc(file_block.size_unzipped);

				int err;
				z_stream stream;

				stream.next_in = (Bytef*)zipped;
				stream.avail_in = (uInt)file_block.size_zipped;

				stream.next_out = (Bytef*)unzipped;
				stream.avail_out = (uInt)file_block.size_unzipped;

				stream.zalloc = (alloc_func)0;
				stream.zfree = (free_func)0;
				stream.opaque = (voidpf)0;

				err = inflateInit2(&stream, -MAX_WBITS);
				if (err != Z_OK)
				{
					return false;
				}

				err = inflate(&stream, Z_FINISH);
				if (err != Z_STREAM_END)
				{
					inflateEnd(&stream);
					if (err != Z_OK)
					{
						return false;
					}
				}
				else
					err = inflateEnd(&stream);

				uLong crc = crc32(0, (Bytef*)unzipped, file_block.size_unzipped);
				if (crc != file_block.crc)
				{
					return false;
				}

				smBuf.AppendData(unzipped, file_block.size_unzipped);
			}
			else
			{
				smBuf.AppendData(zipped, file_block.size_zipped);
			}
		}
	} while (false);
	return true;
}

bool CBasicCombinFile::GetPackFileStatus(const char* filename, BasicPackFileStatus& status)
{
	basicbase_file_info* info = getFileInfo(filename, &m_Files);
	if (NULL == info)
		return false;

	getFileStatus(info, status);
	return true;
}

bool CBasicCombinFile::AddPackFile(const void* pBuffer, size_t length, const char* filename)
{
	basiclib::CBasicStaticBuffer buffer;
	buffer.Attach((void*)pBuffer, length);
	bool bReturn = AddPackFile(&buffer, filename);
	buffer.Detach();
	return bReturn;
}

bool CBasicCombinFile::AddPackFile(basiclib::CBasicStaticBuffer* buffer, const char* filename)
{
	if (IsReadOnly() || basiclib::IsStringEmpty(filename))
		return false;

	basiclib::CBasicSmartBuffer bufIndex;
	bufIndex.SetDataLength(m_pEnd->size_dir);
	if (m_pEnd->size_dir > 0)
	{
		Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);
		Read(bufIndex.GetDataBuffer(), m_pEnd->size_dir);
	}

	Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	// first mkdir
	basicfile_container* dir = &m_Files;
	if (!basiclib::IsStringEmpty(filename))
	{
		basiclib::CBasicString fullpath;
		basiclib::CBasicString filepath = getRootPath(filename);

		const char* p = stepPath(filename);
		while (!filepath.empty())
		{
			fullpath += filepath;
			basicdir_info* dirinfo = dynamic_cast<basicdir_info*>(getFileInfo(filepath.c_str(), dir));
			if (NULL == dirinfo)
			{
				dirinfo = basiclib::Basic_NewObject<basicdir_info>();
				dirinfo->file_name = fullpath;
				(*dir)[filepath] = dirinfo;

				__addpath(this, *m_pEnd, dir, bufIndex, dirinfo, filepath.c_str());
			}
			fullpath += "/";
			dir = &(dirinfo->sub_files);
			filepath = getRootPath(p);
			p = stepPath(p);
		}
	}

	const char* file = getFileName(filename);
	basicbase_file_info* info = getFileInfo(file, dir);

	basicdir_info* dirinfo = dynamic_cast<basicdir_info*>(info);
	if (dirinfo)
		return false;

	basicfile_info* fileinfo = dynamic_cast<basicfile_info*>(info);
	if (NULL == fileinfo)
	{
		fileinfo = basiclib::Basic_NewObject<basicfile_info>();
		fileinfo->file_name = filename;
		(*dir)[file] = fileinfo;
	}
	else
	{
		removeIndexFromDirBuffer(this, *m_pEnd, bufIndex, fileinfo);
	}
	fileinfo->size_unzipped = buffer->GetLength();

	__addfile(this, *m_pEnd, buffer, dir, bufIndex, fileinfo, filename);

	m_pEnd->size_dir = bufIndex.GetDataLength();
	Write(bufIndex.GetDataBuffer(), m_pEnd->size_dir);
	Write(m_pEnd, sizeof(basicfile_end));
	SetLength(GetPosition());
	return true;
}


bool CBasicCombinFile::DeletePackFile(const char* filepath)
{
	if (IsReadOnly())
		return false;

	if (!getFileInfo(filepath, &m_Files))
		return false;

	basiclib::CBasicStaticBuffer bufIndex;
	char* buf = (char*)bufIndex.Alloc(m_pEnd->size_dir);
	Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);
	Read(buf, m_pEnd->size_dir);
	Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	m_pEnd->size_dir = 0;
	Net_UShort write = 0;
	size_t len = strlen(filepath);
	for (Net_UShort i = 0; i < m_pEnd->file_num_total; ++i)
	{
		basicfile_dir_block* block = (basicfile_dir_block*)buf;
		long dirlen = sizeof(basicfile_dir_block)+block->size_filename + block->size_extend;
		char* pfile = (char*)(block + 1);
		if (block->size_filename < len
			|| strncmp(pfile, filepath, len) != 0
			|| (block->size_filename > len && pfile[len] != '/'))
		{
			Write(buf, dirlen);
			m_pEnd->size_dir += dirlen;
			++write;
		}
		buf += dirlen;
	}

	m_pEnd->file_num_total = write;

	Write(m_pEnd, sizeof(basicfile_end));

	SetLength(GetPosition());

	m_bNeedRepair = true;


	return ReOpen() == BASIC_FILE_OK;
}

//! zip操作
bool CBasicCombinFile::AddPathToFile(const char* path)
{
	if (IsReadOnly())
		return false;

	basiclib::CBasicSmartBuffer bufIndex;
	bufIndex.SetDataLength(m_pEnd->size_dir);
	Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);
	Read(bufIndex.GetDataBuffer(), m_pEnd->size_dir);

	Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	basicfile_container* dir = &m_Files;
	basiclib::CBasicString strPath;
	__adddir(this, *m_pEnd, path, bufIndex, strPath, dir);

	m_pEnd->size_dir = bufIndex.GetDataLength();
	Write(bufIndex.GetDataBuffer(), m_pEnd->size_dir);
	Write(m_pEnd, sizeof(basicfile_end));
	return true;
}

long CBasicCombinFile::buildZipIndex()
{
	long file_length = GetLength();
	if (file_length < 0)	// 发生错误
		return file_length;

	// 判断文件长度是否合法
	if (file_length < sizeof(basicfile_end))
	{
		if (!IsReadOnly() && file_length == 0)
		{	// 如果是写文件，允许文件长度为0
			return BASIC_FILE_OK;
		}
		else
		{
			Close();
			return BASIC_ZIPFILE_BADZIPFILE;
		}
	}

	Net_UShort buffer_size = 65535;	// uint16
	if (file_length < buffer_size)
		buffer_size = (Net_UShort)file_length - sizeof(basicfile_data_block)-sizeof(basicfile_dir_block);

	long remain_length = file_length - buffer_size;
	Seek(-(long)buffer_size, BASIC_FILE_END);

	basiclib::CBasicStaticBuffer buffer;
	char* end = (char*)buffer.Alloc(buffer_size);
	Read(end, buffer_size);

	char* flag = end + buffer_size - sizeof(basicfile_end)+1;

	while (end <= flag && !((basicfile_end*)flag)->IsBasicFileEndBlock())
		--flag;
	if (flag < end)
	{
		Close();
		return BASIC_ZIPFILE_BADZIPFILE;
	}

	basicfile_end* file_end = (basicfile_end*)flag;
	*m_pEnd = *file_end;
	// 偏移到目录区
	Seek(file_end->offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	basiclib::CBasicStaticBuffer sb;
	Net_UInt extend_length = 0;
	basicfile_dir_block	dir_block;
	Net_UInt diroffset = 0;
	for (int i = 0; i < file_end->file_num_total; ++i)
	{
		Read(&dir_block, sizeof(basicfile_dir_block));

		if (!dir_block.IsBasicFileDirBlock())
		{
			Close();
			return BASIC_ZIPFILE_BADZIPFILE;
		}
		extend_length = dir_block.size_filename + dir_block.size_extend;
		char* file = (char*)sb.Alloc(extend_length);
		Read(file, extend_length);

		char* extend = file + dir_block.size_filename;

		if (dir_block.property_outer_file & property_outer_file_dir)	// is a directory
		{	// 目录
			basicdir_info* info = basiclib::Basic_NewObject<basicdir_info>();
			info->file_name = basiclib::CBasicString(file, dir_block.size_filename - 1);	// 目录去除掉最后一个'/'
			info->zip_type = dir_block.zip_type;
			if (!addFile(info->file_name.c_str(), info, &m_Files))
				basiclib::BASIC_DeleteObject(info);
		}
		else
		{	// 文件
			basicfile_info* info = basiclib::Basic_NewObject<basicfile_info>();
			info->file_name = basiclib::CBasicString(file, dir_block.size_filename);
			info->zip_type = dir_block.zip_type;
			info->size_zipped = dir_block.size_zipped;
			info->size_unzipped = dir_block.size_unzipped;
			info->offset_data_block = dir_block.local_header_offset;
			info->offset_dir_block = diroffset;
			info->crc = dir_block.crc;

			if (!addFile(info->file_name.c_str(), info, &m_Files))
				basiclib::BASIC_DeleteObject(info);
		}
		diroffset += sizeof(basicfile_dir_block)+extend_length;
	}
	return BASIC_FILE_OK;
}


class CBasicCombinFile::BasicExtractor
{
public:
	BasicExtractor(const char* path, CBasicCombinFile* file) : __path(path), __file(file)
	{}

	bool operator()(const pair<basiclib::CBasicString, basicbase_file_info*>& p)
	{
		return __file->extractToFile(__path, p.second);
	}

protected:
	const char*					__path;
	CBasicCombinFile*			__file;
};

bool CBasicCombinFile::ExtractTo(const char* path)
{
	const char* dir = path;

	for_each(m_Files.begin(), m_Files.end(), BasicExtractor(dir, this));
	return true;
}


bool CBasicCombinFile::extractToFile(const char* path, const basicbase_file_info* info)
{
	basiclib::CBasicString filename = path + info->file_name;
	const basicfile_info* file = dynamic_cast<const basicfile_info*>(info);
	const basicdir_info* dir = dynamic_cast<const basicdir_info*>(info);
	if (file)
	{
		basiclib::CBasicSmartBuffer smBuf;
		if (ReadFileFromFileInfo(file, smBuf))
		{
			const char* fullname = filename.c_str();
			basiclib::Basic_mkdir(fullname);
			CBasicFileObj f;
			f.Open(fullname, PF_DISK_FILE | PF_CREATE_ONLY);
			f.Write(smBuf.GetDataBuffer(), smBuf.GetDataLength());
			f.Close();

		}
	}
	else if (dir)
	{
		filename += '/';

		const char* fullname = filename.c_str();
		basiclib::Basic_mkdir(fullname);
		for_each(dir->sub_files.begin(), dir->sub_files.end(), BasicExtractor(path, this));
	}
	return true;
}


bool CBasicCombinFile::repairFile()
{
	bool ret = false;
	basiclib::CBasicString strFilePath = GetFilePath();
	basiclib::CBasicString str = strFilePath + ".repair";
	CBasicFileObj file;
	if (BASIC_FILE_OK == file.Open(str.c_str(), CBasicFileObj::GetFileOpenType()))
	{
		basiclib::CBasicStaticBuffer bufIndex;
		char* p = (char*)bufIndex.Alloc(m_pEnd->size_dir);
		Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);
		Read(p, m_pEnd->size_dir);

		Net_UInt local_header_offset = 0;

		Net_UInt extendlen = 0;
		basiclib::CBasicStaticBuffer tempBuf;
		char* extend = NULL;
		basicfile_data_block datablock;
		basicfile_dir_block* dirblock = (basicfile_dir_block*)p;
		for (int i = 0; i < m_pEnd->file_num_total; ++i)
		{
			// data block
			Seek(dirblock->local_header_offset, BASIC_FILE_BEGIN);
			Read(&datablock, sizeof(basicfile_data_block));
			file.Write(&datablock, sizeof(basicfile_data_block));

			extendlen = datablock.size_filename;
			if (datablock.zip_type == 0)
				extendlen += datablock.size_unzipped;
			else
				extendlen += datablock.size_zipped;

			extend = (char*)tempBuf.Alloc(extendlen);
			Read(extend, extendlen);
			file.Write(extend, extendlen);

			dirblock->local_header_offset = local_header_offset;
			local_header_offset += sizeof(basicfile_data_block)+extendlen;

			dirblock = (basicfile_dir_block*)((char*)(dirblock + 1) + dirblock->size_filename + dirblock->size_extend);
		}

		m_pEnd->offset_dir_to_first_disk = local_header_offset;
		file.Write(bufIndex.GetBuffer(), m_pEnd->size_dir);
		file.Write(m_pEnd, sizeof(basicfile_end));

		CBasicFileObj::Close();

		ret = file.CopyTo(strFilePath.c_str()) == BASIC_FILE_OK;
		file.Close();
		basiclib::Basic_DeleteFile(str.c_str());
		m_bNeedRepair = false;
		ret = true;
	}

	return ret;
}

basicbase_file_info* CBasicCombinFile::deleteFileInfo(const char* file, basicfile_container* dir)
{
	basicbase_file_info* info = NULL;
	basicfile_container::iterator iter = dir->lower_bound(file);
	if (iter != dir->end())
	{
		info = iter->second;
		dir->erase(iter);
	}
	return info;
}

void CBasicCombinFile::getFileStatus(const basicbase_file_info* info, BasicPackFileStatus& status)
{
	status.m_filename = info->file_name;
	const basicfile_info* fileinfo = dynamic_cast<const basicfile_info*>(info);
	if (fileinfo)
	{
		status.m_zipped = fileinfo->size_zipped;
		status.m_unzipped = fileinfo->size_unzipped;
		status.m_crc = fileinfo->crc;
	}
	else
	{
		status.m_attribute = basiclib::_basic_fa_directory;
	}
}


bool CBasicCombinFile::addFile(const char *filename, basicbase_file_info* file, basicfile_container* dir)
{
	const char* sub = strchr(filename, '/');
	if (sub)	// is dir
	{
		if (*(++sub) != 0)	// not end
		{
			basiclib::CBasicString key(filename, sub - filename - 1);
			basicfile_container::iterator iter = dir->lower_bound(key);
			if (iter != dir->end() && iter->first == key)
			{
				basicdir_info* subfile = dynamic_cast<basicdir_info*>(iter->second);
				if (subfile)
					return addFile(sub, file, &subfile->sub_files);
				else
					return false;
			}
			else
			{
				return false;
			}
		}
	}
	(*dir)[filename] = file;
	return true;
}
basiclib::CBasicString CBasicCombinFile::getRootPath(const char* filename)
{
	const char* pathsplit = strchr(filename, '/');
	if (pathsplit)
		return basiclib::CBasicString(filename, pathsplit - filename);

	return basiclib::CBasicString();
}

const char* CBasicCombinFile::getFileName(const char* filename)
{
	const char* pathsplit = strrchr(filename, '/');
	if (pathsplit)
		return pathsplit + 1;
	pathsplit = strrchr(filename, '\\');
	if (pathsplit)
		return  pathsplit + 1;
	return filename;
}

const char* CBasicCombinFile::stepPath(const char* filename)
{
	const char* pathsplit = strchr(filename, '/');
	if (pathsplit)
		return pathsplit + 1;
	pathsplit = strchr(filename, '\\');
	if (pathsplit)
		return pathsplit + 1;
	return "";
}
//合并文件
bool CBasicCombinFile::CombinFileAdd(CBasicCombinFile& combinFile)
{
	if (IsReadOnly())
		return false;

	basiclib::CBasicSmartBuffer bufIndex;
	bufIndex.SetDataLength(m_pEnd->size_dir);
	Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);
	Read(bufIndex.GetDataBuffer(), m_pEnd->size_dir);

	Seek(m_pEnd->offset_dir_to_first_disk, BASIC_FILE_BEGIN);

	basicfile_container* dir = &m_Files;
	basiclib::CBasicString strPath;
	__adddirComBinFile(this, *m_pEnd, &combinFile, &(combinFile.m_Files), bufIndex, strPath, dir);

	m_pEnd->size_dir = bufIndex.GetDataLength();
	Write(bufIndex.GetDataBuffer(), m_pEnd->size_dir);
	Write(m_pEnd, sizeof(basicfile_end));
	return true;
}

BasicPackFileStatus::BasicPackFileStatus()
{
	m_mtime = 0;
	m_zipped = m_unzipped = m_attribute = 0;
}

bool BasicPackFileStatus::IsDirectory() const
{
	return basiclib::_basic_fa_directory == (m_attribute & basiclib::_basic_fa_directory);
}
bool BasicPackFileStatus::IsFile() const
{
	return 0 == (m_attribute & basiclib::_basic_fa_directory);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicPackCombinFile::CBasicPackCombinFile()
{

}

CBasicPackCombinFile::~CBasicPackCombinFile()
{

}
//初始化函数
long CBasicPackCombinFile::InitPackComBinFile(const char* pRelocatePath, const char* lpszFileName)
{
	m_strPathRelocate = pRelocatePath;
	return OpenCombinFile(lpszFileName, PF_DISK_FILE);
}
//获取文件
bool CBasicPackCombinFile::GetFileByName(const char* pFile, basiclib::CBasicSmartBuffer& smBuf)
{
	int nLength = strlen(pFile);
	//处理相对位置
	const char* pPos = strstr(pFile, m_strPathRelocate.c_str());
	if (pPos == NULL)
		return false;
	if (nLength <= m_strPathRelocate.GetLength())
	{
		return false;
	}
	int nAddLength = m_strPathRelocate.GetLength();
	if (*(pPos + nAddLength) == '/')
		nAddLength += 1;
	return GetPackFile(pPos + nAddLength, smBuf);
}

//获取文件，传入相对路径
bool CBasicPackCombinFile::GetFileByRelocateName(const char* pFile, basiclib::CBasicSmartBuffer& smBuf)
{
	return GetPackFile(pFile, smBuf);
}

//合并文件
bool CBasicPackCombinFile::CombinFile(CBasicCombinFile& combinFile)
{
	return CombinFileAdd(combinFile);
}

__NS_BASIC_END

