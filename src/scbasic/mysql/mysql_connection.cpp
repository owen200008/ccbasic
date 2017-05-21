#include "mysql_connection.h"


#define bit_uint1korr(A)	(*(((uint8_t*)(A))))

#define bit_uint2korr(A) ((uint16_t) (((uint16_t) (((unsigned char*) (A))[1])) +\
                                   ((uint16_t) (((unsigned char*) (A))[0]) << 8)))
#define bit_uint3korr(A) ((uint32_t) (((uint32_t) (((unsigned char*) (A))[2])) +\
                                   (((uint32_t) (((unsigned char*) (A))[1])) << 8) +\
                                   (((uint32_t) (((unsigned char*) (A))[0])) << 16)))
#define bit_uint4korr(A) ((uint32_t) (((uint32_t) (((unsigned char*) (A))[3])) +\
                                   (((uint32_t) (((unsigned char*) (A))[2])) << 8) +\
                                   (((uint32_t) (((unsigned char*) (A))[1])) << 16) +\
                                   (((uint32_t) (((unsigned char*) (A))[0])) << 24)))
#define bit_uint5korr(A) ((uint64_t)(((uint32_t) (((unsigned char*) (A))[4])) +\
                                    (((uint32_t) (((unsigned char*) (A))[3])) << 8) +\
                                    (((uint32_t) (((unsigned char*) (A))[2])) << 16) +\
                                   (((uint32_t) (((unsigned char*) (A))[1])) << 24)) +\
                                    (((uint64_t) (((unsigned char*) (A))[0])) << 32))
#define bit_uint6korr(A) ((uint64_t)(((uint32_t) (((unsigned char*) (A))[5])) +\
                                    (((uint32_t) (((unsigned char*) (A))[4])) << 8) +\
                                    (((uint32_t) (((unsigned char*) (A))[3])) << 16) +\
                                    (((uint32_t) (((unsigned char*) (A))[2])) << 24)) +\
                        (((uint64_t) (((uint32_t) (((unsigned char*) (A))[1])) +\
                                    (((uint32_t) (((unsigned char*) (A))[0]) << 8)))) <<\
                                     32))
#define bit_uint7korr(A) ((uint64_t)(((uint32_t) (((unsigned char*) (A))[6])) +\
                                    (((uint32_t) (((unsigned char*) (A))[5])) << 8) +\
                                    (((uint32_t) (((unsigned char*) (A))[4])) << 16) +\
                                   (((uint32_t) (((unsigned char*) (A))[3])) << 24)) +\
                        (((uint64_t) (((uint32_t) (((unsigned char*) (A))[2])) +\
                                    (((uint32_t) (((unsigned char*) (A))[1])) << 8) +\
                                    (((uint32_t) (((unsigned char*) (A))[0])) << 16))) <<\
                                     32))
#define bit_uint8korr(A) ((uint64_t)(((uint32_t) (((unsigned char*) (A))[7])) +\
                                    (((uint32_t) (((unsigned char*) (A))[6])) << 8) +\
                                    (((uint32_t) (((unsigned char*) (A))[5])) << 16) +\
                                    (((uint32_t) (((unsigned char*) (A))[4])) << 24)) +\
                        (((uint64_t) (((uint32_t) (((unsigned char*) (A))[3])) +\
                                    (((uint32_t) (((unsigned char*) (A))[2])) << 8) +\
                                    (((uint32_t) (((unsigned char*) (A))[1])) << 16) +\
                                    (((uint32_t) (((unsigned char*) (A))[0])) << 24))) <<\
                                    32))
////////////////////////////////////////////////////////////////////////////////////////
CLocalResultSet::CLocalResultSet()
{
	m_pRS = nullptr;
}

CLocalResultSet::~CLocalResultSet()
{
	if (m_pRS){
		mysql_free_result(m_pRS);
	}
}

my_ulonglong CLocalResultSet::GetRowsNumber()
{
	return mysql_num_rows(m_pRS);
}

