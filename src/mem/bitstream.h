/***********************************************************************************************
// 文件名:     smartbuffer.h
// 创建者:     蔡振球
// Email:      zqcai@w.cn
// 创建时间:   2012/2/17 11:23:31
// 内容描述:   内存块管理
// 版本信息:   1.0V
************************************************************************************************/
#ifndef BASIC_BITSTREAMBUFFER_H
#define BASIC_BITSTREAMBUFFER_H



template <typename T>
class Net_Vector : public basiclib::basic_vector<T>
{
};

template <typename KEY, typename VAL>
class Net_Map : public basiclib::basic_map<KEY, VAL>
{
};
template <typename T>
class Net_Set : public basiclib::basic_set<T>
{
};

__NS_BASIC_START

class CNetBasicValue;

_BASIC_DLL_API int SerializeUChar(unsigned char* pBuffer, const uint8_t v);
_BASIC_DLL_API int SerializeUShort(unsigned char* pBuffer, const uint16_t v);
_BASIC_DLL_API int SerializeUInt3Bit(unsigned char* pBuffer, const uint32_t v);
_BASIC_DLL_API int SerializeUInt(unsigned char* pBuffer, const uint32_t v);
_BASIC_DLL_API int SerializeLONGLONG(unsigned char* pBuffer, const int64_t v);
_BASIC_DLL_API int SerializeCBasicString(unsigned char* pBuffer, const int8_t* v, uint16_t usLength);
_BASIC_DLL_API int UnSerializeUChar(unsigned char* pBuffer, uint8_t& v);
_BASIC_DLL_API int UnSerializeChar(unsigned char* pBuffer, int8_t& v);
_BASIC_DLL_API int UnSerializeUShort(unsigned char* pBuffer, uint16_t& v);
_BASIC_DLL_API int UnSerializeShort(unsigned char* pBuffer, int16_t& v);
_BASIC_DLL_API int UnSerializeUInt3Bit(unsigned char* pBuffer, uint32_t& v);
_BASIC_DLL_API int UnSerializeUInt(unsigned char* pBuffer, uint32_t& v);
_BASIC_DLL_API int UnSerializeInt(unsigned char* pBuffer, int32_t& v);
_BASIC_DLL_API int UnSerializeLONGLONG(unsigned char* pBuffer, int64_t& v);
_BASIC_DLL_API int UnSerializeCBasicString(unsigned char* pBuffer, basiclib::CBasicString& str);

class _BASIC_DLL_API CBasicBitstream : public basiclib::CBasicSmartBuffer
{
public:
	CBasicBitstream();
	CBasicBitstream(const string& s);
	CBasicBitstream(void* buf, int size);
	CBasicBitstream(const char* buf, int size);
	virtual ~CBasicBitstream();

