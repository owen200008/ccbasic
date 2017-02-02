#ifndef BASIC_SQLITE3DB_H
#define BASIC_SQLITE3DB_H

#include "sqlite3dbquery.h"
#include "sqlite3dbtable.h"

class CCQLite3DB : public basiclib::CBasicObject
{
public:
	CCQLite3DB();
	virtual ~CCQLite3DB();

	bool Open(const char *szFileName, const char* pPwd = NULL, int nLength = 0);
	bool Close();
	//¥Úø™√‹¬Î
	int SetOpenPWD(const char* pPwd, int nLength);
	//…Ë÷√√‹¬Î
	int SetPWD(const char* pPwd, int nLength);

	bool GetDataToTable(const char *szSQL, CCQLite3DBTable* pTable);
	bool ExecQuery(const char *szSQL, CCQLite3DBQuery* pQuery);
	int ExecSQL(const char *szSQL);

	std::string& GetLastError(){ return m_strLastError; }
protected:
	sqlite3_stmt* Compile(const char *szSQL);
	bool CheckDB();
protected:
	sqlite3*					m_pDB;
	std::string					m_strLastError;
};

#endif