unsigned int CLocalResultSet::GetFieldNumber()
{
	return mysql_num_fields(m_pRS);
}
//! 获取下一行
bool CLocalResultSet::NextRow()
{
	m_rowData = mysql_fetch_row(m_pRS);
	return m_rowData != nullptr;
}
//! 初始化列信息
void CLocalResultSet::Init()
{
	int nIndex = 0;
	MYSQL_FIELD* pField = nullptr;
	while(pField = mysql_fetch_field(m_pRS))//获取列名
	{
		m_mapColName.insert(make_pair(basiclib::CBasicString(pField->name, pField->name_length), nIndex));
		nIndex++;
	}
}

//! 获取数据
bool CLocalResultSet::getBoolean(uint32_t columnIndex)
{
	return getInt(columnIndex) ? true : false;
}
bool CLocalResultSet::getBoolean(const char* pColName)
{
	return getBoolean(GetColumeIndexByName(pColName));
}

int32_t CLocalResultSet::getInt(uint32_t columnIndex)
{
	return static_cast<int32_t>(getInt64(columnIndex));
}
int32_t CLocalResultSet::getInt(const char* pColName)
{
	return getInt(GetColumeIndexByName(pColName));
}

uint32_t CLocalResultSet::getUInt(uint32_t columnIndex)
{
	return static_cast<uint32_t>(getUInt64(columnIndex));
}
uint32_t CLocalResultSet::getUInt(const char* pColName)
{
	return getUInt(GetColumeIndexByName(pColName));
}

int64_t CLocalResultSet::getInt64(uint32_t columnIndex)
{
	return getUInt64(columnIndex);
}
int64_t CLocalResultSet::getInt64(const char* pColName)
{
	return getInt64(GetColumeIndexByName(pColName));
}

uint64_t CLocalResultSet::getUInt64(uint32_t columnIndex)
{
	const char* pRetString = getString(columnIndex);
	if (pRetString){
		MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pRS, columnIndex);
		if (pField){
			if (pField->type == FIELD_TYPE_BIT &&
				pField->flags != (BINARY_FLAG | UNSIGNED_FLAG)){
				uint64_t uval = 0;
				std::div_t length = std::div(pField->length, 8);
				if (length.rem) {
					++length.quot;
				}

				switch (length.quot) {
				case 8:uval = (uint64_t)bit_uint8korr(pRetString); break;
				case 7:uval = (uint64_t)bit_uint7korr(pRetString); break;
				case 6:uval = (uint64_t)bit_uint6korr(pRetString); break;
				case 5:uval = (uint64_t)bit_uint5korr(pRetString); break;
				case 4:uval = (uint64_t)bit_uint4korr(pRetString); break;
				case 3:uval = (uint64_t)bit_uint3korr(pRetString); break;
				case 2:uval = (uint64_t)bit_uint2korr(pRetString); break;
				case 1:uval = (uint64_t)bit_uint1korr(pRetString); break;
				}
				return uval;
			}
			if (pField->flags & UNSIGNED_FLAG) {
				return strtoull(pRetString, NULL, 10);
			}
			return strtoll(pRetString, NULL, 10);
		}
	}
	return 0.0f;
}
uint64_t CLocalResultSet::getUInt64(const char* pColName)
{
	return getUInt64(GetColumeIndexByName(pColName));
}

double CLocalResultSet::getDouble(uint32_t columnIndex)
{
	const char* pRetString = getString(columnIndex);
	if (pRetString){
		MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pRS, columnIndex);
		if (pField){
			if (pField->type == FIELD_TYPE_BIT){
				return static_cast<double>(getInt64(columnIndex));
			}
			return strtold(pRetString, nullptr);
		}
	}
	return 0.0f;
}
double CLocalResultSet::getDouble(const char* pColName)
{
	return getDouble(GetColumeIndexByName(pColName));
}

const char* CLocalResultSet::getString(uint32_t columnIndex)
{
	return m_rowData[columnIndex];
}

const char* CLocalResultSet::getString(const char* pColName)
{
	return getString(GetColumeIndexByName(pColName));
}

