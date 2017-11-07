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
			file_container::iterator iter = dir->find(key);
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
	file_container::iterator iter = dir->find(key);
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
	m_end.size_comment = (uint16)m_strComment.length();

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
	block.size_filename = (uint16)filename.length();
	Write(&block, sizeof(zip_file_data_block));
	Write(filename.c_str(), block.size_filename);

	// write index data
	zip_file_dir_block dirblock;
	dirblock.modify_date = block.modify_date;
	dirblock.modify_time = block.modify_time;
	dirblock.property_outer_file = property_outer_file_dir;	// path
	dirblock.local_header_offset = m_end.offset_dir_to_first_disk;
	dirblock.size_filename = block.size_filename;
	dirblock.size_comment = (uint16)dirinfo->comment.length();
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
	block.size_filename = (uint16)fileinfo->file_name.length();
	block.zip_type = 0x08;
	block.size_unzipped = dataBuffer->GetLength();

	zip_file_dir_block dirblock;
	dirblock.modify_date = block.modify_date;
	dirblock.modify_time = block.modify_time;
	dirblock.size_filename = block.size_filename;
	dirblock.zip_type = block.zip_type;
	dirblock.size_comment = (uint16)fileinfo->comment.length();
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

__NS_BASIC_END

