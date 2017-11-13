/***********************************************************************************************
// 文件名:     functor.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:24:28
// 内容描述:   一些常用的仿函数。用于简化程序处理
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_FUNCTOR_H
#define BASIC_FUNCTOR_H

#pragma once


#include "../inc/basic_def.h"
#include "auto_cast.h"
#include "strutil/strutil.h"

#if defined(__GNUC__)
#include <memory>
#include <utility>
#else
#include <xmemory>
#endif


__NS_BASIC_START

//!用于删除容器类中对象的仿函数类。
/*!
用法如下：<br/>
\code
typedef vector<object*> object_container;
object_container con;
...
for_each(con.begin(), con.end(), DeleteObject());
con.clear();
\endcode
*/
class DeleteObject
{
public:
	template<class T>
	void operator()(T* obj)
	{
		obj->~T();
		BasicDeallocate(obj);
	}

	template<class T, class U>
	void operator()(std::pair<T, U> obj)
	{
		this->operator ()(obj.second);
	}
};

//!用于删除容器类中对象的仿函数类。
/*!
用法如下：<br/>
\code
typedef vector<object*> object_container;
object_container con;
...
for_each(con.begin(), con.end(), DeleteObjectAux());
con.clear();
\endcode
*/
class DeleteObjectAux
{
public:
	template<class T>
	void operator()(T* obj)
	{
		delete obj;
	}

	template<class T, class U>
	void operator()(std::pair<T, U> obj)
	{
		this->operator ()(obj.second);
	}
};

//! 用于将字符串插入到数据容器中的仿函数
/*!
例子：<br/>
\date 2008-03-15
\code
TCHAR buf[] = "abc-def-ghi-jkl-mno";
CWBasicStringArray ay;
IntoContaner<CWBasicStringArray>	ic(ay);
SpliteString(buf, '-', ic);
cout<<CombineString('|', ay.begin(), ay.end())<<endl;
\endcode
输出为:
\code
abc|def|ghi|jkl|mno
\endcode
*/
template<class Container>
class WIntoContainer
{
	typedef typename Container::value_type	value_type;
public:
	//! 构造函数
	/*!
	\param con [out]目标容器，必须为支持LPCTSTR 参数以及push_back的容器。
	\param acceptEmpty [in]对于空字符串的处理方式。true: 加入到容器，false不处理。
	*/
	WIntoContainer(Container& con, bool acceptEmpty = false)
		: __con(con), __acceptEmpty(acceptEmpty){}

	void operator()(const TCHAR* lpsz)
	{
		if (__acceptEmpty || !IsStringEmpty(lpsz))
			__con.push_back(auto_cast<value_type>(lpsz));
	}
protected:
	Container&	__con;
	bool		__acceptEmpty;
};

template<class Container>
class IntoContainer_s
{
	typedef typename Container::value_type	value_type;
public:
	//! 构造函数
	/*!
	\param con [out]目标容器，必须为支持LPCTSTR 参数以及push_back的容器。
	\param acceptEmpty [in]对于空字符串的处理方式。true: 加入到容器，false不处理。
	*/
	IntoContainer_s(Container& con, bool acceptEmpty = false)
		: __con(con), __acceptEmpty(acceptEmpty){}

	void operator()(const char* lpsz)
	{
		if (__acceptEmpty || !IsStringEmpty(lpsz))
			__con.push_back(auto_cast_s<value_type>(lpsz));
	}
protected:
	Container&	__con;
	bool		__acceptEmpty;
};

//! 用于将字符串插入到已经分配号数据的容器中
template<class T>
class IntoArray
{
public:
	IntoArray(T* first, size_t& count, bool acceptEmpty = false)
		: __first(first), __count(count), __acceptEmpty(acceptEmpty), __last(first + count)
	{
		__count = 0;
	}

	void operator()(LPCTSTR lpsz)
	{
		if (__first < __last && (__acceptEmpty || !IsStringEmpty(lpsz)))
		{
			*__first++ = auto_cast<T>(lpsz);
			++ __count;
		}
	}
protected:
	T*	__first;
	T*	__last;
	size_t&	__count;
	bool	__acceptEmpty;
};


//! 用于解析类似于a=b&c=d&e=f的字符串，并将结果放到map中
/*!
\date 2008-03-15
\code
string ps = "a=b&c=d&e=f";
map<string, string> ms;
ParseParamString(ps.c_str(), "=&", WIntoMapContainer<map<tstring, tstring> >(ms));
\endcode
*/
template<class Container>
class IntoMapContainer_s
{
	typedef typename Container::key_type	key_type;
	typedef typename Container::mapped_type	mapped_type;
public:
	IntoMapContainer_s(Container& con)
		: __con(con){}

	void operator()(char* key, char* value)
	{
		if (key && value)
		{
			//key_type k = auto_cast_s<key_type>(key);
			//mapped_type v = auto_cast_s<mapped_type>(value);
			key_type k = key;
			mapped_type v = value;
			__con[k] = v;
		}
	}
protected:
	Container&	__con;
};

template<class Container>
class WIntoMapContainer
{
	typedef typename Container::key_type	key_type;
	typedef typename Container::mapped_type	mapped_type;
public:
	WIntoMapContainer(Container& con)
		: __con(con){}

	void operator()(LPTSTR key, LPTSTR value)
	{
		if (key && value)
		{
			key_type k = auto_cast<key_type>(key);
			mapped_type v = auto_cast<mapped_type>(value);
			__con[k] = v;
		}
	}
protected:
	Container&	__con;
};


template<class T>
bool IsThisClass(T cClass, T* pClass, int nClassCount)
{
	return find(pClass, pClass + nClassCount, cClass) != pClass + nClassCount;
}

template<class CharType, class T>
size_t ChangeStrCodeToByte(const CharType* lpszMarket, T* pszMarket, size_t nCount, CharType cTok)
{
	BasicSpliteString(lpszMarket, cTok, IntoArray<T>(pszMarket, nCount));
	return nCount;
}

template<class CharType, class T>
typename __BasicString<CharType>::StringType ChangeByteCodeToStr(T* pszMarket, int nCount, CharType cTok)
{
	if(nCount <= 0)
	{
		T* eos = pszMarket;
		while( *eos++ ) ;

		nCount = eos - pszMarket - 1;
	}

	return Basic_CombineString(cTok, pszMarket, pszMarket + nCount);
}


__NS_BASIC_END



#endif 
