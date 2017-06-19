/***********************************************************************************************
// 文件名:     crc32.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2014-1-16 16:36:23
// 内容描述:   定义 CRC16 的算法函数
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_CRC32_H
#define BASIC_CRC32_H


#pragma once
__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明

//!计算内存的CRC32值
/*! 
*\param pszData		 需要计算CRC的数据缓冲区指针
*\return 成功: 返回 计算的CRC
*\remarks 
*\warning
*/

uint32_t _BASIC_DLL_API Basic_crc32(unsigned char* pszData, unsigned int nDataLength);
long _BASIC_DLL_API Basic_crc32_File(const char* lpszFileName, uint32_t &crc);

//! 使用计算文件的CRC32值
/*! 
*\param lpszFileName：文件名
*\param crc32即计算的文件CRC32值
*\return 成功: BASIC_FILE_OK， 失败返回非0 值，参考文件错误
*\remarks 
*\warning
*/
//long Basic_crc32_File(LPCTSTR lpszFileName, unsigned long &crc32);

/////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_END
//////////////////////////////////////////////////////////////////////////////////////////////

#endif 