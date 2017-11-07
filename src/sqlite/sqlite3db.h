#ifndef BASIC_SQLITE3DB_H
#define BASIC_SQLITE3DB_H

#include "sqlite3dbquery.h"
#include "sqlite3dbtable.h"

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
class _BASIC_DLL_API CCQLite3DB : public basiclib::CBasicObject
{
public:
	CCQLite3DB();
	virtual ~CCQLite3DB();

	bool Open(const char *szFileName, const char* pPwd = NULL, int nLength = 0);
	bool Close();
	//打开密码
	int SetOpenPWD(const char* pPwd, int nLength);
	//设置密码
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
#pragma warning (pop)

#endif