/***********************************************************************************************
// 文件名:     filenotify.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:42:48
// 内容描述:   定义文件目录监控的类
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_FILENOTIFY_H
#define BASIC_FILENOTIFY_H

//包含头文件
#include <vector>
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#pragma	pack(1)
__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
struct _FILE_NOTIFY_ITEM;					//通知的结构
struct _DIRECTORY_INFO;						//监控的目录

//class CBasicObject;
	class CBasicFileNotify;					//文件监控类
///////////////////////////////////////////////////////////////////////////////////////////////

//! 通知的结构
/*!
* 
*/
struct _FILE_NOTIFY_ITEM		//internal struct
{
	DWORD	m_dwAction;					/*!< 通知的类型，见定义 BASIC_FILE_NOTIFY_CHANGE_* */
	char	m_szFileName[MAX_PATH];		/*!< 通知的文件名（绝对路径） */
	//! 构造函数
	/*!  
	*/
	_FILE_NOTIFY_ITEM()
	{
		memset(this, 0, sizeof(_FILE_NOTIFY_ITEM));
	}
};

//! 通知结构的数组
/*!
* 
*/
typedef  std::vector<_FILE_NOTIFY_ITEM>  _fni_array;

//! 处理通知的回调函数
/*!
* 
*/
typedef long (*Change_Handler)(void* lpVoid, DWORD dwAction, const char* lpszFileName);
////////////////////////////////////////////////////////////////////////
#define DIW_SUBTREE		0x0001			//监控子目录

#define DIW_DEFAULT		DIW_SUBTREE

#define MAX_BUFFER			4096

//! 监控目录的信息
/*!
* 
*/
struct _DIRECTORY_INFO		//internal struct
{
	HANDLE		m_hDir;							/*!< 句柄 */
	char		m_lpszDirName[MAX_PATH];		/*!< 监控的目录 */
	char		m_lpszFileName[64];				/*!< 监控的文件名 */
#ifdef __BASICWINDOWS
	CHAR        m_lpBuffer[MAX_BUFFER];			/*!< 完成端口使用的缓冲区 */
	DWORD       m_dwBufLength;					/*!< 缓冲区大小 */
	OVERLAPPED  m_olOverlapped;					/*!< 完成端口对象 */
#endif
	time_t		m_tmLastChange;					/*!< 文件的最后修改时间 */

	DWORD		m_dwNotifyFilter;				/*!< 过滤选项 BASIC_FILE_NOTIFY_CHANGE_* */
	DWORD		m_dwWathOptions;				/*!< 监控属性 DIW_* */

	void*			m_lpVoid;					/*!< 用户自定义的数据块 */
	Change_Handler	m_pfuncHander;				/*!< 用户自定义的回调函数 */
	
	_fni_array	m_ayFileInfo;					/*!< 通知队列 */

	//! 构造函数 
	/*!  
	*/
	_DIRECTORY_INFO()
	{
#ifdef __BASICWINDOWS
		int nSetLen = (int)(&((_DIRECTORY_INFO*)0)->m_ayFileInfo);
	#else
		int nSetLen = (char*)&m_ayFileInfo-(char*)this;
	#endif
		memset(this, 0, nSetLen);
	}
	
	//! 析构函数 
	/*!  
	*/
	~_DIRECTORY_INFO()
	{
#ifdef __BASICWINDOWS
		if (m_hDir != NULL && m_hDir != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hDir);		//I know this
		}
	#endif
	}
	//! 判断是否要监控子目录
	/*!  
	*/
	BOOL IsWatchSubTree() { return (m_dwWathOptions & DIW_SUBTREE); }
};

typedef  std::vector<_DIRECTORY_INFO*>  _di_array;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BASIC_FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001   /*!< 监控文件名的变化。*/
#define BASIC_FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002   /*!< 监控文件夹的变化。*/
#define BASIC_FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004   /*!< 监控文件名或文件夹的属性变化。*/
#define BASIC_FILE_NOTIFY_CHANGE_SIZE         0x00000008   /*!< 监控文件的大小变化。*/
#define BASIC_FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010   /*!< 监控文件名的最后写的变化。*/
#define BASIC_FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020   /*!< 监控文件名的最后访问变化。*/
#define BASIC_FILE_NOTIFY_CHANGE_CREATION     0x00000040   /*!< 监控文件夹中文件的创建。*/
#define BASIC_FILE_NOTIFY_CHANGE_SECURITY     0x00000100	/*!< 监控文件夹中文件的安全方面变化。*/

