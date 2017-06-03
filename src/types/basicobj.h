/***********************************************************************************************
// 文件名:     basicobj.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 8:41:24
// 内容描述:   基础类型库文件
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_BASICOBJ_H
#define BASIC_BASICOBJ_H

//本文件定义的所有类、结构、函数等的声明和派生关系
///////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
class CBasicObject;
///////////////////////////////////////////////////////////////////////////
//
/*! \class CBasicObject
 * \brief CBasicObject class.
 *
 * basiclib基础类定义.
 */

class _BASIC_DLL_API CBasicObject
{
public:
	CBasicObject();
	virtual ~CBasicObject();

	// Diagnostic allocations
	void* operator new(size_t nSize);
	void* operator new(size_t nSize, void* pPoint);
	void operator delete(void* p);
	void operator delete(void* p, void* pPont);
};
__NS_BASIC_END

#endif

