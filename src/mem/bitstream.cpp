#include "../inc/basic.h"

__NS_BASIC_START

inline int SerializeUChar(unsigned char* pBuffer, const Net_UChar v)
{
	pBuffer[0] = v;
	return 1;
}

inline int SerializeUShort(unsigned char* pBuffer, const Net_UShort v)
{
	pBuffer[0] = (v)& 0xFF;
	pBuffer[1] = (v >> 8) & 0xFF;
	return 2;
}

inline int SerializeUInt(unsigned char* pBuffer, const Net_UInt v)
{
	pBuffer[0] = (v)& 0xFF;
	pBuffer[1] = (v >> 8) & 0xFF;
	pBuffer[2] = (v >> 16) & 0xFF;
	pBuffer[3] = (v >> 24) & 0xFF;
	return 4;
}

inline int SerializeLONGLONG(unsigned char* pBuffer, const Net_LONGLONG v)
{
	pBuffer[0] = (v)& 0xFF;
	pBuffer[1] = (v >> 8) & 0xFF;
	pBuffer[2] = (v >> 16) & 0xFF;
	pBuffer[3] = (v >> 24) & 0xFF;
	pBuffer[4] = (v >> 32) & 0xFF;
	pBuffer[5] = (v >> 40) & 0xFF;
	pBuffer[6] = (v >> 48) & 0xFF;
	pBuffer[7] = (v >> 56) & 0xFF;
	return 8;
}
inline int SerializeCBasicString(unsigned char* pBuffer, const Net_Char* v, Net_UShort usLength)
{
	int nRetSize = 0;
	nRetSize = SerializeUShort(pBuffer, usLength);
	if (usLength > 0)
	{
		memcpy(pBuffer + nRetSize, v, usLength);
	}
	return nRetSize + usLength;
}

inline int UnSerializeUChar(unsigned char* pBuffer, Net_UChar& v)
{
	v = pBuffer[0];
	return 1;
}
inline int UnSerializeChar(unsigned char* pBuffer, Net_Char& v)
{
	v = pBuffer[0];
	return 1;
}

