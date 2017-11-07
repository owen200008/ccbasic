/***********************************************************************************************
// 文件名:     fileobj.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:43:13
// 内容描述:   定义文件处理基类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_FILEOBJ_H
#define BASIC_FILEOBJ_H

#pragma once
__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
//文件打开方式
//所有文件都以只读或读写、共享、二进制方式打开
//缺省方式：创建一个新文件，如果文件存在，内容不变。
#define PF_OPEN_ONLY	0x0001		/*!< 如果文件不存在，打开失败。*/
#define PF_OPEN_TRUN	0x0002		/*!< 如果文件存在，长度置为零。*/
#define PF_READ_ONLY	0x0004		/*!< 打开文件，只读。*/
#define PF_CREATE_ONLY	0x0008		/*!< 如果文件存在，打开失败，否则创建一个新文件。*/
#define PF_MEM_ONLY		0x0010		/*!< 内存文件，不能改变大小*/

//文件类型
#define PF_FILE_MASK	0xF000		/*!< 文件类型*/

#define PF_DISK_FILE	0x2000		/*!< 磁盘文件*/
#define PF_MEM_FILE		0x3000		/*!< 内存文件*/

//class CBasicObject;
	class CBasicFileObj;				//文件处理基类
/////////////////////////////////////////////////////////////////////////////
class CFileBase;
//! 封装了对磁盘文件和内存文件的一系列操作
/*!
* 封装了对文件的一系列操作，包括对文件内容的读写，对文件的拷贝、删除，文件状态的操作
*/
class _BASIC_DLL_API CBasicFileObj : public basiclib::CBasicObject
{
	//BASIC_DECLARE_DYNCREATE(CWBasicFileObj)
public:
	// Constructors  
	//! 构造函数
	/*!
	*  初始化所有的成员变量。
	*/
	CBasicFileObj();

	//! 构造函数 
	/*!
	* 用来创建内存文件对象的构造函数
	*\param pBuffer 用来来初始华内存文件对象的数据块，函数调用完，该对象可以被销毁
	*\param lCount  指明参数1传入数据块的长度
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	CBasicFileObj(void* pBuffer, long lCount);

	//! 构造函数
	/*!
	* 用来创建磁盘文件对象的构造函数，参数参考Open函数说明
	*\param lpszFileName
	*\param dwOpenFlags
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	CBasicFileObj(const char* lpszFileName, DWORD dwOpenFlags);

	//! 析构函数
	/*!
	*/
	virtual ~CBasicFileObj();

	// Attributes
	//! \brief 判断指定的文件是否处于打开状态 
	/*!
	*\return TRUE:表示文件是打开的;FALSE:文件没有打开
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	BOOL IsOpen() const;

	//! \brief 判断指定的文件是否处于禁止写状态
	/*!
	*\return TRUE:表示文件是只读;FALSE:文件非只读,也可能文件没有打开
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	BOOL IsReadOnly() const { return  (m_dwOpenFlags & PF_READ_ONLY); }

	//! \brief 判断指定的文件是否处只打开状态
	/*!
	*\return TRUE:表示文件是OpenOnly;FALSE:文件非OpenOnly,也可能文件没有打开
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	BOOL IsOpenOnly() const { return  (m_dwOpenFlags & PF_OPEN_ONLY); }

	//! \brief 判断指定的文件是否是内存文件
	/*!
	*\return TRUE:表示当前操作的是内存文件;FALSE:非内存文件,也可能文件没有打开
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	BOOL IsMemoryFile() const;

	//! \brief 判断指定的文件是否是只读的内存文件
	/*!
	*\return TRUE:表示当前操作的是只读的内存文件;FALSE:非只读，或者非内存文件，或者没有打开
	*/
	BOOL IsMemoryOnly() const { return  (m_dwOpenFlags & PF_MEM_ONLY); }

