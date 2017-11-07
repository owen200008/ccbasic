#include "../inc/basic.h"
CCQLite3DBQuery::CCQLite3DBQuery()
{
	m_pDB = NULL;
	m_pStmt = NULL;
	m_nCols = 0;
	m_bEof = true;
}

CCQLite3DBQuery::CCQLite3DBQuery(sqlite3 *pdb, sqlite3_stmt *pStmt, bool bEof)
{
	InitQuery(pdb, pStmt, bEof);
}
void CCQLite3DBQuery::InitQuery(sqlite3 *pdb, sqlite3_stmt *pStmt, bool bEof)
{
	m_pDB = pdb;
	m_pStmt = pStmt;
	m_bEof = bEof;
	m_nCols = sqlite3_column_count(pStmt);
}
CCQLite3DBQuery::CCQLite3DBQuery(const CCQLite3DBQuery &rQuery)
{
	m_pStmt = rQuery.m_pStmt;
	const_cast<CCQLite3DBQuery &>(rQuery).m_pStmt = NULL;
	m_nCols = rQuery.m_nCols;
	m_bEof = rQuery.m_bEof;
}

CCQLite3DBQuery& CCQLite3DBQuery::operator =(const CCQLite3DBQuery &rQuery)
{
	m_pStmt = rQuery.m_pStmt;
	const_cast<CCQLite3DBQuery &>(rQuery).m_pStmt = NULL;
	m_nCols = rQuery.m_nCols;
	m_bEof = rQuery.m_bEof;

	return *this;
}

CCQLite3DBQuery::~CCQLite3DBQuery()
{
	Finalize();
}

bool CCQLite3DBQuery::CheckStmt()
{
	if (m_pStmt == NULL)
	{
		return false;
	}
	return true;
}

int CCQLite3DBQuery::FieldNums()
{
	return m_nCols;
}

//根据字段名返回列索引  
int CCQLite3DBQuery::FieldIndex(const char* szField)
{
	if (!CheckStmt())
	{
		return -2;
	}

	if (szField)
	{
		for (int nField = 0; nField < m_nCols; nField++)
		{
			//后面还有很多类似的函数，参数差不多，需要一个sqlite3_stmt*和列索引值，这应该是内部查询了之后返回的结果，而不是事先保存的  
			const char *szTemp = sqlite3_column_name(m_pStmt, nField);
			if (strcmp(szTemp, szField) == 0)
			{
				return nField;
			}
		}
	}
	return -1;
}
const char* CCQLite3DBQuery::FieldName(int nField)
{
	if (!CheckStmt())
	{
		return NULL;
	}

	if (nField < 0 || nField > m_nCols - 1)
	{
		return NULL;
	}
	return sqlite3_column_name(m_pStmt, nField);
}

int CCQLite3DBQuery::FieldDataType(int nField)
{
	if (!CheckStmt())
	{
		return -1;
	}

	if (nField < 0 || nField > m_nCols - 1)
	{
		return -1;
	}
	return sqlite3_column_type(m_pStmt, nField);
}
const char* CCQLite3DBQuery::FieldDeclType(int nField)
{
	if (!CheckStmt())
	{
		return NULL;
	}

	if (nField < 0 || nField > m_nCols - 1)
	{
		return NULL;
	}
	return sqlite3_column_decltype(m_pStmt, nField);
}

const char* CCQLite3DBQuery::FieldValue(int nField)
{
	if (!CheckStmt())
	{
		return NULL;
	}

	if (nField < 0 || nField > m_nCols - 1)
	{
		return NULL;
	}
	return (const char*)sqlite3_column_text(m_pStmt, nField);
}

const char* CCQLite3DBQuery::FieldValue(const char *szField)
{
	int nField = FieldIndex(szField);
	return FieldValue(nField);
}

bool CCQLite3DBQuery::FieldIsNull(int nField)
{
	return (FieldDataType(nField) == SQLITE_NULL);
}

bool CCQLite3DBQuery::FieldIsNull(const char *szField)
{
	int nField = FieldIndex(szField);
	return (FieldDataType(nField) == SQLITE_NULL);
}

bool CCQLite3DBQuery::GetIntValue(int nField, int &rDest)
{
	if (FieldDataType(nField) == SQLITE_NULL)
	{
		return false;
	}
	else
	{
		rDest = sqlite3_column_int(m_pStmt, nField);
		return true;
	}
}

bool CCQLite3DBQuery::GetIntValue(const char *szField, int &rDest)
{
	int nField = FieldIndex(szField);
	return GetIntValue(nField, rDest);
}

bool CCQLite3DBQuery::GetFloatValue(int nField, double &rDest)
{
	if (FieldDataType(nField) == SQLITE_NULL)
	{
		return false;
	}
	else
	{
		rDest = sqlite3_column_double(m_pStmt, nField);
		return true;
	}
}
bool CCQLite3DBQuery::GetFloatValue(const char *szField, double &rDest)
{
	int nField = FieldIndex(szField);
	return GetFloatValue(nField, rDest);
}

bool CCQLite3DBQuery::GetStringValue(int nField, char *&rDest)
{
	if (FieldDataType(nField) == SQLITE_NULL)
	{
		return false;
	}
	else
	{
		rDest = const_cast<char *>((const char*)sqlite3_column_text(m_pStmt, nField));
		return true;
	}
}
bool CCQLite3DBQuery::GetStringValue(const char *szField, char *&rDest)
{
	int nField = FieldIndex(szField);
	return GetStringValue(nField, rDest);
}

bool CCQLite3DBQuery::Eof()
{
	if (!CheckStmt())
	{
		return true;
	}

	return m_bEof;
}

void CCQLite3DBQuery::NextRow()
{
	if (!CheckStmt())
	{
		m_bEof = true;
	}

	int nRet = sqlite3_step(m_pStmt);

	if (nRet == SQLITE_DONE)
	{
		// no rows  
		m_bEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do  
	}
	else
	{
		ASSERT(0);
		nRet = sqlite3_finalize(m_pStmt);
		m_pStmt = 0;
		m_strLastError = sqlite3_errmsg(m_pDB);
	}
}

void CCQLite3DBQuery::Finalize()
{
	if (m_pStmt)
	{
		int nRet = sqlite3_finalize(m_pStmt);
		m_pStmt = 0;
		if (nRet != SQLITE_OK)
		{
			ASSERT(0);
			m_strLastError = sqlite3_errmsg(m_pDB);
		}
	}
}