inline int UnSerializeUShort(unsigned char* pBuffer, Net_UShort& v)
{
	v = pBuffer[0] | pBuffer[1] << 8;
	return 2;
}
inline int UnSerializeShort(unsigned char* pBuffer, Net_Short& v)
{
	v = pBuffer[0] | pBuffer[1] << 8;
	return 2;
}
inline int UnSerializeUInt(unsigned char* pBuffer, Net_UInt& v)
{
	v = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
	return 4;
}
inline int UnSerializeInt(unsigned char* pBuffer, Net_Int& v)
{
	v = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
	return 4;
}
inline int UnSerializeLONGLONG(unsigned char* pBuffer, Net_LONGLONG& v)
{
	Net_LONGLONG vHigh = pBuffer[4] << 0 | pBuffer[5] << 8 | pBuffer[6] << 16 | pBuffer[7] << 24;
	Net_LONGLONG vLow = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
	v = vLow | vHigh << 32;
	return 8;
}
inline int UnSerializeCBasicString(unsigned char* pBuffer, basiclib::CBasicString& str)
{
	int nRet = 0;
	Net_UShort usLength = 0;
	nRet = UnSerializeUShort(pBuffer, usLength);
	if (usLength > 0){
		char* lpszData = str.GetBufferSetLength(usLength);
		memcpy(lpszData, pBuffer + nRet, usLength);
	}
	return nRet + usLength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CNetBasicValue::CNetBasicValue()
{
	EmptyValue();
}
CNetBasicValue::~CNetBasicValue()
{
	Release();
}

CNetBasicValue::CNetBasicValue(const Net_Double& value)
{
	EmptyValue();
	SetDouble(value);
}

CNetBasicValue::CNetBasicValue(const Net_LONGLONG& value)
{
	EmptyValue();
	SetLongLong(value);
}

CNetBasicValue::CNetBasicValue(const Net_Int value)
{
	EmptyValue();
	SetLong(value);
}

CNetBasicValue::CNetBasicValue(const char* value, size_t len)
{
	EmptyValue();
	SetString(value, len);
}

CNetBasicValue::CNetBasicValue(const CNetBasicValue& value)
{
	EmptyValue();
	*this = value;
}

void CNetBasicValue::EmptyValue()
{
	memset(this, 0, sizeof(CNetBasicValue));
}
void CNetBasicValue::Release()
{
	switch (m_type)
	{
	case NETVALUE_STRING:
	{
		delete m_pBufValue;
		m_pBufValue = NULL;
	}
	break;
	}
	EmptyValue();
}

void CNetBasicValue::SetLong(const Net_Int value)
{
	Release();
	m_type = NETVALUE_LONG;
	m_lValue[0] = value;
}

void CNetBasicValue::SetDouble(const Net_Double& value)
{
	Release();
	m_type = NETVALUE_DOUBLE;
	m_dValue = value;
}

void CNetBasicValue::SetLongLong(const Net_LONGLONG& value)
{
	Release();
	m_type = NETVALUE_LONGLONG;
	memcpy(&m_llValue, &value, sizeof(Net_LONGLONG));
}

void CNetBasicValue::SetString(const char* value, size_t len)
{
	Release();
	if (len == NullLen)
		len = __tcslen(value);

	if (len < sizeof(double) / sizeof(char))
	{
		m_type = NETVALUE_CHAR;
		memcpy(m_cValue, value, len * sizeof(char));
		m_cValue[len] = 0;
	}
	else
	{
		m_type = NETVALUE_STRING;
		m_pBufValue = new Net_CBasicString(value, len);
	}
}

Net_Int CNetBasicValue::GetLong() const
{
	Net_Int ret = DTE_LONG_NULL;
	toData(ret);
	return ret;
}
Net_Double CNetBasicValue::GetDouble() const
{
	double ret = DTE_DOUBLE_NULL;
	toData(ret);
	return ret;
}

Net_LONGLONG CNetBasicValue::GetLongLong() const
{
	Net_LONGLONG ret = 0;
	toData(ret);
	return ret;
}

CBasicString CNetBasicValue::GetString() const
{
	CBasicString ret;

	switch (m_type)
	{
	case NETVALUE_LONG:
	case NETVALUE_DOUBLE:
	case NETVALUE_LONGLONG:
		if (!IsNull())
			ret = DataToString(m_type, (void*)m_cValue);
		break;
	case NETVALUE_CHAR:
		ret = m_cValue;
		break;
	case NETVALUE_STRING:
		ret = *m_pBufValue;
		break;
	}
	return ret;
}
const char* CNetBasicValue::GetStringRef() const
{
	const char* pRet = nullptr;
	switch (m_type)
	{
	case NETVALUE_CHAR:
		pRet = m_cValue;
		break;
	case NETVALUE_STRING:
		pRet = m_pBufValue->c_str();
		break;
	}
	return pRet;
}


void CNetBasicValue::toData(Net_Int& lValue) const
{
	lValue = DTE_LONG_NULL;
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			lValue = m_lValue[0];
			break;
		case NETVALUE_DOUBLE:
			lValue = (Net_Int)m_dValue;
			break;
		case NETVALUE_LONGLONG:
			lValue = (Net_Int)m_llValue;
			break;
		case NETVALUE_CHAR:
			sscanf(m_cValue, "%ld", &lValue);
			break;
		case NETVALUE_STRING:
			sscanf(m_pBufValue->c_str(), "%ld", &lValue);
			break;
		}
	}
}


void CNetBasicValue::toData(Net_Double& fValue) const
{
	fValue = DTE_DOUBLE_NULL;
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			fValue = (double)m_lValue[0];
			break;
		case NETVALUE_DOUBLE:
			fValue = m_dValue;
			break;
		case NETVALUE_LONGLONG:
			fValue = (double)m_llValue;
			break;
		case NETVALUE_CHAR:
			sscanf(m_cValue, "%lf", &fValue);
			break;
		case NETVALUE_STRING:
			sscanf(m_pBufValue->c_str(), "%lf", &fValue);
			break;
		}
	}
}

void CNetBasicValue::toData(Net_LONGLONG& llValue) const
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			llValue = m_lValue[0];
			break;
		case NETVALUE_LONGLONG:
			memcpy(&llValue, &m_llValue, sizeof(Net_LONGLONG));
			break;
		case NETVALUE_DOUBLE:
			llValue = (Net_LONGLONG)m_dValue;
			break;
		case NETVALUE_CHAR:
		{
#ifdef __BASICWINDOWS
			char szBuf[16] = "%I64d";
#else
			char szBuf[16] = "%lld";
#endif
			sscanf(m_cValue, szBuf, &llValue);
		}
		break;
		case NETVALUE_STRING:
		{
#ifdef __BASICWINDOWS
			char szBuf[16] = "%I64d";
#else
			char szBuf[16] = "%lld";
#endif
			sscanf(m_pBufValue->c_str(), szBuf, &llValue);
		}
		break;
		}
	}
}
long CNetBasicValue::GetDataLength() const
{
	if (IsString())
	{
		unsigned short nType = GetDataType();
		if (nType == NETVALUE_CHAR)
			return __tcslen(m_cValue);

		return m_pBufValue->GetLength();
	}
	return sizeof(CNetBasicValue);
}