	//! \brief 获得文件操作，当前的位置
	/*!
	*\return 返回>=0的当前位置
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	long GetPosition() const;

	//! \brief 获得文件已经打开文件的状态
	/*!
	*获得文件已经打开文件的状态
	*\param rStatus 文件状态的结构
	*\return 如果函数调用成功返回 BASIC_FILE_OK 否则返回一个非0的值(见定义 filedefine.h)
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	long GetStatus(TLFileStatus& rStatus) const;


	//! \brief 获得文件的所在的目录
	/*!
	*\return 如果函数调用成功返回打开文件坐在目录(绝对路径)
	*\remarks 如果是用OpenMemFile打开和用CWBasicFileObj(void* pBuffer, long lCount)构造的对象将返回空
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	CBasicString GetFileDirPath() const;

	//! \brief 获得文件名
	/*!
	*\return 如果函数调用成功返回打开文件的文件名
	*\remarks 如果是用OpenMemFile打开和用CWBasicFileObj(void* pBuffer, long lCount)构造的对象将返回空
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	CBasicString GetFileName() const;

	//! \brief 获得没有后缀的文件名
	/*!
	*\return 如果函数调用成功返回打开文件的文件名
	*\remarks 如果是用OpenMemFile打开和用CWBasicFileObj(void* pBuffer, long lCount)构造的对象将返回空
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	CBasicString GetFileTitle() const;

	//! \brief 获得文件的全路径
	/*!
	*\return 如果函数调用成功返回打开文件的全路径
	*\remarks 如果是用OpenMemFile打开和用CWBasicFileObj(void* pBuffer, long lCount)构造的对象将返回空
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	CBasicString GetFilePath() const;

	//! \brief 判断文件是否合法
	/*!
	*\return 如果函数调用成功返回TRUE，否则返回FALSE。
	*\remarks 派生类可能需要重载这个函数，用于判断有结构的文件是否合法。
	*/
	virtual BOOL IsValidFile() { return TRUE; }
	// Operations
	//! \brief 打开文件
	/*!
	*\param lpszFileName 文件名路径(绝对路径或相对路径)
	*\param dwOpenFlags  文件操作选项
	*  <ul>文件操作选项
	*     <ol>
	*     <li> PF_OPEN_ONLY		如果文件不存在，打开失败 </li>
	*     <li> PF_OPEN_TRUN		如果文件存在，长度置为零</li>
	*     <li> PF_READ_ONLY		打开文件，只读</li>
	*     <li> PF_CREATE_ONLY	如果文件存在，打开失败，否则创建一个新文件</li>
	*     <li> PF_MEM_ONLY		内存文件，不能改变大小</li>
	*     <li> PF_DISK_FILE		磁盘文件</li>
	*     <li> PF_MEM_FILE		如果文件存在，打开失败，否则创建一个新文件</li>
	*     </ol>
	*  </ul>
	*\return 如果函数调用成功返回 BASIC_FILE_OK 否则返回一个非0的值 (见定义 filedefine.h)
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long Open(const char* lpszFileName, DWORD dwOpenFlags);

	//! \brief 打开内存文件
	/*!
	*\param pBuffer 用来来初始华内存文件对象的数据块，函数调用完，该对象可以被销毁
	*\param lCount  指明参数1传入数据块的长度
	*\param lLength 指明分配内存文件的大小，如果lLength < lCount 则 lLength = lCount
	*\return 如果函数调用成功返回 BASIC_FILE_OK 否则返回一个非0的值(见定义 filedefine.h)
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long OpenMemFile(void* pBuffer, long lCount, long lLength = 0);

	//! \brief 重新打开文件
	/*!
	*\return 如果函数调用成功返回 BASIC_FILE_OK 否则返回一个非0的值
	*\remarks 如果是用OpenMemFile打开和用CWBasicFileObj(void* pBuffer, long lCount)构造的对象该调用将失败
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long ReOpen();

	//! \brief 将文件的操作位置设置到最后
	/*!
	*\return 返回当前的位置
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	long SeekToEnd();

	//! \brief 将文件的操作位置设置到开始
	/*!
	*\return 返回当前的位置
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	long SeekToBegin();

	// Overridables
	//! \brief 设置文件的当前位置
	/*!
	*\param lOff  设置相对应参数2的偏移
	*\param nFrom
	*  <ul>
	*     <ol>设置偏移的相对值
	*     <li> BASIC_FILE_BEGIN		文件头
	*     <li> BASIC_FILE_CURRENT		当前位置
	*     <li> BASIC_FILE_END			文件尾
	*     </ol>
	*  </ul>
	*\return 返回当前的位置
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long Seek(long lOff, uint32_t nFrom);

	//! \brief 设置文件的长度
	/*!
	*\param lNewLen  设置文件的新长度
	*\param cFill    空内容的填充值
	*\return 如果函数调用成功返回 BASIC_FILE_OK 否则返回一个非0的值 (错误代码见 filedefine.h)
	*\remarks 如果是以属性 PF_MEM_ONLY 打开的文件，不支持该功能，返回BASIC_FILE_SETLEN_DENIED
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long SetLength(long lNewLen, char cFill = 0);

	//! \brief 获得文件的长度
	/*!
	*\return 返回的长度
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long GetLength();

	//! \brief 获得文件的长度
	/*!
	*\param lpBuf  提前准备的用例存放读出数据的缓存
	*\param lCount 指明缓存的大小
	*\return 大于0返回读出数据的长度，小于0是错误代码（见 filedefine.h）
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long Read(void* lpBuf, long lCount);

	//! \brief 将指定内容写入到文件中
	/*!
	*\param lpBuf  要写入文件的内容的地址
	*\param lCount 要写入文件的内容的大小
	*\param lRepeat 要写入文件的内容需要重复写入的次数，默认是1
	*\param cFill   空内容的填充值,默认是0
	*\return 大于0返回写入数据的长度，小于0是错误代码（见 filedefine.h）
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long Write(const void* lpBuf, long lCount, long lRepeat = 1, char cFill = 0);

	//! \brief 将指定内容插入入到文件中
	/*!
	*\param lpBuf  要写入文件的内容的地址
	*\param lCount 要写入文件的内容的大小
	*\param lRepeat 要写入文件的内容需要重复写入的次数，默认是1
	*\param cFill   空内容的填充值,默认是0
	*\return 大于0返回插入数据的长度，小于0是错误代码（见 filedefine.h）
	*\remarks 该函数的参数和Write很相似，可以配合Seek函数一起使用，指定插入的当前位置
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long Insert(const void* lpBuf, long lCount, long lRepeat = 1, char cFill = 0);

	//! \brief 刷新文件的缓存
	/*!
	*/
	virtual long Flush();

