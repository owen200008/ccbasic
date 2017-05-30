/***********************************************************************************************
// 文件名:     fileman.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:42:26
// 内容描述:   定义文件管理的一些全局函数和类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_FILEMAN_H
#define BASIC_FILEMAN_H

#pragma once

__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
//class CBasicObject;
	class CBasicFileFind;				//文件查找

/////////////////////////////////////////////////////////////////////////////////////////////
#define WSTR_INVALID(x) (x == NULL || x[0] == _T('\0'))					//!< 判断字符串指针
#define STR_INVALID(x) (x == NULL || x[0] == '\0')					//!< 判断字符串指针


//<a class="el" href="fileman_8h-source.html">fileman.h</a>第<a class="el" href="fileman_8h-source.html#l00253">253</a>行定义。

//! 取文件全路径
/*! 
*\param lpszPathOut 一个指向接受输出结果的内存指针，输出全路径
*\param lpszFileIn 输入相对路径
*\return 成功: BASIC_FILE_OK；失败: 返回 BASIC_FILE_* 定义在文件 filedefine.h；
*\remarks 
*\warning 1、不能检测全路径是否存在 2、与当前工作目录相关
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API Basic_GetFileFullPath(char* lpszPathOut, const char* lpszFileIn);

//! 获得文件状态
/*! 
*\param lpszFileName 文件路径
*\param rStatus 一个指向接受输出结果的内存指针，文件状态TLFileStatusW结构
*\return 成功: BASIC_FILE_OK；失败:  返回 BASIC_FILE_* 定义在文件 filedefine.h；
*\remarks 
*\warning 
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API Basic_GetFileStatus(const char* lpszFileName, TLFileStatus& rStatus);
//! 获得文件名
/*! 
*\param lpszPathName 文件路径
*\param lpszName 一个指向接受输出结果的内存指针，输出文件名
*\param nMax 指定第二个参数(lpszName)的长度，如果实际要返回文件名超出这个长度，则失败并返回实际要返回的文件名长度
*\return 成功: BASIC_FILE_OK；失败: 返回实际要返回的文件名长度；
*\remarks 
*\warning 第二个参数不能为NULL
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/	
long _BASIC_DLL_API Basic_GetFileName(const char* lpszPathName, char* lpszName, int nMax);

//! 获得不带扩展名的文件名
/*! 
*\param lpszPathName 文件路径
*\param lpszTitle 一个指向接受输出结果的内存指针，输出不带扩展名的文件名
*\param nMax 指定第二个参数(lpszTitle)的长度，如果实际要返回文件名超出这个长度，则失败并返回实际要返回的文件名长度
*\return 成功: BASIC_FILE_OK；失败: 返回实际要返回的文件名长度；
*\remarks 
*\warning 
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API Basic_GetFileTitle(const char* lpszPathName, char* lpszTitle, int nMax);
//! 从全路径文件名中，取得路径
/*! 
*\param lpszPathName 文件路径
*\param lpszDirPath 一个指向接受输出结果的内存指针，输出文件所在的文件夹路径
*\param nMax 指定第二个参数(lpszDirPath)的长度，最大为_MAX_PATH，如果实际要返回文件名超出这个长度，则失败并返回_MAX_PATH
*\return 成功: BASIC_FILE_OK；失败: 返回_MAX_PATH
*\remarks 
*\warning 第二个参数不能为NULL
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API Basic_GetFileDirPath(const char* lpszPathName, char* lpszDirPath, int nMax);
//! 取文件属性
/*! 
*\param lpszFileName 文件路径
*\return 成功: 返回一个标识文件或者文件夹属性的DWORD；失败: 通过GetFileErrorID()获得错误信息；
*\remarks 
*\warning 
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
DWORD _BASIC_DLL_API WBasic_GetFileAttributes(LPCTSTR lpszFileName);
DWORD _BASIC_DLL_API Basic_GetFileAttributes(const char* lpszFileName);

//! 设置文件状态
/*! 
*\param lpszFileName 文件路径
*\param status 一个指向文件状态结构TLFileStatusW的常指针
*\return 成功: BASIC_FILE_OK；失败: 通过GetFileErrorID()获得错误信息；
*\remarks 
*\warning 
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>//!!!等待再析
*/
long _BASIC_DLL_API WBasic_SetFileStatus(LPCTSTR lpszFileName, const TLFileStatusW& rStatus);
long _BASIC_DLL_API Basic_SetFileStatus(const char* lpszFileName, const TLFileStatus& rStatus);
//! 重命名文件或者文件夹
/*! 
*\param lpszOldName 要被重命名的文件或者文件夹的名字
*\param lpszNewName 新文件或者文件夹的名字
*\return 成功: BASIC_FILE_OK；失败: 通过GetFileErrorID()获得错误信息；
*\remarks 实际调用WinAPI的MoveFile(yhm??)
*\warning 1、新文件或文件夹不能已经存在 2、新文件可以在不同的文件系统下，但是新文件夹必须在与老文件夹在相同的驱动盘下
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API WBasic_RenameFile(LPCTSTR lpszOldName, LPCTSTR lpszNewName);
long _BASIC_DLL_API Basic_RenameFile(const char* lpszOldName, const char* lpszNewName);
//! 删除文件
/*! 
*\param lpszFileName 要被删除的文件名
*\return 成功: BASIC_FILE_OK；失败: 通过GetFileErrorID()获得错误信息；
*\remarks 实际调用WinAPI的DeleteFile
*\warning 1、不可删除只读文件
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API WBasic_DeleteFile(LPCTSTR lpszFileName);
long _BASIC_DLL_API Basic_DeleteFile(const char* lpszFileName);
//! 复制文件
/*! 
*\param lpExistingFileName 已经存在的文件
*\param lpNewFileName 新文件
*\param bFailIfExists FALSE: 如果文件存在，覆盖；TRUE: 如果文件存在，不覆盖
*\return 成功: BASIC_FILE_OK；失败: 通过GetFileErrorID()获得错误信息；
*\remarks 实际调用WinAPI的CopyFile
*\warning 
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API WBasic_CopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists);
long _BASIC_DLL_API Basic_CopyFile(const char* lpExistingFileName, const char* lpNewFileName, BOOL bFailIfExists);

//! 判断是否通配符
/*! 
*\param pszFile 被用来搜索匹配的路径字符串，最长MAX_PATH
*\param pszSpec 通配符'*','?'比如要找WORD文件，写成"*.doc"最长MAX_PATH
*\return 成功:TRUE；失败:FALSE
*\remarks 实际调用WinAPI的PathMatchSpec
*\warning 
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
BOOL _BASIC_DLL_API WBasic_PathMatchSpec(LPCTSTR pszFile, LPCTSTR pszSpec);
BOOL _BASIC_DLL_API Basic_PathMatchSpec(const char* pszFile, const char* pszSpec);
//! 取得文件名指针
/*! 
*\param lpszPathName 文件路径
*\return 文件名指针
*\remarks 返回的文件名指针只是在输入参数 lpszPathName 中的偏移，同时支持 '/' '\\'
*\warning 
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/	
_BASIC_DLL_API char* Basic_FindFileName(const char* lpszPathName);

//! 创建目录
/*! 
*\param lpszPath 要创建的目录名
*\return 成功: BASIC_FILE_OK；失败: BASIC_FILE_MKDIR_ERROR；
*\remarks 输入参数 lpszPath 同时支持'/'和'\\'。查找最后一个'/'或'\\'，这个字符前面的内容作为需要建立的目录。后面的内容看作是文件名，不创建目录。
*\remarks 如要创建目录 basic，要输入 d:\\basic\\ 或者 d:/basic/
*\warning 
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API Basic_mkdir(const char* lpszPath);
long _BASIC_DLL_API WBasic_mkdir(LPCTSTR lpszPath);

_BASIC_DLL_API char* Basic_TempFileName(const char* lpszDir, const char* lpszHead, const char* lpszExt, char* lpszBuffer, int nMax);

#define BASIC_FO_MOVE           0x0001			//!< 文件移动
#define BASIC_FO_COPY           0x0002			//!< 文件复制
#define BASIC_FO_DELETE         0x0003			//!< 文件删除
#define BASIC_FO_RENAME         0x0004			//!< 文件改名

//! 文件目录操作
/*! 
*\param wFunc 文件操作选项
*  <ul>
*  <li> 文件操作选项
*     <ol>
*     <li> BASIC_FO_MOVE		文件移动
*     <li> BASIC_FO_COPY		文件复制
*     <li> BASIC_FO_DELETE		文件删除
*     <li> BASIC_FO_RENAME		文件改名
*     </ol>
*  </ul>
*\param pFrom 原文件或者目录名
*\param pTo 目的文件或者目录名
*\return 
*\remarks 
*\warning  支持删除单个文件
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API Basic_FileOperation(uint32_t wFunc, const char* pFrom, const char* pTo);

#define BASIC_FIND_SUBDIR			0x0001			//!< 查找子目录
#define BASIC_FIND_DIR				0x0002			//!< 返回目录的名称

//! 查找目录里面的所有文件
/*! 
*\param lpszFilePath 指定要查找的文件路径
*\param lpszFileName 通配符
*\param dwFindMode 查找的选项 见定义：BASIC_FIND_* 。BASIC_FIND_SUBDIR:查找子目录  BASIC_FIND_DIR：返回目录的名称
*\param f 模板函数，对查找到的文件进行处理。函数原型 long func(LPCTSTR lpszFileName, time_t tmCreateTime, time_t tmModifyTime, long lFileLength, BYTE cAttr)
          cAttr 文件的属性，见定义 enum TLFileAttribute 
*\return 返回此路径下的符合条件的文件数
*\remarks 如果 lpszFilePath 有文件名，参数 lpszFileName 无效。如果要查找全部的问题，lpszFileName = NULL
*\warning
*/