#define INVALID_WATCH_ID		0


#define TFN_OPS_MODE		0x000F			//监控模式
#define TFN_OPS_COMPORT		0x0000			//使用完成端口，只能在Windows下使用
#define TFN_OPS_SCANFILE	0x0001			//使用扫描目录的方法监控

////////addtogroup0812

/** @addtogroup CLASS
 *  @{
*/ 
////////addtogroup0812

//! \brief 文件和文件夹监控类
/*!
* 
*/
#pragma warning (push)
#pragma warning (disable: 4251)

class CBasicFileNotify : public CBasicObject
{

public:
	//! 构造函数 
	/*!  
	*\sa <a href = "sample\file_test\CBasicFileNotify_TEST.cpp">CWBasicFileObj_TEST.cpp</a> 
	*/
	CBasicFileNotify(DWORD dwOptions = TFN_OPS_COMPORT);
	
	//! 析构函数 
	/*!  
	*\sa <a href = "sample\file_test\CBasicFileNotify_TEST.cpp">CWBasicFileObj_TEST.cpp</a> 
	*/
	virtual ~CBasicFileNotify();

public:
	//! 增加监控目录 
	/*! 
	*\param lpszPath 监控的路径
	*\param dwNotifyFilter  监控的内容
	*  <ul>监控的内容 
	*     <ol>
	*     <li> BASIC_FILE_NOTIFY_CHANGE_FILE_NAME		监控文件名的变化。
	*     <li> BASIC_FILE_NOTIFY_CHANGE_DIR_NAME		监控文件夹的变化。
	*     <li> BASIC_FILE_NOTIFY_CHANGE_ATTRIBUTES		监控文件名或文件夹的属性变化。
	*     <li> BASIC_FILE_NOTIFY_CHANGE_SIZE			监控文件的大小变化。
	*     <li> BASIC_FILE_NOTIFY_CHANGE_LAST_WRITE		监控文件名的最后写的变化。
	*     <li> BASIC_FILE_NOTIFY_CHANGE_LAST_ACCESS		监控文件名的最后访问变化。
	*     <li> BASIC_FILE_NOTIFY_CHANGE_CREATION		监控文件夹中文件的创建。
	*     <li> BASIC_FILE_NOTIFY_CHANGE_SECURITY		监控文件夹中文件的安全方面变化。
	*     </ol>
	*  </ul>
	*\param dwWatchOps 设置是否监控子目录 默认是 DIW_SUBTREE
	*\param lpVoid  用户自定义的数据块，可以在整个监控过程作为数据传递用.
	*\param pfuncHander 监控所有的回调函数：\n
		typedef long (*Change_Handler)(void* lpVoid, DWORD dwAction, const char* lpszFileName);
	*\return 返回监控的ID，需要保持，用作一下函数的操作使用
	*\sa <a href = "sample\file_test\CBasicFileNotify_TEST.cpp">CWBasicFileObj_TEST.cpp</a> 
	*/
	DWORD	AddWatch(const char* lpszPath, DWORD dwNotifyFilter, DWORD dwWatchOps = DIW_DEFAULT, LPVOID lpVoid = NULL, Change_Handler pfuncHander = NULL);
	
	//! 删除一个监控项目 
	/*! 
	*\param dwWatchID 	监控ID ，AddWatch的返回值.
	*\return 返回TRUE表示输出成功，FALSE失败，可能是ID不存在
	*/
	BOOL	RemoveWatch(DWORD dwWatchID);
	
	//! 停止整个监控任务 
	/*!  
	*/
	void	StopWatch();
	
	//! 检查监控队列中的项目
	/*! 
	*\param dwWatchID 	监控ID ，AddWatch的返回值.
	*\return 返回队列中监控项目的数量 
	*/
	long	CheckNotifyItem(DWORD dwWatchID);
	
