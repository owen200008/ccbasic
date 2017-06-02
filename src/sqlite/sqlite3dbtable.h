#ifndef BASIC_SQLITE3DBTABLE_H
#define BASIC_SQLITE3DBTABLE_H

class _BASIC_DLL_API CCQLite3DBTable : public basiclib::CBasicObject
{
public:
	CCQLite3DBTable();
	virtual ~CCQLite3DBTable();

	void InitTable(char **paszResults, int nRows, int nCols);

	int NumOfFields();
	int NumOfRows();

	const char* NameOfField(int nField);

	const char* ValueOfField(int nField);
	const char* ValueOfField(const char *szField);

	bool FieldIsNull(int nField);
	bool FieldIsNull(const char *szField);

	unsigned int GetUIntField(const char *szField);
	int GetIntField(const char *szField);
	double GetDoubleField(const char *szField);

	const char* GetStringField(const char *szField);

	bool SetRow(int nRow);

	void finalizeClose();
protected:
	bool CheckResluts();
protected:
	int							m_nCurrentRow;
	int							m_nRows;
	int							m_nCols;
	char**						m_paszResults;
};

#endif