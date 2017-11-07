/***********************************************************************************************
// 文件名:     auto_cast.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 12:25:09
// 内容描述:   实现一个自动判别型别(主要是字符串和POD数据类型之间的)类型转换的类.
对于一些特例，可以自行特例化auto_cast_impl来实现。
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_AUTO_CAST_H
#define BASIC_AUTO_CAST_H


#include "../inc/basic.h"
#include "strutil/strutil.h"
#include "private.h"

__NS_BASIC_START

template<typename Source, typename Target, class OneString>
Target auto_cast_impl(const Source& rhs, tstringstream* pstream, __private::Type2Type<Target>, OneString)
{
	Target t;
	tstringstream stream;
	if (NULL == pstream)
		pstream = &stream;
	*pstream << rhs;
	*pstream >> t;
	return t;
}

template<typename Source>
unsigned char auto_cast_impl(const Source& rhs, tstringstream* pstream, __private::Type2Type<unsigned char> _1, __private::True _2)
{
	unsigned short ret = auto_cast_impl(rhs, pstream, __private::Type2Type<unsigned short>(), _2);
	return (unsigned char)ret;
}



template<typename Source, typename Target>
Target auto_cast_impl(const Source& rhs, tstringstream* pstream, __private::Type2Type<Target>, __private::False)
{
	return Target(rhs);
}


template<typename Source>
Source auto_cast_aux(const Source& rhs, tstringstream* pstream, __private::Type2Type<Source>)
{
	return rhs;
}


template<typename Source, typename Target>
Target auto_cast_aux(const Source& rhs, tstringstream* pstream, __private::Type2Type<Target>)
{
	using namespace basiclib::__private;
	typedef typename IsString<Target>::Result	IsTargetString;
	typedef typename IsString<Source>::Result	IsSourceString;
	typedef typename XorOp<IsTargetString, IsSourceString>::Result	HasOneStringType;
	return auto_cast_impl(rhs, pstream, Type2Type<Target>(), HasOneStringType());
}


template<typename Target, typename Source>
Target auto_cast(const Source& rhs, tstringstream* pstream = NULL)
{
	return auto_cast_aux(rhs, pstream, __private::Type2Type<Target>());
};

template<typename Source, typename Target, class OneString>
Target auto_cast_impl_s(const Source& rhs, tsstringstream_s* pstream, __private::Type2Type<Target>, OneString)
{
	Target t;
	tsstringstream_s stream;
	if (NULL == pstream)
		pstream = &stream;
	*pstream << rhs;
	*pstream >> t;
	return t;
}

template<typename Source>
unsigned char auto_cast_impl_s(const Source& rhs, tsstringstream_s* pstream, __private::Type2Type<unsigned char> _1, __private::True _2)
{
	unsigned short ret = auto_cast_impl_s(rhs, pstream, __private::Type2Type<unsigned short>(), _2);
	return (unsigned char)ret;
}

template<typename Source, typename Target>
Target auto_cast_impl_s(const Source& rhs, tsstringstream_s* pstream, __private::Type2Type<Target>, __private::False)
{
	return Target(rhs);
}

template<typename Source>
Source auto_cast_aux_s(const Source& rhs, tsstringstream_s* pstream, __private::Type2Type<Source>)
{
	return rhs;
}

template<typename Source, typename Target>
Target auto_cast_aux_s(const Source& rhs, tsstringstream_s* pstream, __private::Type2Type<Target>)
{
	using namespace basiclib::__private;
	typedef typename IsString<Target>::Result	IsTargetString;
	typedef typename IsString<Source>::Result	IsSourceString;
	typedef typename XorOp<IsTargetString, IsSourceString>::Result	HasOneStringType;
	return auto_cast_impl_s(rhs, pstream, Type2Type<Target>(), HasOneStringType());
}

template<typename Target, typename Source>
Target auto_cast_s(const Source& rhs, tsstringstream_s* pstream = NULL)
{
	return auto_cast_aux_s(rhs, pstream, __private::Type2Type<Target>());
};

__NS_BASIC_END

#endif