	//! 清空监控队列中的项目
	/*! 
	*\param dwWatchID 	监控ID ，AddWatch的返回值.
	*\return 返回队列中监控项目的数量 
	*/
	void	ClearNotifyItem(DWORD dwWatchID);
	
	
	//! 获得并清除第一个监控项目
	/*! 
	*\param dwWatchID 	监控ID ，AddWatch的返回值.
	*\return 返回如下值
	*  <ul>对于 windows系统
	*     <ol>
	*     <li> FILE_ACTION_ADDED		
	*     <li> FILE_ACTION_REMOVED		
	*     <li> FILE_ACTION_MODIFIED		
	*     <li> FILE_ACTION_RENAMED_OLD_NAME			
	*     <li> FILE_ACTION_RENAMED_NEW_NAME		 
	*     </ol>
	*  </ul> 
	*/
	DWORD	PopFirstNotifyItem(DWORD dwWatchID, char* lpszFileName, int nMaxLength);
	
	//! 获得并清除最后一个监控项目
	/*! 
	*\param dwWatchID 	监控ID ，AddWatch的返回值.
	*\return 返回如下值 
	*  <ul>对于 windows系统
	*     <ol>
	*     <li> FILE_ACTION_ADDED		监控文件名的变化。
	*     <li> FILE_ACTION_REMOVED		监控文件夹的变化。
	*     <li> FILE_ACTION_MODIFIED		监控文件名或文件夹的属性变化。
	*     <li> FILE_ACTION_RENAMED_OLD_NAME			监控文件的大小变化。
	*     <li> FILE_ACTION_RENAMED_NEW_NAME		监控文件名的最后写的变化。 
	*     </ol>
	*  </ul> 
	*/
	DWORD	PopLastNotifyItem(DWORD dwWatchID, char* lpszFileName, int nMaxLength);
protected:
	
	static THREAD_RETURN HandleDirectoryChange(LPVOID lpVoid);			/*!< 监控处理线程 */

	void	HandleChangeNotify(_DIRECTORY_INFO* lpdi, const CBasicString& strFileName, DWORD dwAction);  /*!< 监控通知函数 */
	
	void	StartWatch();		/*!< 开始监控 */
	void	EndWatch();			/*!< 停止监控 */
	BOOL	IsThreadRunning() const {return m_eRunFlag == ThreadRun && m_hThread != NULL;}  /*!< 线程是否运行 */
	BOOL	IsValidDirectoryInfo(_DIRECTORY_INFO* lpdi);  /*!< 监控信息是否合法 */

	DWORD	PopNotifyItem(DWORD dwWatchID, char* lpszFileName, int nMaxLength, int nPos);	/*!< 弹出一个通知 */
	
	BOOL	IsCompPort() { return (m_dwNotifyOps & TFN_OPS_MODE) == TFN_OPS_COMPORT; }		/*!< 是否是完成端口的 */
	BOOL	IsScanFile() { return (m_dwNotifyOps & TFN_OPS_MODE) == TFN_OPS_SCANFILE; }		/*!< 是否是扫描目录的 */
protected:
#ifdef __BASICWINDOWS
	void	HandleDirectoryChange_CompPort();		/*!< 完成端口的监控函数 */
#endif
	void	HandleDirectoryChange_ScanDir();			/*!< 扫描目录的处理函数 */
	int		ScanDir(_DIRECTORY_INFO* lpdi, const char* lpszDir, const time_t tmLastChange, time_t& tmCurChange, time_t tmBefore);	/*!< 扫描目录 */
private:
	_di_array	m_ayDirInfo;	/*!<监控目录队列 */
	HANDLE		m_hCompPort;	/*!<句柄 */
	HANDLE		m_hThread;		/*!<线程句柄 */
	DWORD		m_dwNotifyOps;	/*!<监控的属性  */

	enum {ThreadRun, ThreadStop} m_eRunFlag;	/*!<线程状态 */

	CCriticalSection		m_mtxAddRemove;		/*!<操作队列的锁 */
};

#pragma warning (pop)

//////////////////addtogroup0812
/** @} */
//////////////////addtogroup0812


__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma	pack()
#endif 
