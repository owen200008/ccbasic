/***********************************************************************************************
// 文件名:     mem.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:22:09
// 内容描述:   此文件提高全局的内存分配管理，主要是提高小内存的分配效率
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_MEM_H
#define BASIC_MEM_H

#include <stdlib.h>
#include <stdint.h>

__NS_BASIC_START

typedef uint32_t (*gethandleid_func)();
_BASIC_DLL_API_C  _BASIC_DLL_API void BindGetHandleIDFunc(gethandleid_func func);

//! 分配大小为size的内存
    /*!
	 *\param size 需要分配的内存大小
	 *\return 成功返回分配的内存指针，失败返回NULL
	 *\remarks 此函数分配的内存必须用BasicDeallocate函数来释放
	 *\sa BasicDeallocate()
	 */
_BASIC_DLL_API_C  _BASIC_DLL_API void* BasicAllocate(size_t size);


//! 重新分配内存大小,原来的数据保留
  /*! 
  *\param p 原始的数据指针
  *\param size 新需要分配的长度
  *\return 成功返回新指针地址，删除原来的指针，是否返回NULL，原来的数据保留
  */
_BASIC_DLL_API_C  _BASIC_DLL_API void* BasicReallocate(void* p, size_t size);

//! 释放由BasicAllocate函数分配的内存
   /*!
    *\param p 需要释放的内存指针
	*\param size 内存指针的长度，如果未知，那么不传，传递的目的是提高释放效率
	*\return 无
	*\remarks 当提供内存指针大小的时候，可以加速释放的速度
	*\sa BasicAllocate()
	*/
_BASIC_DLL_API_C  _BASIC_DLL_API void BasicDeallocate(void* p);
_BASIC_DLL_API_C  _BASIC_DLL_API char* BasicStrdup(const char* p);

//!取内存分配操作信息,此操作速度快
	/*!
	 *\param nAllocateCount 总分配次数
	 *\param nDeallocateCount 总释放次数
	 *\param nSumAllocate   总分配的内存大小(不包含内存中额外的数据)
	 *\param nSumDeallocate 总释放的内存大小(不包含内存中额外的数据)	 
	 *\return 无
	 */
_BASIC_DLL_API_C  _BASIC_DLL_API void  BasicGetOperationInfo(
			size_t& nAllocateCount, 
			size_t& nDeallocateCount,
			size_t& nUseMemory,
			size_t& nAllocateSize,
			size_t& nDeAllocateSize
			);

//!设置内存运行模式,必须在启动时候初始化
#define MemRunMemCheck_RunFast				0x00000000
#define MemRunMemCheck_RunSizeCheck			0x00000001
#define MemRunMemCheck_RunTongJi			0x00000002
#define MemRunMemCheck_RunCheckMem			0x00000004		
_BASIC_DLL_API_C  void _BASIC_DLL_API BasicSetMemRunMemCheck(uint32_t nMode = 0, int nMin = 0, int nMax = 0);
_BASIC_DLL_API_C  void _BASIC_DLL_API DumpRunMemCheck();

//!清空之前的内存分配记录
_BASIC_DLL_API_C  void _BASIC_DLL_API BasicEmptyMemAll();

//!输出当前的分配信息
_BASIC_DLL_API_C  void _BASIC_DLL_API BasicShowCurrentMemInfo();
_BASIC_DLL_API_C  long _BASIC_DLL_API BasicGetHandleIDMemInfo(uint32_t nHandleID);

/////////////////////////////////////////////////////////////////////////////////////
//共享内存
//  
//! 分配大小为size的共享内存
/*!
*\param size 需要分配的内存大小 = 0 打开已经存在的共享内存，不存在返回NULL
*       != 0 分配size大小的内存。如果已经存在 返回NULL 
*\param lpszName 共享的名称 =NULL 不共享，只是一个进程使用。!=NULL 进程间共享
*\return 成功返回分配的内存指针，失败返回NULL
*\remarks 此函数分配的内存必须用BasicDeleteShareMem 函数来释放
*         使用函数分配的共享内存，可以和普通内存一样使用，可以在进程中共享。
*\sa BasicDeleteShareMem()
*/
_BASIC_DLL_API_C  _BASIC_DLL_API void* BasicAllocShareMem(size_t size, LPCTSTR lpszName);

//! 释放由BasicAlloShareMem函数分配的内存
/*!
*\param p 需要释放的内存指针
*\return 无
*\remarks 如果输入的 p 不是由 BasicAlloShareMem 分配的，函数直接返回，不做任何处理。
*\sa BasicAlloShareMem()
*/
_BASIC_DLL_API_C  void   _BASIC_DLL_API BasicDeleteShareMem(void* p);

__NS_BASIC_END
#endif 