	/////////////////////////////////////////////////////////////////////////////
	//串行化
	//字符串操作
	CBasicBitstream& operator << (const uint8_t v);
	CBasicBitstream& operator << (const int8_t v);
	CBasicBitstream& operator << (const uint16_t v);
	CBasicBitstream& operator << (const int16_t v);
	CBasicBitstream& operator << (const uint32_t v);
	CBasicBitstream& operator << (const int32_t v);
	CBasicBitstream& operator << (const int64_t v);
	CBasicBitstream& operator << (const double v);
	CBasicBitstream& operator << (const CNetBasicValue* pV);
	CBasicBitstream& operator << (const CNetBasicValue& v);
	CBasicBitstream& operator << (const basiclib::CBasicString* pV);
	CBasicBitstream& operator << (const basiclib::CBasicSmartBuffer* pV);
	CBasicBitstream& operator << (const int8_t* v);
	CBasicBitstream& operator << (const uint8_t* v);
	CBasicBitstream& operator << (const basiclib::CBasicString& data);
	CBasicBitstream& operator << (const basiclib::CBasicSmartBuffer& pV);
	template<typename A>
	CBasicBitstream& operator << (const Net_Set<A>& data)
	{
		uint16_t uSize = data.size();
		*this << uSize;
		for (auto& key : data)
		{
			*this << key;
		}
		return *this;
	}
	template<class A>
	CBasicBitstream& operator << (const Net_Vector<A>& data)
	{
		uint16_t uSize = (uint16_t)data.size();
		*this << uSize;
		for (auto& key : data)
		{
			*this << key;
		}
		return *this;
	}
	template<class A, class B>
	CBasicBitstream& operator << (const Net_Map<A, B>& data){
		uint16_t uSize = (uint16_t)data.size();
		*this << uSize;
		for (auto& key : data)
		{
			*this << key.first;
			*this << key.second;
		}
		return *this;
	}
	/////////////////////////////////////////////////////////////////////////////
	//反串行化
	//字符串
	CBasicBitstream& operator >> (uint8_t& v);
	CBasicBitstream& operator >> (int8_t& v);
	CBasicBitstream& operator >> (uint16_t& v);
	CBasicBitstream& operator >> (int16_t& v);
	CBasicBitstream& operator >> (uint32_t& v);
	CBasicBitstream& operator >> (int32_t& v);
	CBasicBitstream& operator >> (int64_t& v);
	CBasicBitstream& operator >> (double& v);
	CBasicBitstream& operator >> (basiclib::CBasicString* pV);
	CBasicBitstream& operator >> (basiclib::CBasicSmartBuffer* pV);
	CBasicBitstream& operator >> (CNetBasicValue* pV);
	CBasicBitstream& operator >> (CNetBasicValue& v);
	CBasicBitstream& operator >> (int8_t* v);
	CBasicBitstream& operator >> (uint8_t* v);
	CBasicBitstream& operator >> (basiclib::CBasicString& data);
	CBasicBitstream& operator >> (basiclib::CBasicSmartBuffer& pV);
	template<class A>
	CBasicBitstream& operator >> (Net_Set<A>& data)
	{
		uint16_t uSize = 0;
		*this >> uSize;
		if (uSize > 0)
		{
			for (uint16_t i = 0; i < uSize; i++)
			{
				A intKey;
				*this >> intKey;
				data.insert(intKey);
			}
		}
		return *this;
	}
	template<class A>
	CBasicBitstream& operator >> (Net_Vector<A>& data)
	{
		uint16_t uSize = 0;
		*this >> uSize;
		if (uSize > 0)
		{
			data.resize(uSize);
			for (uint16_t i = 0; i < uSize; i++)
			{
				*this >> data[i];
			}
		}
		return *this;
	}
	template<class A, class B>
	CBasicBitstream& operator >> (Net_Map<A, B>& data){
		uint16_t uSize = 0;
		*this >> uSize;
		for (uint16_t i = 0; i < uSize; i++)
		{
			A intKey;
			*this >> intKey;
			*this >> data[intKey];
		}
		return *this;
	}
	/////////////////////////////////////////////////////////////////////////////
	void SerializeDataBuffer(const int8_t* pData, uint16_t usLength);
	/////////////////////////////////////////////////////////////////////////////
	void UnSerializeCString(basiclib::CBasicString* pV);
	void UnSerializeSmbuf(basiclib::CBasicSmartBuffer* pV);
protected:
	unsigned char m_szBuf[8];			//最大编码8字节整型
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double TINY_VALUE = (1E-10);

const char		g_strDoubleNull[8] = { (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0x80 };
const double	DTEDOUBLE_NULL = *(double*)g_strDoubleNull;

#define DTE_CHAR_NULL		"NUL"			//字符串空值
#define DTE_LONG_NULL		LONG_MIN		//整数空值
#define DTE_DATETIME_NULL	SS_MINTIME		//日期时间空值
#define DTE_COLOR_NULL		0x7fffffff		//颜色COLORREF

#define IsColorNull(x)		((x) == DTE_COLOR_NULL)
#define SetColorNull(x)		(x) = DTE_COLOR_NULL

#define IsDoubleNullExt(x)		((((x)==basiclib::DTEDOUBLE_NULL)) && ((char*)(&x))[7] == (char)0x80)
#define IsDoubleNull(x)			(((x)==NULL) || (((*(x)==basiclib::DTEDOUBLE_NULL)) && ((char*)(x))[7] == (char)0x80))
#define SetDoubleNull(x)			x=basiclib::DTEDOUBLE_NULL

#define	IsLongNull(x)			(x==DTE_LONG_NULL)
#define SetLongNull(x)			x=DTE_LONG_NULL

#define	IsTimeNull(x)			(x == DTE_DATETIME_NULL)
#define SetTimeNull(x)			x = DTE_DATETIME_NULL

const unsigned char NETVALUE_NULL = 0;
const unsigned char NETVALUE_CHAR = 1;
const unsigned char NETVALUE_STRING = 2;
const unsigned char NETVALUE_LONG = 3;
const unsigned char NETVALUE_DOUBLE = 4;
const unsigned char NETVALUE_LONGLONG = 5;

class _BASIC_DLL_API CNetBasicValue : public basiclib::CBasicObject
{
public:
	CNetBasicValue();
	~CNetBasicValue();
	CNetBasicValue(const double& value);
	CNetBasicValue(const int64_t& value);
	CNetBasicValue(const int32_t value);
	CNetBasicValue(const char* value, size_t len = NullLen);
	CNetBasicValue(const CNetBasicValue& value);

	void			SetLong(const int32_t value);
	void			SetDouble(const double& value);
	void			SetLongLong(const int64_t& value);
	void			SetString(const char* value, size_t len = NullLen);

	int32_t			GetLong() const;
	double			GetDouble() const;
	int64_t	GetLongLong() const;
	CBasicString	GetString() const;
	const char*		GetStringRef() const;

	uint8_t		GetDataType() const{ return m_type; }
	long			GetDataLength() const;

	bool			IsNull() const;
	void			SetNull();
	bool			IsString() const;

	// data comparison
	int CompareBasicValue(const CNetBasicValue* pRhs) const;
	int CompareInt(int32_t rhs) const;
	int CompareDouble(const double& rhs) const;
	int CompareLongLong(int64_t rhs) const;
	int ComparePointString(const char* rhs, size_t len) const;

	// assignment
	const CNetBasicValue& operator = (const CNetBasicValue& rhs);
	const CNetBasicValue& operator = (const int32_t rhs);
	const CNetBasicValue& operator = (const double& rhs);
	const CNetBasicValue& operator = (const int64_t& rhs);
	const CNetBasicValue& operator = (const char* rhs);
	// add
	const CNetBasicValue& operator += (const CNetBasicValue& rhs);
	const CNetBasicValue& operator += (const int32_t rhs);
	const CNetBasicValue& operator += (const double& rhs);
	const CNetBasicValue& operator += (const int64_t& rhs);
	const CNetBasicValue& operator += (const char* rhs);
	// sub
	const CNetBasicValue& operator -= (const CNetBasicValue& rhs);
	const CNetBasicValue& operator -= (const int32_t rhs);
	const CNetBasicValue& operator -= (const double& rhs);
	const CNetBasicValue& operator -= (const int64_t& rhs);
	// multi
	const CNetBasicValue& operator *= (const CNetBasicValue& rhs);
	const CNetBasicValue& operator *= (const int32_t rhs);
	const CNetBasicValue& operator *= (const double& rhs);
	const CNetBasicValue& operator *= (const int64_t& rhs);
	// div
	const CNetBasicValue& operator /= (const CNetBasicValue& rhs);
	const CNetBasicValue& operator /= (const int32_t rhs);
	const CNetBasicValue& operator /= (const double& rhs);
	const CNetBasicValue& operator /= (const int64_t& rhs);

	//序列化
	void SeriazeSMBuf(basiclib::CBasicSmartBuffer& smBuf) const;
	void UnSeriazeSMBuf(basiclib::CBasicSmartBuffer& smBuf);

	int Seriaze(char* pData, int nLength);
	int UnSeriaze(const char* pData, int nLength);
	//获取序列化长度
	int GetSeriazeLength() const;
protected:
	void			toData(int32_t& lValue) const;
	void			toData(double& fValue) const;
	void			toData(int64_t& llValue) const;

	void	Release();
	void	EmptyValue();
	void	AddString(const char* lpStr);
	static	CBasicString DataToString(uint16_t nDataType, const void* value);
protected:
	uint8_t				m_type;
	union{
		int32_t				m_lValue[2];
		int64_t				m_llValue;
		double				m_dValue;
		char				m_cValue[8];
		CBasicString*		m_pBufValue;
	};
};

inline CNetBasicValue operator+(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	CNetBasicValue res(lhs);
	res += rhs;
	return res;
}

inline CNetBasicValue operator-(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	CNetBasicValue res(lhs);
	res -= rhs;
	return res;
}

inline CNetBasicValue operator*(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	CNetBasicValue res(lhs);
	res *= rhs;
	return res;
}

inline CNetBasicValue operator/(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	CNetBasicValue res(lhs);
	res /= rhs;
	return res;
}

inline bool operator>(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) > 0;
}
inline bool operator>(const CNetBasicValue& lhs, const int32_t rhs)
{
	return lhs.CompareInt(rhs) > 0;
}

inline bool operator>=(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) >= 0;
}
inline bool operator>=(const CNetBasicValue& lhs, const int32_t rhs)
{
	return lhs.CompareInt(rhs) >= 0;
}