bool CNetBasicValue::IsNull() const
{
	bool ret = false;
	switch (m_type)
	{
	case NETVALUE_NULL:
		ret = true;
		break;
	case NETVALUE_LONG:
		ret = (m_lValue[0] == DTE_LONG_NULL);
		break;
	case NETVALUE_DOUBLE:
		ret = IsDoubleNullExt(m_dValue);
		break;
	case NETVALUE_LONGLONG:
		break;
	case NETVALUE_CHAR:
		ret = (m_cValue[0] == 0);
		break;
	case NETVALUE_STRING:
		ret = (m_pBufValue == NULL);
		break;
	}
	return ret;
}

void CNetBasicValue::SetNull()
{
	Release();
	m_type = NETVALUE_NULL;
}

bool CNetBasicValue::IsString() const
{
	unsigned short nType = GetDataType();
	return ((nType == NETVALUE_STRING) && (m_pBufValue != NULL)) || nType == NETVALUE_CHAR;
}

int CNetBasicValue::CompareBasicValue(const CNetBasicValue* pRhs) const
{
	const CNetBasicValue& rhs = *pRhs;
	if (IsNull())
		return rhs.IsNull() ? 0 : -1;

	if (rhs.IsNull())
		return 1;

	int ret = 0;
	switch (rhs.m_type)
	{
	case NETVALUE_LONG:
		ret = CompareInt(rhs.m_lValue[0]);
		break;
	case NETVALUE_LONGLONG:
		ret = CompareLongLong(rhs.m_llValue);
		break;
	case NETVALUE_DOUBLE:
		ret = CompareDouble(rhs.m_dValue);
		break;
	case NETVALUE_CHAR:
		ret = ComparePointString(rhs.m_cValue, sizeof(m_cValue) / sizeof(m_cValue[0]));
		break;
	case NETVALUE_STRING:
		ret = ComparePointString(rhs.m_pBufValue->c_str(), rhs.m_pBufValue->GetLength());
		break;
	}
	return ret;

}
int CNetBasicValue::CompareInt(Net_Int rhs) const
{
	if (IsNull())
		return (rhs == DTE_LONG_NULL) ? 0 : -1;

	if (rhs == DTE_LONG_NULL)
		return 1;

	int ret = 0;
	switch (m_type)
	{
	case NETVALUE_LONG:
		ret = (m_lValue[0] < rhs) ? -1 : ((m_lValue[0] > rhs) ? 1 : 0);
		break;
	case NETVALUE_LONGLONG:
		ret = (m_llValue < (Net_LONGLONG)rhs) ? -1 : ((m_llValue >(Net_LONGLONG)rhs) ? 1 : 0);
		break;
	case NETVALUE_DOUBLE:
	{
		double delta = m_dValue - rhs;
		ret = (fabs(delta) < TINY_VALUE) ? 0 : ((delta > 0.0) ? 1 : -1);
	}
	break;
	case NETVALUE_CHAR:
	case NETVALUE_STRING:
	{
		long value = GetLong();
		return (value < rhs) ? -1 : ((value > rhs) ? 1 : 0);
	}
	break;
	}
	return ret;
}

int CNetBasicValue::CompareLongLong(Net_LONGLONG rhs) const
{
	if (IsNull())
		return (rhs == DTE_LONG_NULL) ? 0 : -1;

	if (rhs == DTE_LONG_NULL)
		return 1;

	int ret = 0;
	switch (m_type)
	{
	case NETVALUE_LONG:
		ret = (m_lValue[0] < rhs) ? -1 : ((m_lValue[0] > rhs) ? 1 : 0);
		break;
	case NETVALUE_LONGLONG:
		ret = (m_llValue < rhs) ? -1 : ((m_llValue > rhs) ? 1 : 0);
		break;
	case NETVALUE_DOUBLE:
	{
		double delta = m_dValue - rhs;
		ret = (fabs(delta) < TINY_VALUE) ? 0 : ((delta > 0.0) ? 1 : -1);
	}
	break;
	case NETVALUE_CHAR:
	{
		ret = (atol(m_cValue) < rhs) ? -1 : ((m_lValue[0] > rhs) ? 1 : 0);
	}
	break;
	case NETVALUE_STRING:
	{
		Net_LONGLONG value = GetLongLong();
		ret = (value < rhs) ? -1 : ((value > rhs) ? 1 : 0);
	}
	break;
	}
	return ret;
}

