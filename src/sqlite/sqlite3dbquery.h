#ifndef BASIC_SQLITE3DBQUERY_H
#define BASIC_SQLITE3DBQUERY_H

#include "sqlite3.h"
#include <string>

#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
class _BASIC_DLL_API CCQLite3DBQuery : public basiclib::CBasicObject
{
public:
	CCQLite3DBQuery();
	CCQLite3DBQuery(sqlite3 *pdb, sqlite3_stmt *pStmt, bool bEof);
	CCQLite3DBQuery(const CCQLite3DBQuery &rQuery);
	CCQLite3DBQuery& operator= (const CCQLite3DBQuery &rQuery);
	virtual ~CCQLite3DBQuery();

	void InitQuery(sqlite3 *pdb, sqlite3_stmt *pStmt, bool bEof);

	int FieldNums();

	int FieldIndex(const char* szField);
	const char* FieldName(int nField);

	int FieldDataType(int nField);
	const char* FieldDeclType(int nField);

	const char* FieldValue(int nField);
	const char* FieldValue(const char *szField);

	bool FieldIsNull(int nField);
	bool FieldIsNull(const char *szField);

	bool GetIntValue(int nField, int &rDest);
	bool GetIntValue(const char *szField, int &rDest);

	bool GetFloatValue(int nField, double &rDest);
	bool GetFloatValue(const char *szField, double &rDest);

	bool GetStringValue(int nField, char *&rDest);
	bool GetStringValue(const char *szField, char *&rDest);

	bool Eof();

	void NextRow();

	void Finalize();
	std::string& GetLastError(){ return m_strLastError; }
private:
	bool CheckStmt();
private:
	sqlite3*					m_pDB;
	sqlite3_stmt*				m_pStmt;
	bool						m_bEof;
	int							m_nCols;
	std::string					m_strLastError;
};
#pragma warning (pop)

#endif