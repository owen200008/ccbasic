#include "../inc/basic.h"

__NS_BASIC_START

int SerializeUChar(unsigned char* pBuffer, const uint8_t v)
{
	pBuffer[0] = v;
	return 1;
}

int SerializeUShort(unsigned char* pBuffer, const uint16_t v)
{
	pBuffer[0] = (v)& 0xFF;
	pBuffer[1] = (v >> 8) & 0xFF;
	return 2;
}

int SerializeUInt(unsigned char* pBuffer, const uint32_t v)
{
	pBuffer[0] = (v)& 0xFF;
	pBuffer[1] = (v >> 8) & 0xFF;
	pBuffer[2] = (v >> 16) & 0xFF;
	pBuffer[3] = (v >> 24) & 0xFF;
	return 4;
}
int SerializeUInt3Bit(unsigned char* pBuffer, const uint32_t v)
{
	pBuffer[0] = (v)& 0xFF;
	pBuffer[1] = (v >> 8) & 0xFF;
	pBuffer[2] = (v >> 16) & 0xFF;
	return 3;
}

int SerializeLONGLONG(unsigned char* pBuffer, const int64_t v)
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
int SerializeCBasicString(unsigned char* pBuffer, const int8_t* v, uint16_t usLength)
{
	int nRetSize = 0;
	nRetSize = SerializeUShort(pBuffer, usLength);
	if (usLength > 0)
	{
		memcpy(pBuffer + nRetSize, v, usLength);
	}
	return nRetSize + usLength;
}

int UnSerializeUChar(unsigned char* pBuffer, uint8_t& v)
{
	v = pBuffer[0];
	return 1;
}
int UnSerializeChar(unsigned char* pBuffer, int8_t& v)
{
	v = pBuffer[0];
	return 1;
}

int UnSerializeUShort(unsigned char* pBuffer, uint16_t& v)
{
	v = pBuffer[0] | pBuffer[1] << 8;
	return 2;
}
int UnSerializeShort(unsigned char* pBuffer, int16_t& v)
{
	v = pBuffer[0] | pBuffer[1] << 8;
	return 2;
}
int UnSerializeUInt3Bit(unsigned char* pBuffer, uint32_t& v)
{
	v = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16;
	return 3;
}
int UnSerializeUInt(unsigned char* pBuffer, uint32_t& v)
{
	v = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
	return 4;
}
int UnSerializeInt(unsigned char* pBuffer, int32_t& v)
{
	v = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
	return 4;
}
int UnSerializeLONGLONG(unsigned char* pBuffer, int64_t& v)
{
	int64_t vHigh = pBuffer[4] << 0 | pBuffer[5] << 8 | pBuffer[6] << 16 | pBuffer[7] << 24;
	int64_t vLow = pBuffer[0] | pBuffer[1] << 8 | pBuffer[2] << 16 | pBuffer[3] << 24;
	v = vLow | vHigh << 32;
	return 8;
}
int UnSerializeCBasicString(unsigned char* pBuffer, basiclib::CBasicString& str)
{
	int nRet = 0;
	uint16_t usLength = 0;
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

CNetBasicValue::CNetBasicValue(const double& value)
{
	EmptyValue();
	SetDouble(value);
}

CNetBasicValue::CNetBasicValue(const int64_t& value)
{
	EmptyValue();
	SetLongLong(value);
}

CNetBasicValue::CNetBasicValue(const int32_t value)
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

void CNetBasicValue::SetLong(const int32_t value)
{
	Release();
	m_type = NETVALUE_LONG;
	m_lValue[0] = value;
}

void CNetBasicValue::SetDouble(const double& value)
{
	Release();
	m_type = NETVALUE_DOUBLE;
	m_dValue = value;
}

void CNetBasicValue::SetLongLong(const int64_t& value)
{
	Release();
	m_type = NETVALUE_LONGLONG;
	memcpy(&m_llValue, &value, sizeof(int64_t));
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
		m_pBufValue = new basiclib::CBasicString(value, len);
	}
}

int32_t CNetBasicValue::GetLong() const
{
	int32_t ret = (int32_t)DTE_LONG_NULL;
	toData(ret);
	return ret;
}
double CNetBasicValue::GetDouble() const
{
	double ret = DTEDOUBLE_NULL;
	toData(ret);
	return ret;
}