int CNetBasicValue::CompareDouble(const Net_Double& rhs) const
{
	if (IsNull())
		return IsDoubleNullExt(rhs) ? 0 : -1;

	if (IsDoubleNullExt(rhs))
		return 1;

	int ret = 0;
	double delta = 0.0;
	switch (m_type)
	{
	case NETVALUE_LONG:
		delta = (double)m_lValue[0] - rhs;
		break;
	case NETVALUE_LONGLONG:
		delta = (double)m_llValue - rhs;
		break;
	case NETVALUE_DOUBLE:
		delta = m_dValue - rhs;
		break;
	case NETVALUE_CHAR:
	case NETVALUE_STRING:
		delta = GetDouble() - rhs;
		break;
	}
	return (fabs(delta) < TINY_VALUE) ? 0 : ((delta > 0.0) ? 1 : -1);
}

int CNetBasicValue::ComparePointString(const char* rhs, size_t len) const
{
	if (IsNull())
		return (IsStringEmpty(rhs)) ? 0 : -1;

	if (IsStringEmpty(rhs))
		return 1;

	if (len == NullLen)
		len = __tcslen(rhs);

	int ret = 0;
	switch (m_type)
	{
	case NETVALUE_CHAR:
		ret = __tcsncmp(m_cValue, rhs, min(len, sizeof(m_cValue) / sizeof(m_cValue[0])));
		break;
	case NETVALUE_STRING:
		ret = __tcsncmp(m_pBufValue->c_str(), rhs, len);
		break;
	}
	return ret;
}

//获取序列化长度
int CNetBasicValue::GetSeriazeLength() const
{
	int nRetSize = 1;
	switch (m_type)
	{
	case NETVALUE_CHAR:
	{
		nRetSize = 9;
	}
	break;
	case NETVALUE_STRING:
	{
		Net_UShort l = (Net_UShort)m_pBufValue->GetLength();
		nRetSize = l + 3;
	}
	break;
	case NETVALUE_LONG:
	{
		nRetSize = 5;
	}
	break;
	case NETVALUE_DOUBLE:
	{
		nRetSize = 9;
	}
	break;
	case NETVALUE_LONGLONG:
	{
		nRetSize = 9;
	}
	break;
	}
	return nRetSize;
}
int CNetBasicValue::Seriaze(char* pData, int nLength)
{
	if (nLength < sizeof(CNetBasicValue))
		return -1;
	int nRetSize = 0;
	pData[0] = m_type;
	nRetSize = 1;
	switch (m_type)
	{
	case NETVALUE_CHAR:
	{
		memcpy(pData + 1, m_cValue, 8);
		nRetSize = 9;
	}
	break;
	case NETVALUE_STRING:
	{
		Net_UShort l = (Net_UShort)m_pBufValue->GetLength();
		if (nLength < l + 3)
			return -1;
		nRetSize = SerializeCBasicString((unsigned char*)(pData + 1), m_pBufValue->c_str(), l) + 1;
	}
	break;
	case NETVALUE_LONG:
	{
		SerializeUInt((unsigned char*)pData, m_lValue[0]);
		nRetSize = 5;
	}
	break;
	case NETVALUE_DOUBLE:
	{
		SerializeLONGLONG((unsigned char*)(pData + 1), (*(Net_LONGLONG*)&m_dValue));
		nRetSize = 9;
	}
	break;
	case NETVALUE_LONGLONG:
	{
		SerializeLONGLONG((unsigned char*)(pData + 1), m_llValue);
		nRetSize = 9;
	}
	break;
	}
	return nRetSize;
}
int CNetBasicValue::UnSeriaze(const char* pData, int nLength)
{
	if (nLength < 1)
		return -1;
	//读取类型
	m_type = pData[0];
	switch (m_type)
	{
	case NETVALUE_CHAR:
	{
		if (nLength < sizeof(CNetBasicValue))
			return -1;
		memcpy(m_cValue, pData + 1, 8);
		return 9;
	}
	break;
	case NETVALUE_STRING:
	{
		if (nLength < 3)
			return -1;
		m_pBufValue = new Net_CBasicString();
		Net_UShort l = 0;
		UnSerializeUShort((unsigned char*)(pData + 1), l);
		if (l > 0)
		{
			if (nLength < 3 + l)
				return -1;
			char* lpszData = m_pBufValue->GetBufferSetLength(l);
			memcpy(lpszData, pData + 3, l);
		}
		return l + 3;
	}
	break;
	case NETVALUE_LONG:
	{
		if (nLength < 5)
			return -1;
		UnSerializeInt((unsigned char*)(pData + 1), m_lValue[0]);
		return 5;
	}
	break;
	case NETVALUE_DOUBLE:
	{
		if (nLength < sizeof(CNetBasicValue))
			return -1;
		Net_LONGLONG vValue = 0;
		UnSerializeLONGLONG((unsigned char*)(pData + 1), vValue);
		m_dValue = *(Net_Double*)&vValue;
		return 9;
	}
	break;
	case NETVALUE_LONGLONG:
	{
		if (nLength < sizeof(CNetBasicValue))
			return -1;
		UnSerializeLONGLONG((unsigned char*)(pData + 1), m_llValue);
		return 9;
	}
	break;
	}
	return sizeof(*this);
}

