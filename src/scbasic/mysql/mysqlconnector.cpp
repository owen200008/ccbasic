#include "mysqlconnector.h"

CParseMySQLPacketBuffer::CParseMySQLPacketBuffer()
{
	m_nPacketLength = 0;
}
CParseMySQLPacketBuffer::~CParseMySQLPacketBuffer()
{

}

//定位数据头
long CParseMySQLPacketBuffer::IsPacketFull(long lMaxPacketSize)
{
	if (m_cbBuffer < 4)
		return MYSQLCONNECT_LOCATION_HEADER_NOENOUGH;
	if (m_nPacketLength == 0)
		basiclib::UnSerializeUInt3Bit((unsigned char*)m_pszBuffer, m_nPacketLength);
	if (m_nPacketLength > lMaxPacketSize || m_nPacketLength == 0){
		return MYSQLCONNECT_LOCATION_HEADER_NOTALLOWED;
	}
	if (m_cbBuffer >= 4 + m_nPacketLength){
		return MYSQLCONNECT_LOCATION_HEADER_FIND;
	}
	return MYSQLCONNECT_LOCATION_HEADER_NOENOUGH;
}

/*下一个头*/
bool CParseMySQLPacketBuffer::ResetHeader()
{
	if (0 == m_nPacketLength)
	{
		return false;
	}
	int nHeadSize = 4;

	char* pTail = m_pszBuffer + m_cbBuffer;
	char* pNext = m_pszBuffer + m_nPacketLength + nHeadSize;
	if (pNext == pTail)
	{
		SetDataLength(0);
        m_nPacketLength = 0;
		return true;
	}
	if (pNext < pTail)
	{
		m_cbBuffer = pTail - pNext;
		memmove(m_pszBuffer, pNext, m_cbBuffer);
		//
		char* pLeft = m_pszBuffer + m_cbBuffer;
		int nLeft = pTail - pLeft;
		memset(pLeft, 0, nLeft);
		//reset
		m_nPacketLength = 0;
		return true;
	}
	ASSERT(0);
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
MysqlRowColData* MysqlResultData::GetCurrentRowDataByIndex(int nIndex){
    if (nullptr == m_pCurrentRow)
        return nullptr;
    return &(m_pCurrentRow->m_vtRowCol[nIndex]);
}
bool MysqlResultData::NextRow(){
    if (nullptr == m_pCurrentRow){
        m_pCurrentRow = m_pRowData;
        return m_pCurrentRow != nullptr;
    }
    m_pCurrentRow = m_pCurrentRow->m_pNextRowData;
    return m_pCurrentRow != nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MysqlResultData::GetColCStringByIndex(int nIndex, basiclib::CBasicString& strRet){
    MysqlRowColData* pRowColData = GetCurrentRowDataByIndex(nIndex);
    if (nullptr == pRowColData)
        return false;
    strRet.assign(m_smRowData.GetDataBuffer() + pRowColData->m_nPos, pRowColData->m_nLength);
    return true;
}
bool MysqlResultData::GetColCString(const char* pColName, basiclib::CBasicString& strRet){
    MapMysqlCol::iterator iter = m_mapCol.find(pColName);
    if (iter == m_mapCol.end())
        return false;
    MysqlRowColData* pRowColData = GetCurrentRowDataByIndex(iter->second);
    if (nullptr == pRowColData)
        return false;
    strRet.assign(m_smRowData.GetDataBuffer() + pRowColData->m_nPos, pRowColData->m_nLength);
    return true;
}
bool MysqlResultData::GetColSmartBuffer(const char* pColName, basiclib::CBasicSmartBuffer& strRet) {
	MapMysqlCol::iterator iter = m_mapCol.find(pColName);
	if (iter == m_mapCol.end())
		return false;
	MysqlRowColData* pRowColData = GetCurrentRowDataByIndex(iter->second);
	if (nullptr == pRowColData)
		return false;
	strRet.AppendData(m_smRowData.GetDataBuffer() + pRowColData->m_nPos, pRowColData->m_nLength);
	return true;
}

bool MysqlResultData::GetColUIntData(const char* pColName, Net_UInt& nValue){
    MapMysqlCol::iterator iter = m_mapCol.find(pColName);
    if (iter == m_mapCol.end())
        return false;
    MysqlRowColData* pRowColData = GetCurrentRowDataByIndex(iter->second);
    if (nullptr == pRowColData)
        return false;
    char szBuf[16] = { 0 };
    memcpy(szBuf, m_smRowData.GetDataBuffer() + pRowColData->m_nPos, pRowColData->m_nLength);
    nValue = atol(szBuf);
    return true;
}
Net_UInt MysqlResultData::GetColUInt(const char* pColName, Net_UInt nDefault) {
	if (GetColUIntData(pColName, nDefault)) {
		return nDefault;
	}
	return nDefault;
}

bool MysqlResultData::GetColIntData(const char* pColName, Net_Int& nValue){
    MapMysqlCol::iterator iter = m_mapCol.find(pColName);
    if (iter == m_mapCol.end())
        return false;
    MysqlRowColData* pRowColData = GetCurrentRowDataByIndex(iter->second);
    if (nullptr == pRowColData)
        return false;
    char szBuf[32] = { 0 };
    memcpy(szBuf, m_smRowData.GetDataBuffer() + pRowColData->m_nPos, pRowColData->m_nLength);
    nValue = atol(szBuf);
    return true;
}
Net_Int MysqlResultData::GetColInt(const char* pColName, Net_Int nDefault) {
	if (GetColIntData(pColName, nDefault)) {
		return nDefault;
	}
	return nDefault;
}

bool MysqlResultData::GetColDoubleData(const char* pColName, Net_Double& dValue){
    MapMysqlCol::iterator iter = m_mapCol.find(pColName);
    if (iter == m_mapCol.end())
        return false;
    MysqlRowColData* pRowColData = GetCurrentRowDataByIndex(iter->second);
    if (nullptr == pRowColData)
        return false;
    char szBuf[64] = { 0 };
    memcpy(szBuf, m_smRowData.GetDataBuffer() + pRowColData->m_nPos, pRowColData->m_nLength);
    dValue = atof(szBuf);
    return true;
}
Net_Double MysqlResultData::GetColDouble(const char* pColName, Net_Double dDefault) {
	if (GetColDoubleData(pColName, dDefault)) {
		return dDefault;
	}
	return dDefault;
}

bool MysqlResultData::GetColLongLongData(const char* pColName, Net_LONGLONG& llValue){
    MapMysqlCol::iterator iter = m_mapCol.find(pColName);
    if (iter == m_mapCol.end())
        return false;
    MysqlRowColData* pRowColData = GetCurrentRowDataByIndex(iter->second);
    if (nullptr == pRowColData)
        return false;
    char szBuf[64] = { 0 };
    memcpy(szBuf, m_smRowData.GetDataBuffer() + pRowColData->m_nPos, pRowColData->m_nLength);
    llValue = atoll(szBuf);
    return true;
}
Net_LONGLONG MysqlResultData::GetColLongLong(const char* pColName, Net_LONGLONG llDefault) {
	if (GetColLongLongData(pColName, llDefault)) {
		return llDefault;
	}
	return llDefault;
}