int64_t CNetBasicValue::GetLongLong() const
{
	int64_t ret = 0;
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


void CNetBasicValue::toData(int32_t& lValue) const
{
	lValue = (int32_t)DTE_LONG_NULL;
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			lValue = m_lValue[0];
			break;
		case NETVALUE_DOUBLE:
			lValue = (int32_t)m_dValue;
			break;
		case NETVALUE_LONGLONG:
			lValue = (int32_t)m_llValue;
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


void CNetBasicValue::toData(double& fValue) const
{
	fValue = DTEDOUBLE_NULL;
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

void CNetBasicValue::toData(int64_t& llValue) const
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			llValue = m_lValue[0];
			break;
		case NETVALUE_LONGLONG:
			memcpy(&llValue, &m_llValue, sizeof(int64_t));
			break;
		case NETVALUE_DOUBLE:
			llValue = (int64_t)m_dValue;
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
int CNetBasicValue::CompareInt(int32_t rhs) const
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
		ret = (m_llValue < (int64_t)rhs) ? -1 : ((m_llValue >(int64_t)rhs) ? 1 : 0);
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

int CNetBasicValue::CompareLongLong(int64_t rhs) const
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
		int64_t value = GetLongLong();
		ret = (value < rhs) ? -1 : ((value > rhs) ? 1 : 0);
	}
	break;
	}
	return ret;
}

int CNetBasicValue::CompareDouble(const double& rhs) const
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

