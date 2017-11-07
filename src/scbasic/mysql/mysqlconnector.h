#ifndef _MYSQL_CONNECTOR_H_
#define _MYSQL_CONNECTOR_H_

#include <basic.h>
#include "../scbasic_head.h"
#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define	MYSQLCONNECT_LOCATION_HEADER_FIND		0
#define MYSQLCONNECT_LOCATION_HEADER_NOENOUGH	1
#define MYSQLCONNECT_LOCATION_HEADER_NOTALLOWED  2
class _SCBASIC_DLL_API CParseMySQLPacketBuffer : public basiclib::CBasicSmartBuffer
{
public:
	CParseMySQLPacketBuffer();
	virtual ~CParseMySQLPacketBuffer();

	//! 定位数据头
	long IsPacketFull(long lMaxPacketSize);

	//! 获取包长
	Net_UInt GetTotalPacketLength(){ return m_nPacketLength + 4; }

	//! 下一个
	bool ResetHeader();
protected:
	Net_UInt	m_nPacketLength;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFINEDISPATCH_CORUTINE_SQLEXEC     0
#define DEFINEDISPATCH_CORUTINE_SQLQUERY    1
#define DEFINEDISPATCH_CORUTINE_SQLPING     2
#define DEFINEDISPATCH_CORUTINE_SQLMULTI    3
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct _SCBASIC_DLL_API MysqlResultOK{
    int                 m_nAffectRow;
    int                 m_nLastInsertID;
    Net_UShort          m_usServerState;
    Net_UShort          m_usWarnCount;
    Net_CBasicString    m_strInfo;
    MysqlResultOK(){
        m_nAffectRow = 0;
        m_nLastInsertID = 0;
        m_usServerState = 0;
        m_usWarnCount = 0;
    }
};

struct _SCBASIC_DLL_API MysqlResultError{
    Net_UShort          m_usErrorID;
    char				m_szSQLState[5];
    Net_CBasicString    m_strError;
    MysqlResultError(){
        m_usErrorID = 0;
        memset(m_szSQLState, 0, 5);
    }
};

struct _SCBASIC_DLL_API MysqlColData{
    Net_UShort                      m_usCharSet;
    Net_UInt                        m_unColLength;
    Net_UChar                       m_nColType;
    MysqlColData(){
        m_usCharSet = 0;
        m_unColLength = 0;
        m_nColType = 0;
    }
};

struct _SCBASIC_DLL_API MysqlRowColData{
    int m_nPos;
    int m_nLength;
    MysqlRowColData(){
        m_nPos = 0;
        m_nLength = 0;
    }
};
typedef basiclib::basic_vector<MysqlRowColData>    VTRowColData;
struct _SCBASIC_DLL_API MysqlRowData : public basiclib::CBasicObject{
    VTRowColData    m_vtRowCol;
    MysqlRowData*   m_pNextRowData;
    MysqlRowData(){
        m_pNextRowData = nullptr;
    }
    virtual ~MysqlRowData(){
        if (m_pNextRowData){
            delete m_pNextRowData;
        }
    }
};
typedef basiclib::basic_map<basiclib::CBasicString, int> MapMysqlCol;
typedef basiclib::basic_vector<MysqlColData>    VTColData;
struct _SCBASIC_DLL_API MysqlResultData : public basiclib::CBasicObject{
    int                             m_nFieldCount;
    int                             m_nExtraCount;
    MapMysqlCol                     m_mapCol;
    VTColData                       m_colData;
    MysqlRowData*                   m_pRowData;
    MysqlRowData*                   m_pCurrentRow;
    MysqlRowData*                   m_pAddEnd;
    basiclib::CBasicSmartBuffer     m_smRowData;
    MysqlResultData(){
        m_nFieldCount = 0;
        m_nExtraCount = 0;
        m_pRowData = nullptr;
        m_pCurrentRow = nullptr;
        m_pAddEnd = nullptr;
    }
    virtual ~MysqlResultData(){
        if (m_pRowData){
            delete m_pRowData;
        }
    }
    //下一行
    bool NextRow();
    //获取数据
    MysqlRowColData* GetCurrentRowDataByIndex(int nIndex);
    //获取数据
    bool GetColCString(const char* pColName, basiclib::CBasicString& strRet);
	bool GetColSmartBuffer(const char* pColName, basiclib::CBasicSmartBuffer& strRet);
    bool GetColUIntData(const char* pColName, Net_UInt& nValue);
	Net_UInt GetColUInt(const char* pColName, Net_UInt nDefault = 0);
    bool GetColIntData(const char* pColName, Net_Int& nValue);
	Net_Int GetColInt(const char* pColName, Net_Int nDefault = 0);
    bool GetColDoubleData(const char* pColName, Net_Double& dValue);
	Net_Double GetColDouble(const char* pColName, Net_Double dDefault);
    bool GetColLongLongData(const char* pColName, Net_LONGLONG& llValue);
	Net_LONGLONG GetColLongLong(const char* pColName, Net_LONGLONG llDefault);
    bool GetColCStringByIndex(int nIndex, basiclib::CBasicString& strRet);
    
};
typedef basiclib::basic_vector<MysqlResultData> VTMysqlResultData;

struct _SCBASIC_DLL_API MysqlResultEOF{
    Net_UShort m_usWarnCount;
    Net_UShort m_usState;
    MysqlResultEOF(){
        m_usWarnCount = 0;
        m_usState = 0;
    }
};

struct _SCBASIC_DLL_API MysqlReplyExec : basiclib::CBasicObject{
    Net_UChar           m_cFieldCount;
    MysqlResultOK       m_ok;
    MysqlResultError    m_error;
    MysqlReplyExec*     m_pNext;
    MysqlReplyExec(){
        m_cFieldCount = 0;
        m_pNext = nullptr;
    }
    virtual ~MysqlReplyExec(){
        if (m_pNext){
            delete m_pNext;
        }
    }
};
struct _SCBASIC_DLL_API MysqlReplyQuery : basiclib::CBasicObject{
    Net_UChar           m_cFieldCount;
    MysqlResultData     m_data;
    MysqlResultError    m_error;
    MysqlReplyQuery*    m_pNext;
    MysqlReplyQuery(){
        m_cFieldCount = 0;
        m_pNext = nullptr;
    }
    virtual ~MysqlReplyQuery(){
        if (m_pNext){
            delete m_pNext;
        }
    }
};

//多语句执行请求包
class _SCBASIC_DLL_API MysqlMultiRequest : public basiclib::CBasicObject{
public:
    MysqlMultiRequest(int nType, const char* pSQL){ 
        m_nType = nType;
        m_pSQL = pSQL;
        m_pNext = nullptr; 
    }
    virtual ~MysqlMultiRequest(){}
    int                 m_nType;
    const char*         m_pSQL;
    MysqlMultiRequest*  m_pNext;
};
class _SCBASIC_DLL_API MysqlMultiRequestExec : public MysqlMultiRequest{
public:
    MysqlMultiRequestExec(const char* pSQL) : MysqlMultiRequest(DEFINEDISPATCH_CORUTINE_SQLEXEC, pSQL){
    }
    virtual ~MysqlMultiRequestExec(){}
    MysqlReplyExec m_reply;
};
class _SCBASIC_DLL_API MysqlMultiRequestQuery : public MysqlMultiRequest{
public:
    MysqlMultiRequestQuery(const char* pSQL) : MysqlMultiRequest(DEFINEDISPATCH_CORUTINE_SQLQUERY, pSQL){
    }
    virtual ~MysqlMultiRequestQuery(){}
    MysqlReplyQuery m_reply;
};

#pragma warning (pop)
#endif