	//! \brief 克隆出一个新的文件操作对象
	/*!
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	//virtual CWBasicFileObj* Duplicate() const;

	//! \brief 关闭文件
	/*!
	*/
	virtual void Close();

	//! \brief 将当前文件的内容拷贝到指定的文件中 
	/*!
	*\remarks 对于内存文件，如果lpszFileName=NULL，相当于当前内存的内容存盘。
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long CopyTo(const char* lpszFileName);

	//! \brief 将指定文件的内容拷贝到当前文件
	/*!
	*\remarks 对于内存文件，如果lpszFileName=NULL，相当于重新从磁盘读入内容到内存。
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	virtual long CopyFrom(const char* lpszFileName);

	//! \brief 将 pFile 的文件内容复制到本对象
	/*!
	*\remarks 两个文件都必须打开。
	*/
	virtual long CopyFromFile(CBasicFileObj* pFile);

	//! \brief 从 pFile 中复制内容到内存文件
	/*!
	*\remarks pFile必须打开，而 this 必须没有打开。
	*/
	virtual long OpenMemFromFile(CBasicFileObj* pFile);

	//! \brief 把本对象复制到内存文件中
	/*!
	*\remarks 本对象必须打开，并且合法。
	*/
	//virtual long CloneToMemFile(CWBasicFileObj*& pNewFile);

	//! \brief 打开一个克隆文件
	/*!
	*\remarks
	*/
	//virtual long OpenCloneFile(LPCTSTR lpszFileName, CWBasicFileObj*& pNewFile, DWORD dwOpenFlags);

	//only for memory files

	//! \brief 获得文件的数据缓存
	/*!
	*\remarks 只有内存文件对象能返回缓存地址，磁盘文件将返回空地址
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	void* GetDataBuffer();

	//! \brief 获得文件的当前数据地址
	/*!
	*\param lCount 设置和返回返回的数据长度
	*\remarks 只有内存文件对象能返回缓存地址，磁盘文件将返回空地址，该函数和文件的当前位置有关，应该和函数Seek配合使用
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	void* GetCurDataBuffer(long& lCount);
public:
	//! \brief 重命名文件
	/*!
	*\param lpszOldName 现有文件的名字(相对路径或绝对路径)
	*\param lpszNewName 需要重新命名的文件名(相对路径或绝对路径)
	*\remarks 该函数是static成员函数，可以不需要构造就可以调用
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	static long Rename(const char* lpszOldName, const char* lpszNewName);

	//! \brief 删除文件
	/*!
	*\param lpszFileName 现有文件的名字(相对路径或绝对路径)
	*\remarks 该函数是static成员函数，可以不需要构造就可以调用
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	static long Remove(const char* lpszFileName);

	//! \brief 删除文件
	/*!
	*\param lpszFileName 现有文件的名字(相对路径或绝对路径)
	*\param rStatus	     函数出参，返回文件的状态
	*\remarks 该函数是static成员函数，可以不需要构造就可以调用
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	static long GetStatus(const char* lpszFileName, TLFileStatus& rStatus);

	//! \brief 删除文件
	/*!
	*\param lpszFileName 现有文件的名字(相对路径或绝对路径)
	*\param rStatus	     需要设置的文件状态
	*\remarks 该函数是static成员函数，可以不需要构造就可以调用
	*\sa <a href = "sample\file_test\CWBasicFileObj_TEST.cpp">CWBasicFileObj_TEST.cpp</a>
	*/
	static long SetStatus(const char* lpszFileName, const TLFileStatus& rStatus);

protected:
	DWORD GetFileOpenType() const { return (m_dwOpenFlags & PF_FILE_MASK); }            /*!< 返回文件打开的文件类型 */
	DWORD GetFileOpenFlags(DWORD dwMask) const { return (m_dwOpenFlags & dwMask); }     /*!< 取打开的属性 PF_* */

	DWORD GetFileOpenFlags() const { return m_dwOpenFlags; }							/*!< 返回全部的属性 */
	void  SetFileOpenFlags(DWORD dwValue, DWORD dwMask)									/*!< 设置文件的属性 */
	{
		m_dwOpenFlags &= ~dwMask;
		m_dwOpenFlags |= (dwValue & dwMask);
	}

	void	ClearMember();		/*!< 清空成员变量 */

	long  CopyFileContent(CBasicFileObj* pFile);		/*!< 复制文件内容 */
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	DWORD			m_dwOpenFlags;		/*!< 文件打开的属性 PF_* */
	CBasicString	m_strFileName;		/*!< 文件名（全路径） */

	CFileBase*		m_pFileObj;		/*!< 文件处理对象 no delete 在函数close() 中删除该对象 */
};
//////////////////addtogroup0812
/** @} */
//////////////////addtogroup0812
__NS_BASIC_END
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif 

