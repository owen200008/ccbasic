#include "../inc/basic.h"

CCQLite3DB::CCQLite3DB()
{
	m_pDB = NULL;
}
CCQLite3DB::~CCQLite3DB()
{
	Close();
}

bool CCQLite3DB::Open(const char *szFileName, const char* pPwd, int nLength)
{
	int nRet = sqlite3_open(szFileName, &m_pDB);
	if (nRet != SQLITE_OK)
	{
		m_strLastError = sqlite3_errmsg(m_pDB);
		return false;
	}
	if (pPwd)
	{
		nRet = SetOpenPWD(pPwd, nLength);
		if (nRet != SQLITE_OK)
		{
			m_strLastError = sqlite3_errmsg(m_pDB);
			return false;
		}

		CCQLite3DBTable tableDB;
		if (!GetDataToTable("select count(*) FROM sqlite_master WHERE type='table'", &tableDB))
		{
			//使用没有密码的打开
			Close();
			return Open(szFileName, NULL, 0);
		}
	}
	return true;
}

bool CCQLite3DB::Close()
{
	if (m_pDB)
	{
		if (sqlite3_close(m_pDB) == SQLITE_OK)
		{
			m_pDB = 0; //一旦关闭数据库指针，要置为0，防止多次关闭出错  
		}
		else
		{
			ASSERT(0);
			m_strLastError = "Unable to close database";
			return false;
		}
	}
	return true;
}

//打开密码
int CCQLite3DB::SetOpenPWD(const char* pPwd, int nLength)
{
	return sqlite3_key(m_pDB, pPwd, nLength);
}
//设置密码
int CCQLite3DB::SetPWD(const char* pPwd, int nLength)
{
	return sqlite3_rekey(m_pDB, pPwd, nLength);
}

bool CCQLite3DB::GetDataToTable(const char *szSQL, CCQLite3DBTable* pTable)
{
	if (!CheckDB())
	{
		return false;
	}

	char* szError = 0;
	char** paszResults = 0;
	int nRet;
	int nRows(0);
	int nCols(0);

	nRet = sqlite3_get_table(m_pDB, szSQL, &paszResults, &nRows, &nCols, &szError);

	if (nRet == SQLITE_OK && pTable)
	{
		pTable->InitTable(paszResults, nRows, nCols);
	}
	else
	{
		m_strLastError = szError;
		return false;
	}
	return true;
}
int CCQLite3DB::ExecSQL(const char *szSQL)
{
	if (!CheckDB())
	{
		return -1;
	}

	char* szError = 0;

	int nRet = sqlite3_exec(m_pDB, szSQL, 0, 0, &szError);

	if (nRet == SQLITE_OK)
	{
		return sqlite3_changes(m_pDB);  //返回这个执行影响的行数  
	}
	else
	{
		m_strLastError = szError;
		sqlite3_free(szError);
		return -1;
	}
}
bool CCQLite3DB::CheckDB()
{
	if (!m_pDB)
	{
		ASSERT(0);
		m_strLastError = "Database not open";
		return false;
	}
	return true;
}

sqlite3_stmt* CCQLite3DB::Compile(const char *szSQL)
{
	if (!CheckDB())
	{
		return NULL;
	}

	const char *szTail = 0;
	sqlite3_stmt *pStmt;

	int nRet = sqlite3_prepare(m_pDB, szSQL, -1, &pStmt, &szTail);

	if (nRet != SQLITE_OK)
	{
		m_strLastError = sqlite3_errmsg(m_pDB);
		return NULL;
	}
	return pStmt;
}
////////////////////////////////////////////////////////////////////////////////
bool CCQLite3DB::ExecQuery(const char *szSQL, CCQLite3DBQuery* pQuery)
{
	if (!CheckDB())
	{
		return false;
	}

	//编译一条指针对象，用临时变量存储，传递给CCQLite3DBQuery后，  
	//这个临时sqlite3_stmt*对象会自动消失，最后只有一份保留在CCQLite3DBQuery中  
	sqlite3_stmt *pStmt = Compile(szSQL);
	if (NULL == pStmt)
	{
		return false;
	}

	int nRet = sqlite3_step(pStmt);

	if (nRet == SQLITE_DONE) //表明这个查询没有返回结果  
	{
		pQuery->InitQuery(m_pDB, pStmt, true);
	}
	else if (nRet == SQLITE_ROW) //这个查询中至少有一行记录  
	{
		pQuery->InitQuery(m_pDB, pStmt, false);
	}
	else
	{
		nRet = sqlite3_finalize(pStmt);
		m_strLastError = sqlite3_errmsg(m_pDB);
		return false;
	}
	return true;
}

