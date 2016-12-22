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
class _BASIC_DLL_API CBasicLoadDll : public CBasicObject
{
public:
	CBasicLoadDll();
	CBasicLoadDll(const char* lpszLibFileName);
	~CBasicLoadDll();
public:
	long LoadLibrary(const char* lpszLibFileName);
	long FreeLibrary();
	void* GetProcAddress(const char* lpszProcName);
	basiclib::CBasicString& GetLibName(){return m_strLoadFileName;}

	/*相同动态库替换，使用规则,支持win32
	1.两个动态库都不能free不然会崩溃
	2.只替换导出的函数，包括全局函数，成员函数，静态成员函数
	3.尽量不要使用全局变量，静态成员变量，这些都无法继承，从默认值开始
	*/
	bool ReplaceDll(CBasicLoadDll& dll, const std::function<void(const char* pLog)>& logFunc);
protected:
	basiclib::CBasicString	m_strLoadFileName;
	void*					m_hModule;
};
////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_END

#endif 