long _BASIC_DLL_API Basic_FindAllFileInPath(const char* lpszFilePath, const char* lpszFileName, DWORD dwFindMode, const std::function<long(const char*, time_t, time_t, long, BYTE)>& f);

//! 查找目录里面的所有文件
/*! 
*\param lpszFilePath 指定要查找的文件路径
*\param lpszFileName 通配符
*\param ayFile 一个CWBasicStringArray指针
*\param dwFindMode 查找的选项 见定义：BASIC_FIND_* 。BASIC_FIND_SUBDIR:查找子目录
*\return 返回此路径下的符合条件的文件数
*\remarks 如果 lpszFilePath 有文件名，参数 lpszFileName 无效。如果要查找全部的问题，lpszFileName = NULL
*\warning
*\sa <a href = "sample\file_test\fileman_TEST.cpp">fileman_TEST.cpp</a>
*/
long _BASIC_DLL_API Basic_FindAllFileInPath(const char* lpszFilePath, const char* lpszFileName, CBasicStringArray& ayFile, DWORD dwFindMode);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class _BASIC_DLL_API CBasicFileFind : public CBasicObject
{
public:
	//! 构造函数
	/*!
	*  初始化所有成员变量
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	CBasicFileFind();
	//! 析构函数
	/*!
	*  释放分配的内存
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	virtual ~CBasicFileFind();

	// Attributes
public:

	//! 获得文件长度
	/*!
	*  返回一个ULONGLONG型的文件长度
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	ULONGLONG GetLength() const;

	//! 获得文件名
	/*!
	*  返回一个CWBasicString型的字符串
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	CBasicString GetFileName() const;


	//! 获得文件所在的文件夹路径
	/*!
	*  返回一个CWBasicString型的字符串
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	CBasicString GetFilePath() const;

	//! 获得不带文件扩展名的文件名
	/*!
	*  返回一个CWBasicString型的字符串
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	CBasicString GetFileTitle() const;

	//! 获得网络文件的URL
	/*!
	*  返回一个CWBasicString型的字符串
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	CBasicString GetFileURL() const;

	//! 获得文件所在的目录，不是根目录
	/*!
	*  返回一个CWBasicString型的字符串
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	CBasicString GetRoot() const;

	//! 获得文件最后修改时间
	/*!
	*  返回一个time_t型时间
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	time_t GetLastWriteTime() const;

	//! 获得文件最后访问时间
	/*!
	*  返回一个time_t型时间
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	time_t GetLastAccessTime() const;

	//! 获得文件创建时间
	/*!
	*  返回一个time_t型时间
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	time_t GetCreationTime() const;

	///////group0812
	/** @name 文件属性判断
	*/
	//@{
	///////group0812

	//! 匹配文件属性
	/*!
	*  返回TURE: 具有此属性；返回FALSE: 不具有此属性；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL MatchesMask(DWORD dwMask) const;

	//! 文件是否是目录
	/*!
	*  返回TURE: 是目录；返回FALSE: 不是目录；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsDots() const;

	//! 文件是否只读
	/*!
	*  返回TURE: 只读；返回FALSE: 非只读；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsReadOnly() const;

	//! 文件是否是目录
	/*!
	*  返回TURE: 是目录；返回FALSE: 不是目录；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsDirectory() const;

	//! 文件是否压缩
	/*!
	*  返回TURE: 是；返回FALSE: 否；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsCompressed() const;

	//! 文件是否是系统文件
	/*!
	*  返回TURE: 是；返回FALSE: 否；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsSystem() const;

	//! 文件是否隐藏
	/*!
	*  返回TURE: 是；返回FALSE: 否；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsHidden() const;

	//! 文件是否是临时文件
	/*!
	*  返回TURE: 是；返回FALSE: 否；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsTemporary() const;

	//! 文件是否有一般属性，即不具备其他任何属性
	/*!
	*  返回TURE: 是；返回FALSE: 否；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsNormal() const;

	//! 文件是否归档属性
	/*!
	*  返回TURE: 是；返回FALSE: 否；
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	BOOL IsArchived() const;

	///////group0812
	//@}
	///////group0812


	///////group0812
	/** @name 文件操作Operations
	*/
	//@{
	///////group0812

	// Operations

	//! 关闭文件查找句柄
	/*!
	*  关闭文件查找句柄
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	void Close();

	//! 查找文件
	/*!
	*\param lpszFilePath 文件路径
	*\param lpszFileName 文件名，如果=NULL，表示查找所有的文件。如果 lpszFilePath 有文件名，该参数无效。
	*\return 成功: BASIC_FILE_OK；失败: 通过GetFileErrorID()获得错误信息；
	*\remarks
	*\warning
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	long FindFile(const char* lpszFilePath, const char* lpszFileName = NULL);

	//! 查找下一个文件
	/*!
	*  在调用FindFile后，需要调用这个函数，之后才能调用其他函数
	*\sa <a href = "sample\file_test\CBasicFileFindW_TEST.cpp">CBasicFileFindW_TEST.cpp</a>
	*/
	long FindNextFile();

	///////group0812
	//@}
	///////group0812

protected:
	void CloseContext();		//!< 关闭查找上下文

protected:
	void* m_pFoundInfo;			//!< 已经查找到的文件信息
	void* m_pNextInfo;			//!< 下一个文件信息
	HANDLE m_hContext;			//!< 文件搜索句柄
	CBasicString m_strRoot;		//!< 文件目录路径
};
//////////////////addtogroup0812
/** @} */
//////////////////addtogroup0812

//路径分隔符定义
#ifdef __BASICWINDOWS
#define WIDEPATHSPLIT 				_T('\\')		//单个字符
#define PATHSPLIT_S					'\\'
#define PATHSPLITSTRING_S			"\\"			//字符串
#else
#define WIDEPATHSPLIT 				_T('/')			//单个字符
#define PATHSPLIT_S 				'/'			//单个字符
#define PATHSPLIT					PATHSPLIT_S
#define PATHSPLITSTRING_S			"/"				//字符串
#define PATHSPLIT_OTHER				'\\'		//单个字符
#define PATHSPLITSTRING_OTHER		"\\"		//字符串
#endif

//! 格式化路径
/*! 
*\param strPath 路径
*/
void _BASIC_DLL_API Basic_RegulatePathString(CBasicString& strPath);

//! 格式化文件名
/*! 
*\param strFileName 文件名
*/
void _BASIC_DLL_API Basic_RegulateFileNameString(CBasicString& strFileName);
__NS_BASIC_END
/////////////////////////////////////////////////////////////////////////////////////////////

#endif 
