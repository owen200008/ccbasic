/***********************************************************************************************
// 文件名:     loaddll.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012-2-21 23:25:28
// 内容描述:   调用动态库接口，封装一些API函数
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_LOADDLL_H
#define BASIC_LOADDLL_H

#pragma once
__NS_BASIC_START
/////////////////////////////////////////////////////////////////////////////////////////////
//声明
////////////////////////////////////////////////////////////////////////////////////////
class CBasicLoadDll : public CBasicObject
{
public:
	CBasicLoadDll();
	CBasicLoadDll(const char* lpszLibFileName);
	~CBasicLoadDll();
public:
	long LoadLibrary(const char* lpszLibFileName);
	long FreeLibrary();
	void* GetProcAddress(const char* lpszProcName);
protected:
	basiclib::CBasicString	m_strLoadFileName;
	void*					m_hModule;
};
////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_END

#endif 