inline bool operator<(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) < 0;
}
inline bool operator<(const CNetBasicValue& lhs, const int32_t rhs)
{
	return lhs.CompareInt(rhs) < 0;
}
inline bool operator<=(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) <= 0;
}
inline bool operator<=(const CNetBasicValue& lhs, const int32_t rhs)
{
	return lhs.CompareInt(rhs) <= 0;
}

inline bool operator==(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) == 0;
}
inline bool operator==(const CNetBasicValue& lhs, const int32_t rhs)
{
	return lhs.CompareInt(rhs) == 0;
}
inline bool operator!=(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) != 0;
}
inline bool operator!=(const CNetBasicValue& lhs, const int32_t rhs)
{
	return lhs.CompareInt(rhs) != 0;
}

__NS_BASIC_END
/////////////////////////////////////////////////////////////////////////////////////////////
//结构定义
typedef uint8_t						Net_UChar;											//1个字节
typedef int8_t						Net_Char;											//1个字节
typedef uint16_t					Net_UShort;											//无符号2字节
typedef int16_t						Net_Short;											//2字节
typedef uint32_t					Net_UInt;											//无符号4字节
typedef int32_t						Net_Int;											//4字节
typedef int64_t						Net_LONGLONG;										//8字节
typedef double						Net_Double;											//8字节
typedef intptr_t					Net_PtrInt;											//same with point size
typedef basiclib::CBasicString		Net_CBasicString;                                   //define the cstring
typedef basiclib::CNetBasicValue	Net_CNetBasicValue;                                 //define the cstring
typedef basiclib::CBasicBitstream	Net_CBasicBitstream;                                //define the cstring
//////////////////////////////////////////////////////////////////////////////////////////////////
//支持序列化的map和vector定义
typedef Net_Vector<Net_Int>													VTNetInt;
typedef VTNetInt::iterator													VTNetIntIterator;
typedef VTNetInt::const_iterator											VTNetIntIteratorConst;
typedef Net_Vector<Net_UInt>				                                VTNetUInt;
typedef VTNetUInt::iterator													VTNetUIntIterator;
typedef VTNetUInt::const_iterator											VTNetUIntIteratorConst;
typedef Net_Map<Net_Int, Net_Int>											MapNetIntToInt;
typedef MapNetIntToInt::iterator											MapNetIntToIntIterator;
typedef MapNetIntToInt::const_iterator										MapNetIntToIntIteratorConst;
typedef Net_Map<Net_UInt, Net_Int>											MapNetUIntToInt;
typedef MapNetUIntToInt::iterator											MapNetUIntToIntIterator;
typedef MapNetUIntToInt::const_iterator										MapNetUIntToIntIteratorConst;
typedef Net_Map<Net_UInt, Net_UInt>					                        MapNetUIntToUInt;
typedef MapNetUIntToUInt::iterator											MapNetUIntToUIntIterator;
typedef MapNetUIntToUInt::const_iterator									MapNetUIntToUIntIteratorConst;

#endif 