void CNetBasicValue::SeriazeSMBuf(basiclib::CBasicSmartBuffer& smBuf) const
{
	//类型
	smBuf.AppendData((const char*)&m_type, 1);
	switch (m_type)
	{
	case NETVALUE_CHAR:
	{
		smBuf.AppendData(m_cValue, 8);
	}
		break;
	case NETVALUE_STRING:
	{
		Net_UShort l = (Net_UShort)m_pBufValue->GetLength();
		unsigned char szBuf[2] = { 0 };
		SerializeUShort(szBuf, l);
		smBuf.AppendData((char*)szBuf, 2);
		if (l > 0)
		{
			smBuf.AppendData(m_pBufValue->c_str(), l);
		}
	}
		break;
	case NETVALUE_LONG:
	{
		char szBuf[4] = { 0 };
		SerializeUInt((unsigned char*)szBuf, m_lValue[0]);
		smBuf.AppendData(szBuf, 4);
	}
		break;
	case NETVALUE_DOUBLE:
	{
		char szBuf[8] = { 0 };
		SerializeLONGLONG((unsigned char*)szBuf, (*(Net_LONGLONG*)&m_dValue));
		smBuf.AppendData(szBuf, 8);
	}
		break;
	case NETVALUE_LONGLONG:
	{
		char szBuf[8] = { 0 };
		SerializeLONGLONG((unsigned char*)szBuf, m_llValue);
		smBuf.AppendData(szBuf, 8);
	}
		break;
	}
}
void CNetBasicValue::UnSeriazeSMBuf(basiclib::CBasicSmartBuffer& smBuf)
{
	//读取类型
	smBuf.ReadData(&m_type, 1);
	switch (m_type)
	{
	case NETVALUE_CHAR:
	{
		smBuf.ReadData(m_cValue, 8);
	}
	break;
	case NETVALUE_STRING:
	{
		m_pBufValue = new Net_CBasicString();
		Net_UShort l = 0;
		unsigned char szBuf[2] = { 0 };
		smBuf.ReadData(szBuf, 2);
		UnSerializeUShort(szBuf, l);
		if (l > 0)
		{
			char* lpszData = m_pBufValue->GetBufferSetLength(l);
			smBuf.ReadData(lpszData, l);
		}
	}
	break;
	case NETVALUE_LONG:
	{
		unsigned char szBuf[4] = { 0 };
		smBuf.ReadData(szBuf, 4);
		UnSerializeInt(szBuf, m_lValue[0]);
	}
	break;
	case NETVALUE_DOUBLE:
	{
		unsigned char szBuf[8] = { 0 };
		smBuf.ReadData(szBuf, 8);
		Net_LONGLONG vValue = 0;
		UnSerializeLONGLONG(szBuf, vValue);
		m_dValue = *(Net_Double*)&vValue;
	}
	break;
	case NETVALUE_LONGLONG:
	{
		unsigned char szBuf[8] = { 0 };
		smBuf.ReadData(szBuf, 8);
		UnSerializeLONGLONG(szBuf, m_llValue);
	}
	break;
	}
}