//��ȡ���л�����
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
		uint16_t l = (uint16_t)m_pBufValue->GetLength();
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
		uint16_t l = (uint16_t)m_pBufValue->GetLength();
		if (nLength < l + 3)
			return -1;
		nRetSize = SerializeCBasicString((unsigned char*)(pData + 1), (int8_t*)m_pBufValue->c_str(), l) + 1;
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
		SerializeLONGLONG((unsigned char*)(pData + 1), (*(int64_t*)&m_dValue));
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
	//��ȡ����
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
		m_pBufValue = new basiclib::CBasicString();
		uint16_t l = 0;
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
		int64_t vValue = 0;
		UnSerializeLONGLONG((unsigned char*)(pData + 1), vValue);
		m_dValue = *(double*)&vValue;
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
	//����
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
		uint16_t l = (uint16_t)m_pBufValue->GetLength();
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
		SerializeLONGLONG((unsigned char*)szBuf, (*(int64_t*)&m_dValue));
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
	//��ȡ����
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
		m_pBufValue = new basiclib::CBasicString();
		uint16_t l = 0;
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
		int64_t vValue = 0;
		UnSerializeLONGLONG(szBuf, vValue);
		m_dValue = *(double*)&vValue;
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
const CNetBasicValue& CNetBasicValue::operator = (const int32_t rhs)
{
	SetLong(rhs);
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator = (const double& rhs)
{
	SetDouble(rhs);
	return *this;
}
const CNetBasicValue& CNetBasicValue::operator = (const int64_t& rhs)
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

const CNetBasicValue& CNetBasicValue::operator += (const int32_t rhs)
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

const CNetBasicValue& CNetBasicValue::operator += (const double& rhs)
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

const CNetBasicValue& CNetBasicValue::operator += (const int64_t& rhs)
{
	if (!IsNull())
	{
		switch (m_type)
		{
		case NETVALUE_LONG:
			*this = ((int64_t)m_lValue[0] + rhs);
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

const CNetBasicValue& CNetBasicValue::operator -= (const int32_t rhs)
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

const CNetBasicValue& CNetBasicValue::operator -= (const double& rhs)
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

const CNetBasicValue& CNetBasicValue::operator -= (const int64_t& rhs)
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
const CNetBasicValue& CNetBasicValue::operator *= (const int32_t rhs)
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

const CNetBasicValue& CNetBasicValue::operator *= (const double& rhs)
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

const CNetBasicValue& CNetBasicValue::operator *= (const int64_t& rhs)
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

const CNetBasicValue& CNetBasicValue::operator /= (const int32_t rhs)
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
const CNetBasicValue& CNetBasicValue::operator /= (const double& rhs)
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
const CNetBasicValue& CNetBasicValue::operator /= (const int64_t& rhs)
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
CBasicBitstream& CBasicBitstream::operator >> (int8_t* v)
{
	uint16_t l = 0;
	*this >> l;
	if (l > 0)
		ReadData(v, l);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (uint8_t* v)
{
	uint16_t l = 0;
	*this >> l;
	if (l > 0)
		ReadData((char*)v, l);
	return *this;
}

CBasicBitstream& CBasicBitstream::operator >> (CBasicSmartBuffer& os)
{
	uint16_t l = 0;
	*this >> l;
	if (l > 0)
	{
		os.SetDataLength(l);
		ReadData(os.GetDataBuffer(), l);
	}
	return *this;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicBitstream& CBasicBitstream::operator << (const uint8_t v)
{
	AppendData((const char*)&v, sizeof(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const int8_t v)
{
	AppendData((const char*)&v, sizeof(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const uint16_t v)
{
	AppendData((char*)m_szBuf, SerializeUShort(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const int16_t v)
{
	AppendData((char*)m_szBuf, SerializeUShort(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const uint32_t v)
{
	AppendData((char*)m_szBuf, SerializeUInt(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const int32_t v)
{
	AppendData((char*)m_szBuf, SerializeUInt(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const int64_t v)
{
	AppendData((char*)m_szBuf, SerializeLONGLONG(m_szBuf, v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const double v)
{
	AppendData((char*)m_szBuf, SerializeLONGLONG(m_szBuf, (*(int64_t*)&v)));
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

CBasicBitstream& CBasicBitstream::operator << (const basiclib::CBasicString* pV)
{
	SerializeDataBuffer((int8_t*)pV->c_str(), pV->GetLength());
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const basiclib::CBasicString& data)
{
	SerializeDataBuffer((int8_t*)data.c_str(), data.GetLength());
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const basiclib::CBasicSmartBuffer* pV)
{
	SerializeDataBuffer((int8_t*)pV->GetDataBuffer(), (uint16_t)pV->GetDataLength());
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const int8_t* v)
{
	SerializeDataBuffer(v, (uint16_t)strlen((char*)v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const uint8_t* v)
{
	SerializeDataBuffer((const int8_t*)v, (uint16_t)strlen((const char*)v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator << (const CBasicSmartBuffer& insRet)
{
	SerializeDataBuffer((int8_t*)insRet.GetDataBuffer(), (uint16_t)insRet.GetDataLength());
	return *this;
}
void CBasicBitstream::SerializeDataBuffer(const int8_t* pData, uint16_t usLength)
{
	uint32_t nResLength = m_cbBuffer;
	SetDataLength(nResLength + sizeof(uint16_t) + usLength);
	SerializeCBasicString((unsigned char*)(m_pszBuffer + nResLength), pData, usLength);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBasicBitstream& CBasicBitstream::operator >> (uint8_t& v)
{
	ReadData(&v, sizeof(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (int8_t& v)
{
	ReadData(&v, sizeof(v));
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (uint16_t& v)
{
	ReadData(m_szBuf, 2);
	UnSerializeUShort(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (int16_t& v)
{
	ReadData(m_szBuf, 2);
	UnSerializeShort(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (uint32_t& v)
{
	ReadData(m_szBuf, 4);
	UnSerializeUInt(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (int32_t& v)
{
	ReadData(m_szBuf, 4);
	UnSerializeInt(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (int64_t& v)
{
	ReadData(m_szBuf, 8);
	UnSerializeLONGLONG(m_szBuf, v);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (double& v)
{
	int64_t vValue;
	*this >> vValue;
	v = *(double*)&vValue;
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (basiclib::CBasicString* pV)
{
	UnSerializeCString(pV);
	return *this;
}
CBasicBitstream& CBasicBitstream::operator >> (basiclib::CBasicString& data)
{
	UnSerializeCString(&data);
	return *this;
}

void CBasicBitstream::UnSerializeCString(basiclib::CBasicString* pV)
{
	uint16_t l = 0;
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
	uint16_t l = 0;
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

