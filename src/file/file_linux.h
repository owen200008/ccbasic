#ifndef INC_FILELINUX_H
#define INC_FILELINUX_H
/////////////////////////////////////////////////////////////////////////////////////////////
// 文件名:		file_linux.h
// 创建者:		于浩淼
// 创建时间:	2008.10.21
// 内容描述:	文件处理类用到的系统函数在linux下的实现
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
//__NS_BASIC_START
#if (defined(__LINUX) || defined(__MAC) || defined(__ANDROID))
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
/////////////////////////////////////////////////////////////////////////////////////////////
//
#define NO_ERROR 0L 
//文件属性

#define INVALID_FILE_ATTRIBUTES				((DWORD)-1)

#define FILE_ATTRIBUTE_READONLY             0x00000001  
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#define FILE_ATTRIBUTE_DEVICE               0x00000040  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  

#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000  

//文件打开的属性
#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)

#define FILE_SHARE_READ                 0x00000001  
#define FILE_SHARE_WRITE                0x00000002  
#define FILE_SHARE_DELETE               0x00000004  

#ifndef _FILETIME_
#define _FILETIME_
typedef time_t   FILETIME;
#endif // !_FILETIME

typedef struct _WIN32_FIND_DATA
{
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	TCHAR   cFileName[ MAX_PATH ];
	TCHAR   cAlternateFileName[ 14 ];
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;

typedef DWORD	SECURITY_ATTRIBUTES;

#ifndef LPSECURITY_ATTRIBUTES
typedef DWORD   *LPSECURITY_ATTRIBUTES;
#endif

typedef DWORD   *LPOVERLAPPED;

#ifndef LPWIN32_FIND_DATAA
#define LPWIN32_FIND_DATAA LPWIN32_FIND_DATA
#endif

#ifndef WIN32_FIND_DATAA
#define WIN32_FIND_DATAA WIN32_FIND_DATA
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL	CloseHandle(HANDLE hObject);
BOOL	CloseFile(HANDLE hObject);

void	SetLastError(DWORD dwError);
DWORD	GetLastError();

//封装过的 Shell 命令，替换‘’中的空格
int _basic_system(LPCTSTR lpCmd);

//文件操作
HANDLE	CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
				   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
#define CreateFileA CreateFile
BOOL	WriteFile(HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped);
BOOL	ReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped);
BOOL	FlushFileBuffers(HANDLE hFile);
DWORD	SetFilePointer(HANDLE hFile,LONG lDistanceToMove,LONG* lpDistanceToMoveHigh,DWORD dwMoveMethod);
BOOL	SetEndOfFile(HANDLE hFile);

BOOL	MoveFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName);
#define MoveFileA MoveFile
BOOL	CopyFile(LPCTSTR lpExistingFileName,LPCTSTR lpNewFileName,BOOL bFailIfExists);
#define CopyFileA CopyFile
BOOL	DeleteFile(LPCTSTR lpFileName);
#define DeleteFileA DeleteFile
DWORD	GetFullPathName(LPCTSTR lpFileName,DWORD nBufferLength,LPTSTR lpBuffer,LPTSTR *lpFilePart);
#define GetFullPathNameA GetFullPathName

BOOL	SetFileTime(LPCTSTR lpFileName,const FILETIME *lpCreationTime,const FILETIME *lpLastAccessTime, const FILETIME *lpLastWriteTime);
BOOL	GetFileTime(HANDLE hFile,FILETIME* lpCreationTime,FILETIME* lpLastAccessTime,FILETIME* lpLastWriteTime);
DWORD	GetFileSize(HANDLE hFile,LPDWORD lpFileSizeHigh);

BOOL	SetFileAttributes(LPCTSTR lpFileName,DWORD dwFileAttributes);
#define SetFileAttributesA SetFileAttributes
DWORD	GetFileAttributes(LPCTSTR lpFileName);
#define GetFileAttributesA GetFileAttributes

//文件查找
HANDLE	FindFirstFile(LPCTSTR lpFileName,LPWIN32_FIND_DATA lpFindFileData);
BOOL	FindNextFile(HANDLE hFindFile,LPWIN32_FIND_DATA lpFindFileData);
BOOL	FindClose(HANDLE hFindFile);

#ifndef FindFirstFileA
#define FindFirstFileA FindFirstFile
#endif

#ifndef FindNextFileA
#define FindNextFileA FindNextFile
#endif

BOOL	PathMatchSpec(LPCTSTR pszFileParam,LPCTSTR pszSpec);
#define PathMatchSpecA PathMatchSpec


/////////////////////////////////////////////////////////////////////////////////////////////
#endif  //__LINUX __MAC
//__NS_BASIC_END
////////////////////////////////////////////////////////////////////////////////////////////
#endif		//INC_FILELINUX_H