const CNetBasicValue& CNetBasicValue::operator = (const CNetBasicValue& rhs)
{
	Release();
	switch (rhs.m_type)
	{
	case NETVALUE_NULL:
		SetNull();
		break;
	case NETVALUE_STRING:
		SetString(rhs.m_pBufValue->c_str(), rhs.m_pBufValue->GetLength());
		break;
	default:
		memcpy(this, &rhs, sizeof(CNetBasicValue));
		break;
	}
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator = (const Net_Int rhs)
{
	SetLong(rhs);
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator = (const Net_Double& rhs)
{
	SetDouble(rhs);
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator = (const Net_LONGLONG& rhs)
{
	SetLongLong(rhs);
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator = (const char* rhs)
{
	SetString(rhs);
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator += (const CNetBasicValue& rhs)
{
	if (!IsNull())
	{
		switch (rhs.m_type)
		{
		case NETVALUE_NULL:
			SetNull();
			break;
		case NETVALUE_LONG:
			*this += rhs.m_lValue[0];
			break;
		case NETVALUE_DOUBLE:
			*this += rhs.GetDouble();
			break;
		case NETVALUE_LONGLONG:
			*this += rhs.GetLongLong();
			break;
		case NETVALUE_STRING:
		case NETVALUE_CHAR:
			*this += rhs.GetString().c_str();
			break;
		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator += (const Net_Int rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			m_lValue[0] += rhs;
			break;
		case NETVALUE_DOUBLE:
			m_dValue += rhs;
			break;
		case NETVALUE_CHAR:
		case NETVALUE_STRING:
		{
			unsigned short type = NETVALUE_LONG;
			AddString(DataToString(type, (void*)&rhs).c_str());
		}
		break;
		case NETVALUE_LONGLONG:
			m_llValue += rhs;
			break;
		}
	}
	return *this;
}

void CNetBasicValue::AddString(const char* lpStr)
{
	if (NULL == lpStr || lpStr[0] == '\0')
		return;

	switch (m_type)
	{
	case NETVALUE_CHAR:
	{
		CBasicString str(m_cValue);
		str += lpStr;
		SetString(str.c_str(), str.GetLength());
	}
	break;
	case NETVALUE_STRING:
		*m_pBufValue += lpStr;
		break;
	}
}

CBasicString CNetBasicValue::DataToString(unsigned short nDataType, const void* value)
{
	CBasicString strOut;
	switch (nDataType)
	{
	case NETVALUE_LONG:
	{
		strOut.Format("%ld", *(long*)value);
		break;
	}
	case NETVALUE_LONGLONG:
	{
		strOut.Format("%lld", *(long long*)value);
		break;
	}

	case NETVALUE_DOUBLE:
	{
		strOut.Format("%f", *(double*)value);
		break;
	}
	}
	return strOut;
}

const CNetBasicValue& CNetBasicValue::operator += (const Net_Double& rhs)
{
	switch (m_type)
	{
	case NETVALUE_NULL:
		break;
	case NETVALUE_LONG:
		*this = ((double)m_lValue[0] + rhs);
		break;
	case NETVALUE_DOUBLE:
		m_dValue += rhs;
		break;
	case NETVALUE_CHAR:
	case NETVALUE_STRING:
	{
		unsigned short type = NETVALUE_DOUBLE;
		AddString(DataToString(type, (void*)&rhs).c_str());
	}
	break;
	case NETVALUE_LONGLONG:
		*this = m_llValue + rhs;
		break;
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator += (const Net_LONGLONG& rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			*this = ((Net_LONGLONG)m_lValue[0] + rhs);
			break;
		case NETVALUE_DOUBLE:
			m_dValue += rhs;
			break;
		case NETVALUE_CHAR:
		case NETVALUE_STRING:
		{
			unsigned short type = NETVALUE_LONGLONG;
			AddString(DataToString(type, (void*)&rhs).c_str());
		}
		break;
		case NETVALUE_LONGLONG:
			m_llValue += rhs;
			break;
		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator += (const char* rhs)
{
	if (IsNull())
		SetString(rhs);
	else
		AddString(rhs);
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator -= (const CNetBasicValue& rhs)
{
	if (rhs.IsNull())
		SetNull();
	else if (!IsNull())
	{
		switch (rhs.m_type)
		{
		case NETVALUE_LONG:
			*this -= rhs.m_lValue[0];
			break;
		case NETVALUE_DOUBLE:
			*this -= rhs.m_dValue;
			break;
		case NETVALUE_LONGLONG:
			*this -= rhs.m_llValue;
			break;

		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator -= (const Net_Int rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			m_lValue[0] -= rhs;
			break;
		case NETVALUE_DOUBLE:
			m_dValue -= rhs;
			break;
		case NETVALUE_LONGLONG:
			m_llValue -= rhs;
			break;
		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator -= (const Net_Double& rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			SetDouble((double)m_lValue[0] - rhs);
			break;
		case NETVALUE_DOUBLE:
			m_dValue -= rhs;
			break;
		case NETVALUE_LONGLONG:
			SetDouble((double)m_llValue - rhs);
			break;
		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator -= (const Net_LONGLONG& rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			SetLongLong((LONGLONG)m_lValue[0] - rhs);
			break;
		case NETVALUE_DOUBLE:
			m_dValue -= rhs;
			break;
		case NETVALUE_LONGLONG:
			m_llValue -= rhs;
			break;
		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator *= (const CNetBasicValue& rhs)
{
	switch (rhs.m_type)
	{
	case NETVALUE_NULL:
		SetNull();
		break;
	case NETVALUE_LONG:
		*this *= rhs.GetLong();
		break;
	case NETVALUE_DOUBLE:
		*this *= rhs.GetDouble();
		break;
	case NETVALUE_LONGLONG:
		*this *= rhs.GetLongLong();
		break;
	}
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator *= (const Net_Int rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			m_lValue[0] *= rhs;
			break;
		case NETVALUE_DOUBLE:
			m_dValue *= rhs;
			break;
		case NETVALUE_LONGLONG:
			m_llValue *= rhs;
			break;
		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator *= (const Net_Double& rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			SetDouble((double)m_lValue[0] * rhs);
			break;
		case NETVALUE_DOUBLE:
			m_dValue *= rhs;
			break;
		case NETVALUE_LONGLONG:
			SetDouble((double)m_llValue * rhs);
			break;
		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator *= (const Net_LONGLONG& rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			SetLongLong(m_lValue[0] * rhs);
			break;
		case NETVALUE_DOUBLE:
			m_dValue *= (double)rhs;
			break;
		case NETVALUE_LONGLONG:
			m_llValue *= rhs;
			break;
		}
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator /= (const CNetBasicValue& rhs)
{
	switch (rhs.m_type)
	{
	case NETVALUE_NULL:
		SetNull();
		break;
	case NETVALUE_LONG:
		*this /= rhs.GetLong();
		break;
	case NETVALUE_DOUBLE:
		*this /= rhs.GetDouble();
		break;
	case NETVALUE_LONGLONG:
		*this /= rhs.GetLongLong();
		break;
	}
	return *this;
}

const CNetBasicValue& CNetBasicValue::operator /= (const Net_Int rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			m_lValue[0] /= rhs;
			break;
		case NETVALUE_DOUBLE:
			m_dValue /= rhs;
			break;
		case NETVALUE_LONGLONG:
			m_llValue /= rhs;
			break;
		}
	}
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator /= (const Net_Double& rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			SetDouble((double)m_lValue[0] / rhs);
			break;
		case NETVALUE_DOUBLE:
			m_dValue /= rhs;
			break;
		case NETVALUE_LONGLONG:
			SetDouble((double)m_llValue / rhs);
			break;
		}
	}
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator /= (const Net_LONGLONG& rhs)
{
	if (IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			SetLongLong((LONGLONG)m_lValue[0] / rhs);
			break;
		case NETVALUE_DOUBLE:
			m_dValue /= (double)rhs;
			break;
		case NETVALUE_LONGLONG:
			m_llValue /= rhs;
			break;
		}
	}
	return *this;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicBitstream::CBasicBitstream()
{
}

CBasicBitstream::~CBasicBitstream()
{
}

CBasicBitstream::CBasicBitstream(const string& s)
{
	AppendData(s.c_str(), s.size());
}

CBasicBitstream::CBasicBitstream(void* buf, int size)
{
	AppendData((char*)buf, size);
}
CBasicBitstream::CBasicBitstream(const char* buf, int size)
{
	AppendData(buf, size);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicBitstream& CBasicBitstream::operator >> (Net_Char* v)
{
	Net_UShort l = 0;
	*this >> l;
	if (l > 0)
		ReadData(v, l);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_UChar* v)
{
	Net_UShort l = 0;
	*this >> l;
	if (l > 0)
		ReadData((char*)v, l);
	return *this;
}

CBasicBitstream& CBasicBitstream::operator >> (CBasicSmartBuffer& os)
{
	Net_UShort l = 0;
	*this >> l;
	if (l > 0)
	{
		os.SetDataLength(l);
		ReadData(os.GetDataBuffer(), l);
	}
	return *this;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicBitstream& CBasicBitstream::operator << (const Net_UChar v)
{
	AppendData((const char*)&v, sizeof(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_Char v)
{
	AppendData((const char*)&v, sizeof(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_UShort v)
{
	AppendData((char*)m_szBuf, SerializeUShort(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_Short v)
{
	AppendData((char*)m_szBuf, SerializeUShort(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_UInt v)
{
	AppendData((char*)m_szBuf, SerializeUInt(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_Int v)
{
	AppendData((char*)m_szBuf, SerializeUInt(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_LONGLONG v)
{
	AppendData((char*)m_szBuf, SerializeLONGLONG(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_Double v)
{
	AppendData((char*)m_szBuf, SerializeLONGLONG(m_szBuf, (*(Net_LONGLONG*)&v)));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const CNetBasicValue* pV)
{
	pV->SeriazeSMBuf(*this);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const CNetBasicValue& v)
{
	v.SeriazeSMBuf(*this);
	return *this;
}

CBasicBitstream& CBasicBitstream::operator << (const Net_CBasicString* pV)
{
	SerializeDataBuffer(pV->c_str(), pV->GetLength());
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_CBasicString& data)
{
	SerializeDataBuffer(data.c_str(), data.GetLength());
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const basiclib::CBasicSmartBuffer* pV)
{
	SerializeDataBuffer(pV->GetDataBuffer(), pV->GetDataLength());
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_Char* v)
{
	SerializeDataBuffer(v, strlen(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const Net_UChar* v)
{
	SerializeDataBuffer((const Net_Char*)v, strlen((const Net_Char*)v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const CBasicSmartBuffer& insRet)
{
	SerializeDataBuffer(insRet.GetDataBuffer(), insRet.GetDataLength());
	return *this;
}
void CBasicBitstream::SerializeDataBuffer(const Net_Char* pData, Net_UShort usLength)
{
	Net_UInt nResLength = m_cbBuffer;
	SetDataLength(nResLength + sizeof(Net_UShort) + usLength);
	SerializeCBasicString((unsigned char*)(m_pszBuffer + nResLength), pData, usLength);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicBitstream& CBasicBitstream::operator >> (Net_UChar& v)
{
	ReadData(&v, sizeof(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_Char& v)
{
	ReadData(&v, sizeof(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_UShort& v)
{
	ReadData(m_szBuf, 2);
	UnSerializeUShort(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_Short& v)
{
	ReadData(m_szBuf, 2);
	UnSerializeShort(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_UInt& v)
{
	ReadData(m_szBuf, 4);
	UnSerializeUInt(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_Int& v)
{
	ReadData(m_szBuf, 4);
	UnSerializeInt(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_LONGLONG& v)
{
	ReadData(m_szBuf, 8);
	UnSerializeLONGLONG(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_Double& v)
{
	Net_LONGLONG vValue;
	*this >> vValue;
	v = *(Net_Double*)&vValue;
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_CBasicString* pV)
{
	UnSerializeCString(pV);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (Net_CBasicString& data)
{
	UnSerializeCString(&data);
	return *this;
}

void CBasicBitstream::UnSerializeCString(Net_CBasicString* pV)
{
	Net_UShort l = 0;
	*this >> l;
	if (l > 0)
	{
		char* lpszData = pV->GetBufferSetLength(l);
		ReadData(lpszData, l);
	}
}
CBasicBitstream& CBasicBitstream::operator >> (basiclib::CBasicSmartBuffer* pV)
{
	UnSerializeSmbuf(pV);
	return *this;
}

void CBasicBitstream::UnSerializeSmbuf(basiclib::CBasicSmartBuffer* pV)
{
	Net_UShort l = 0;
	*this >> l;
	if (l > 0)
	{
		pV->SetDataLength(l);
		ReadData(pV->GetDataBuffer(), l);
	}
}

CBasicBitstream& CBasicBitstream::operator >> (CNetBasicValue* pV)
{
	pV->UnSeriazeSMBuf(*this);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (CNetBasicValue& v)
{
	v.UnSeriazeSMBuf(*this);
	return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////////////

__NS_BASIC_END

//////////////////////////////////////////////////////////////////////////////////////////////////

