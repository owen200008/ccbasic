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


typedef basiclib::CBasicString		Net_CBasicString;                                   //define the cstring
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

__NS_BASIC_START

#pragma pack(1)

class CNetBasicValue;

_BASIC_DLL_API int SerializeUChar(unsigned char* pBuffer, const Net_UChar v);
_BASIC_DLL_API int SerializeUShort(unsigned char* pBuffer, const Net_UShort v);
_BASIC_DLL_API int SerializeUInt(unsigned char* pBuffer, const Net_UInt v);
_BASIC_DLL_API int SerializeLONGLONG(unsigned char* pBuffer, const Net_LONGLONG v);
_BASIC_DLL_API int SerializeCBasicString(unsigned char* pBuffer, const Net_Char* v, Net_UShort usLength);
_BASIC_DLL_API int UnSerializeUChar(unsigned char* pBuffer, Net_UChar& v);
_BASIC_DLL_API int UnSerializeChar(unsigned char* pBuffer, Net_Char& v);
_BASIC_DLL_API int UnSerializeUShort(unsigned char* pBuffer, Net_UShort& v);
_BASIC_DLL_API int UnSerializeShort(unsigned char* pBuffer, Net_Short& v);
_BASIC_DLL_API int UnSerializeUInt(unsigned char* pBuffer, Net_UInt& v);
_BASIC_DLL_API int UnSerializeInt(unsigned char* pBuffer, Net_Int& v);
_BASIC_DLL_API int UnSerializeLONGLONG(unsigned char* pBuffer, Net_LONGLONG& v);
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
	CBasicBitstream& operator << (const Net_UChar v);
	CBasicBitstream& operator << (const Net_Char v);
	CBasicBitstream& operator << (const Net_UShort v);
	CBasicBitstream& operator << (const Net_Short v);
	CBasicBitstream& operator << (const Net_UInt v);
	CBasicBitstream& operator << (const Net_Int v);
	CBasicBitstream& operator << (const Net_LONGLONG v);
	CBasicBitstream& operator << (const Net_Double v);
	CBasicBitstream& operator << (const CNetBasicValue* pV);
	CBasicBitstream& operator << (const CNetBasicValue& v);
	CBasicBitstream& operator << (const Net_CBasicString* pV);
	CBasicBitstream& operator << (const basiclib::CBasicSmartBuffer* pV);
	CBasicBitstream& operator << (const Net_Char* v);
	CBasicBitstream& operator << (const Net_UChar* v);
	CBasicBitstream& operator << (const Net_CBasicString& data);
	CBasicBitstream& operator << (const basiclib::CBasicSmartBuffer& pV);
	template<typename A>
	CBasicBitstream& operator << (const Net_Set<A>& data)
	{
		Net_UShort uSize = data.size();
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
		Net_UShort uSize = data.size();
		*this << uSize;
		for (auto& key : data)
		{
			*this << key;
		}
		return *this;
	}
	template<class A, class B>
	CBasicBitstream& operator << (const Net_Map<A, B>& data){
		Net_UShort uSize = data.size();
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
	CBasicBitstream& operator >> (Net_UChar& v);
	CBasicBitstream& operator >> (Net_Char& v);
	CBasicBitstream& operator >> (Net_UShort& v);
	CBasicBitstream& operator >> (Net_Short& v);
	CBasicBitstream& operator >> (Net_UInt& v);
	CBasicBitstream& operator >> (Net_Int& v);
	CBasicBitstream& operator >> (Net_LONGLONG& v);
	CBasicBitstream& operator >> (Net_Double& v);
	CBasicBitstream& operator >> (Net_CBasicString* pV);
	CBasicBitstream& operator >> (basiclib::CBasicSmartBuffer* pV);
	CBasicBitstream& operator >> (CNetBasicValue* pV);
	CBasicBitstream& operator >> (CNetBasicValue& v);
	CBasicBitstream& operator >> (Net_Char* v);
	CBasicBitstream& operator >> (Net_UChar* v);
	CBasicBitstream& operator >> (Net_CBasicString& data);
	CBasicBitstream& operator >> (basiclib::CBasicSmartBuffer& pV);
	template<class A>
	CBasicBitstream& operator >> (Net_Set<A>& data)
	{
		Net_UShort uSize = 0;
		*this >> uSize;
		if (uSize > 0)
		{
			for (Net_UShort i = 0; i < uSize; i++)
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
		Net_UShort uSize = 0;
		*this >> uSize;
		if (uSize > 0)
		{
			data.resize(uSize);
			for (Net_UShort i = 0; i < uSize; i++)
			{
				*this >> data[i];
			}
		}
		return *this;
	}
	template<class A, class B>
	CBasicBitstream& operator >> (Net_Map<A, B>& data){
		Net_UShort uSize = 0;
		*this >> uSize;
		for (Net_UShort i = 0; i < uSize; i++)
		{
			A intKey = 0;
			*this >> intKey;
			*this >> data[intKey];
		}
		return *this;
	}
	/////////////////////////////////////////////////////////////////////////////
	void SerializeDataBuffer(const Net_Char* pData, Net_UShort usLength);
	/////////////////////////////////////////////////////////////////////////////
	void UnSerializeCString(Net_CBasicString* pV);
	void UnSerializeSmbuf(basiclib::CBasicSmartBuffer* pV);
protected:
	unsigned char m_szBuf[8];			//最大编码8字节整型
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double TINY_VALUE = (1E-10);

const char		g_strDoubleNull[8] = { (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0, (char)0x80 };
const double	DTE_DOUBLE_NULL = *(double*)g_strDoubleNull;

#define DTE_CHAR_NULL		"NUL"			//字符串空值
#define DTE_LONG_NULL		LONG_MIN		//整数空值
#define DTE_DATETIME_NULL	SS_MINTIME		//日期时间空值
#define DTE_COLOR_NULL		0x7fffffff		//颜色COLORREF

#define IsColorNull(x)		((x) == DTE_COLOR_NULL)
#define SetColorNull(x)		(x) = DTE_COLOR_NULL

#define IsDoubleNullExt(x)		((((x)==basiclib::DTE_DOUBLE_NULL)) && ((char*)(&x))[7] == (char)0x80)
#define IsDoubleNull(x)			(((x)==NULL) || (((*(x)==basiclib::DTE_DOUBLE_NULL)) && ((char*)(x))[7] == (char)0x80))
#define SetDoubleNull(x)		x=basiclib::DTE_DOUBLE_NULL

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
	CNetBasicValue(const Net_Double& value);
	CNetBasicValue(const Net_LONGLONG& value);
	CNetBasicValue(const Net_Int value);
	CNetBasicValue(const char* value, size_t len = NullLen);
	CNetBasicValue(const CNetBasicValue& value);

	void			SetLong(const Net_Int value);
	void			SetDouble(const Net_Double& value);
	void			SetLongLong(const Net_LONGLONG& value);
	void			SetString(const char* value, size_t len = NullLen);

	Net_Int			GetLong() const;
	Net_Double		GetDouble() const;
	Net_LONGLONG	GetLongLong() const;
	CBasicString	GetString() const;
	const char*		GetStringRef() const;

	Net_UChar		GetDataType() const{ return m_type; }
	long			GetDataLength() const;

	bool			IsNull() const;
	void			SetNull();
	bool			IsString() const;

	// data comparison
	int CompareBasicValue(const CNetBasicValue* pRhs) const;
	int CompareInt(Net_Int rhs) const;
	int CompareDouble(const Net_Double& rhs) const;
	int CompareLongLong(Net_LONGLONG rhs) const;
	int ComparePointString(const char* rhs, size_t len) const;

	// assignment
	const CNetBasicValue& operator = (const CNetBasicValue& rhs);
	const CNetBasicValue& operator = (const Net_Int rhs);
	const CNetBasicValue& operator = (const Net_Double& rhs);
	const CNetBasicValue& operator = (const Net_LONGLONG& rhs);
	const CNetBasicValue& operator = (const char* rhs);
	// add
	const CNetBasicValue& operator += (const CNetBasicValue& rhs);
	const CNetBasicValue& operator += (const Net_Int rhs);
	const CNetBasicValue& operator += (const Net_Double& rhs);
	const CNetBasicValue& operator += (const Net_LONGLONG& rhs);
	const CNetBasicValue& operator += (const char* rhs);
	// sub
	const CNetBasicValue& operator -= (const CNetBasicValue& rhs);
	const CNetBasicValue& operator -= (const Net_Int rhs);
	const CNetBasicValue& operator -= (const Net_Double& rhs);
	const CNetBasicValue& operator -= (const Net_LONGLONG& rhs);
	// multi
	const CNetBasicValue& operator *= (const CNetBasicValue& rhs);
	const CNetBasicValue& operator *= (const Net_Int rhs);
	const CNetBasicValue& operator *= (const Net_Double& rhs);
	const CNetBasicValue& operator *= (const Net_LONGLONG& rhs);
	// div
	const CNetBasicValue& operator /= (const CNetBasicValue& rhs);
	const CNetBasicValue& operator /= (const Net_Int rhs);
	const CNetBasicValue& operator /= (const Net_Double& rhs);
	const CNetBasicValue& operator /= (const Net_LONGLONG& rhs);

	//序列化
	void SeriazeSMBuf(basiclib::CBasicSmartBuffer& smBuf) const;
	void UnSeriazeSMBuf(basiclib::CBasicSmartBuffer& smBuf);

	int Seriaze(char* pData, int nLength);
	int UnSeriaze(const char* pData, int nLength);
	//获取序列化长度
	int GetSeriazeLength() const;
protected:
	void			toData(Net_Int& lValue) const;
	void			toData(Net_Double& fValue) const;
	void			toData(Net_LONGLONG& llValue) const;

	void	Release();
	void	EmptyValue();
	void	AddString(const char* lpStr);
	static	CBasicString DataToString(Net_UShort nDataType, const void* value);
protected:
	Net_UChar				m_type;
	union{
		Net_Int				m_lValue[2];
		Net_LONGLONG		m_llValue;
		Net_Double			m_dValue;
		Net_Char			m_cValue[8];
		Net_CBasicString*	m_pBufValue;
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
inline bool operator>(const CNetBasicValue& lhs, const Net_Int rhs)
{
	return lhs.CompareInt(rhs) > 0;
}

inline bool operator>=(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) >= 0;
}
inline bool operator>=(const CNetBasicValue& lhs, const Net_Int rhs)
{
	return lhs.CompareInt(rhs) >= 0;
}

inline bool operator<(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) < 0;
}
inline bool operator<(const CNetBasicValue& lhs, const Net_Int rhs)
{
	return lhs.CompareInt(rhs) < 0;
}
inline bool operator<=(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) <= 0;
}
inline bool operator<=(const CNetBasicValue& lhs, const Net_Int rhs)
{
	return lhs.CompareInt(rhs) <= 0;
}

inline bool operator==(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) == 0;
}
inline bool operator==(const CNetBasicValue& lhs, const Net_Int rhs)
{
	return lhs.CompareInt(rhs) == 0;
}
inline bool operator!=(const CNetBasicValue& lhs, const CNetBasicValue& rhs)
{
	return lhs.CompareBasicValue(&rhs) != 0;
}
inline bool operator!=(const CNetBasicValue& lhs, const Net_Int rhs)
{
	return lhs.CompareInt(rhs) != 0;
}


#pragma pack()
__NS_BASIC_END

typedef basiclib::CNetBasicValue		Net_CNetBasicValue;                                   //define the cstring
//////////////////////////////////////////////////////////////////////////////////////////////////


#endif 