//! 根据列名获取index
uint32_t CLocalResultSet::GetColumeIndexByName(const char* pColName)
{
	MapColNameToIndexIterator iter = m_mapColName.find(pColName);
	if (iter != m_mapColName.end()){
		return iter->second;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
CLocalConnect::CLocalConnect(CMySQL_Connection* pConn)
{
	m_pConn = nullptr;
	m_pPool = nullptr;
}

CLocalConnect::~CLocalConnect()
{
	if (m_pPool && m_pConn)
	{
		m_pPool->ReleaseConnection(m_pConn);
	}
}

//! 绑定
void CLocalConnect::SetConnection(CMySQL_ConnectionPool* pPool, CMySQL_Connection* pConn)
{
	m_pPool = pPool;
	m_pConn = pConn;
}

//! 查询数据库
bool CLocalConnect::SelectMySQL(const char *q, CLocalResultSet& rs)
{
	bool bRet = false;
SELECTRSREPEAT:
	int nRet = mysql_query(m_pConn->m_pMySQL, q);
	if (0 != nRet){
		basiclib::BasicLogEventErrorV("SelectMySQL Error %d", nRet);
		return bRet;
	};
	rs.m_pRS = mysql_store_result(m_pConn->m_pMySQL);
	bRet = rs.m_pRS != nullptr;
	if (bRet){
		rs.Init();
	}
	return bRet;
}

int CLocalConnect::QueryMySQL(const char *q)
{
	int nRet = mysql_query(m_pConn->m_pMySQL, q);
	if (0 != nRet){
		basiclib::BasicLogEventErrorV("SelectMySQL Error %d", nRet);
		return -1;
	};
	return mysql_affected_rows(m_pConn->m_pMySQL);
}

//! 返回AUTO_INCREMENT列生成的ID
my_ulonglong CLocalConnect::GetInsertID()
{
	return mysql_insert_id(m_pConn->m_pMySQL);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMySQL_Connection::CMySQL_Connection()
{
}

CMySQL_Connection::~CMySQL_Connection()
{
}

bool CMySQL_Connection::InitMySQL(CMySQLConnector::OnMySQLConnNotify pNotify, CMySQLConnector_NetWapper* pWapper, Net_UInt nMaxPacketSize)
{
	return m_conn.InitConnector(pNotify, pWapper, nMaxPacketSize);
}

void CMySQL_Connection::MySQLNotifyType(CMySQLConnectorNotifyType notifyType, const char* pNotify)
{
	switch (notifyType){

	}
	basiclib::BasicLogEvent(pNotify);
}

//! 设置编码方式
int CMySQL_Connection::SetCharacterSet(const char *csname)
{
	return mysql_set_character_set(m_pMySQL, csname);
}

//! 设置连接选项
int CMySQL_Connection::SetOptions(enum mysql_option option, const void *arg)
{
	return mysql_options(m_pMySQL, option, arg);
}
int CMySQL_Connection::SetOptions4(enum mysql_option option, const void *arg1, const void *arg2)
{
	return mysql_options4(m_pMySQL, option, arg1, arg2);
}

bool CMySQL_Connection::ConnectToSQL(const char *host,
	const char *user,
	const char *passwd,
	const char *db,
	unsigned int port,
	const char *unix_socket,
	unsigned long clientflag)
{
	return mysql_real_connect(m_pMySQL, host, user, passwd, db, port, unix_socket, clientflag);
}

//! ping服务器保证连接正常
int CMySQL_Connection::PingMySQLServer()
{
	return mysql_ping(m_pMySQL);
}

//! 判断是否连接可用
int CMySQL_Connection::ReconnectMySQL()
{
	return PingMySQLServer();
}
/////////////////////////////////////////////////////////////////////////////////////////////
CMySQL_ConnectionPool::CMySQL_ConnectionPool()
{
	m_nMaxSize = 0;
}

CMySQL_ConnectionPool::~CMySQL_ConnectionPool()
{
	DestoryBasicConnPool();
}


bool CMySQL_ConnectionPool::Init(basiclib::CBasicString& url, basiclib::CBasicString& user, basiclib::CBasicString& password, int nInitSize, int maxSize,
	CMySQLConnector::OnMySQLConnNotify pNotify, OnCreateMySQLNetWapper pCreate, Net_UInt nMaxPacketSize)
{
	if (nInitSize > maxSize)
	{
		return false;
	}
	m_nMaxSize = maxSize;
	m_strUrl = url;
	m_strUsername = user;
	m_strPassword = password;
	m_pNotify = pNotify;
	m_create = pCreate;
	m_nMaxPacketSize = nMaxPacketSize;

	m_connectSafeList.reserve(nInitSize);
	m_createList.reserve(nInitSize);

	InitConnection(nInitSize);
	return true;
}

//在连接池中获得一个连接
bool CMySQL_ConnectionPool::GetConnection(CLocalConnect& localConnect)
{
	int nIndex = 0;
	CMySQL_Connection* pConn = nullptr;
	do{
		pConn = GetConnectionFromList();
	} while (pConn == nullptr && nIndex++ <= 3);
	localConnect.SetConnection(this, pConn);
	return pConn != NULL;
}

//! 获取连接，等到有为止
bool CMySQL_ConnectionPool::GetConnectionWaitForOne(CLocalConnect& localConnect, int nMaxTimes)
{
	int nWaitTimes = 0;
	CMySQL_Connection* pConn = NULL;
	while (pConn == NULL && nWaitTimes < nMaxTimes)
	{
		pConn = GetConnectionFromList();
		if (pConn)
		{
			basiclib::BasicSleep(1);
			nWaitTimes++;
		}
	}
	localConnect.SetConnection(this, pConn);
	return pConn != NULL;
}

//销毁连接池,首先要先销毁连接池的中连接
void CMySQL_ConnectionPool::DestoryBasicConnPool()
{
	m_connectSafeList.clear();

	for (auto& conn : m_createList){
		delete conn;
	}
	m_createList.clear();
}


//执行ontimer函数,确保每个链接不会被断开
void CMySQL_ConnectionPool::HandleOnTimer()
{
	for (auto& conn : m_createList){
		int nRet = conn->ReconnectMySQL();
		if (nRet != 0){
			basiclib::BasicLogEventErrorV("CMySQL_Connection ReconnectMySQL error:%d", nRet);
		}
	}
}

//回收数据库连接
void CMySQL_ConnectionPool::ReleaseConnection(CMySQL_Connection *conn)
{
	if (conn)
	{
		m_connectSafeList.push_back(conn);
	}
}


//初始化连接池，创建最大连接数的一半连接数量
void CMySQL_ConnectionPool::InitConnection(int iInitialSize)
{
	for (int i = 0; i < iInitialSize; i++)
	{
		m_connectSafeList.push_back(CreateConnection());
	}
}

//创建连接,返回一个Connection
CMySQL_Connection* CMySQL_ConnectionPool::CreateConnection()
{
	if (m_createList.size() < m_nMaxSize)
	{
		CMySQL_Connection* conn = new CMySQL_Connection();
		if (conn->InitMySQL(m_pNotify, m_create(), m_nMaxPacketSize)){
			if (conn->ConnectToSQL(m_strUrl.c_str(), m_strUsername.c_str(), m_strPassword.c_str(), nullptr)){
				m_createList.push_back(conn);
				//设置ping 自动重连
				bool opt_reconnect_value = true;
				conn->SetOptions(MYSQL_OPT_RECONNECT, (const char *)&opt_reconnect_value);
				return conn;
			}
		}
	}
	return NULL;
}

//! 获取一条连接
CMySQL_Connection* CMySQL_ConnectionPool::GetConnectionFromList()
{
	if (m_connectSafeList.size() == 0)
		return CreateConnection();
	CMySQL_Connection* pRet = m_connectSafeList[m_connectSafeList.size() - 1];
	m_connectSafeList.pop_back();
	if (pRet->ReconnectMySQL() != 0){
		m_connException.push_back(pRet);
		pRet = nullptr;
	}
	return pRet;
}
