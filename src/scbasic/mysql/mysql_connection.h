#ifndef _MYSQL_CONNECTION_H_
#define _MYSQL_CONNECTION_H_

#include <basic.h>
#include "mysqlconnector.h"

class CLocalConnect;
class CLocalResultSet : public basiclib::CBasicObject
{
public:
	CLocalResultSet();
	virtual ~CLocalResultSet();

	//! 获取行数
	my_ulonglong GetRowsNumber();
	//! 获取域个数
	unsigned int GetFieldNumber();

	//! 获取下一行
	bool NextRow();
	//! 获取数据
	bool getBoolean(uint32_t columnIndex);
	bool getBoolean(const char* pColName);

	int32_t getInt(uint32_t columnIndex);
	int32_t getInt(const char* pColName);

	uint32_t getUInt(uint32_t columnIndex);
	uint32_t getUInt(const char* pColName);

	int64_t getInt64(uint32_t columnIndex);
	int64_t getInt64(const char* pColName);

	uint64_t getUInt64(uint32_t columnIndex);
	uint64_t getUInt64(const char* pColName);

	double getDouble(uint32_t columnIndex);
	double getDouble(const char* pColName);

	const char* getString(uint32_t columnIndex);
	const char* getString(const char* pColName);
protected:
	//! 初始化列信息
	void Init();
	//! 根据列名获取index
	uint32_t GetColumeIndexByName(const char* pColName);
protected:
	typedef basiclib::basic_map<basiclib::CBasicString, int>	MapColNameToIndex;
	typedef MapColNameToIndex::iterator							MapColNameToIndexIterator;

	MYSQL_RES*			m_pRS;
	MYSQL_ROW			m_rowData;
	MapColNameToIndex	m_mapColName;

	friend class CLocalConnect;
};

class CMySQL_ConnectionPool;
class CMySQL_Connection;
class CLocalConnect : public basiclib::CBasicObject
{
public:
	CLocalConnect(CMySQL_Connection* pConn);
	virtual ~CLocalConnect();

	//! 绑定
	void SetConnection(CMySQL_ConnectionPool* pPool, CMySQL_Connection* pConn);

	//! 查询数据库
	bool SelectMySQL(const char *q, CLocalResultSet& rs);
	//! 更新数据库, 返回影响条数
	int QueryMySQL(const char *q);

	//! 返回AUTO_INCREMENT列生成的ID
	my_ulonglong GetInsertID();
protected:
	CMySQL_ConnectionPool*	m_pPool;
	CMySQL_Connection*		m_pConn;

};

class CMySQL_Connection : public basiclib::CBasicObject
{
public:
	CMySQL_Connection();
	virtual ~CMySQL_Connection();

	//! 初始化数据库
	bool InitMySQL(CMySQLConnector::OnMySQLConnNotify pNotify, CMySQLConnector_NetWapper* pWapper, Net_UInt nMaxPacketSize = MYSQL_DEFAULT_MAX_PACKET_SIZE);
	//! 设置编码方式
	int SetCharacterSet(const char *csname);
	//! 设置连接选项
	int SetOptions(enum mysql_option option,const void *arg);
	int SetOptions4(enum mysql_option option, const void *arg1, const void *arg2);
	//! 连接数据库
	bool ConnectToSQL(const char *host,
		const char *user,
		const char *passwd,
		const char *db,
		unsigned int port = 3306,
		const char *unix_socket = nullptr,
		unsigned long clientflag = 0);

	//! 判断是否连接可用PingMySQLServer返回一致
	int ReconnectMySQL();
protected:
	void MySQLNotifyType(CMySQLConnectorNotifyType notifyType, const char* pNotify);
protected:
	//! ping服务器保证连接正常
	int PingMySQLServer();
protected:
	CMySQLConnector	m_conn;

	friend class CLocalConnect;
};

typedef fastdelegate::FastDelegate0<CMySQLConnector_NetWapper*> OnCreateMySQLNetWapper;
class CMySQL_ConnectionPool : public basiclib::CBasicObject
{
public:
	CMySQL_ConnectionPool();
	virtual ~CMySQL_ConnectionPool();

	//! 第一次必须初始化
	bool Init(basiclib::CBasicString& url, basiclib::CBasicString& user, basiclib::CBasicString& password, int nInitSize, int maxSize, 
		CMySQLConnector::OnMySQLConnNotify pNotify, OnCreateMySQLNetWapper pCreate, Net_UInt nMaxPacketSize = MYSQL_DEFAULT_MAX_PACKET_SIZE);
	//! 初始化之后才能获取连接
	bool GetConnection(CLocalConnect& localConnect); //获得数据库连接
	//! 获取连接，等到有为止
	bool GetConnectionWaitForOne(CLocalConnect& localConnect, int nMaxTimes = 10000);
	//! 销毁数据库连接池
	void DestoryBasicConnPool();

	//执行ontimer函数,确保每个链接不会被断开
	void HandleOnTimer();
private:
	void ReleaseConnection(CMySQL_Connection *conn); //将数据库连接放回到连接池的容器中

	void InitConnection(int iInitialSize); //初始化数据库连接池
	CMySQL_Connection*	CreateConnection(); //创建一个连接
	//! 获取一条连接
	CMySQL_Connection* GetConnectionFromList();

private:
	typedef basiclib::basic_vector<CMySQL_Connection*>	VTConnect;

	VTConnect						m_connException;
	VTConnect						m_connectSafeList;
	VTConnect						m_createList;


	basiclib::CBasicString	m_strUrl;
	basiclib::CBasicString	m_strUsername;
	basiclib::CBasicString	m_strPassword;
	unsigned int			m_nMaxSize; //连接池中定义的最大数据库连接数

	CMySQLConnector::OnMySQLConnNotify	m_pNotify;
	OnCreateMySQLNetWapper				m_create;
	Net_UInt							m_nMaxPacketSize;
	CMySQLConnector_NetWapper* pWapper, 

	friend class CLocalConnect;
};


#endif // _MYSQL_CONNECTION_H_
