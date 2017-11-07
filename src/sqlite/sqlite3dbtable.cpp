#include "../inc/basic.h"

///////////////////////////////////////////////////////////////////////////////////////////////
CCQLite3DBTable::CCQLite3DBTable()
{
	m_nCols = 0;
	m_nRows = 0;
	m_nCurrentRow = 0;
	m_paszResults = NULL;
}

void CCQLite3DBTable::ClearNoRelease(){
	m_paszResults = NULL;
	m_paszResults = 0;
}

CCQLite3DBTable::~CCQLite3DBTable()
{
	finalizeClose();
}
int CCQLite3DBTable::NumOfFields()
{
	return m_nCols;
}

int CCQLite3DBTable::NumOfRows() 
{
	return m_nRows;
}

const char* CCQLite3DBTable::NameOfField(int nField)
{
	if (!CheckResluts())
	{
		return NULL;
	}

	if (nField < 0 || nField > m_nCols - 1)
	{
		return NULL;
	}

	return m_paszResults[nField]; //一位数组的头m_nCols个元素存放的是表的字段名称，存储具体位置是m_paszResults[0,,,m_nCols-1]。  
}

const char* CCQLite3DBTable::ValueOfField(int nField)
{
	if (!CheckResluts())
	{
		return NULL;
	}

	if (nField < 0 || nField > m_nCols - 1)
	{
		return NULL;
	}

	//根据要查询的当前行与列值算出在一位数组中的索引下标，额外加一个m_nCols是第一行存储的是字段名  
	int nIndex = m_nCurrentRow*m_nCols + m_nCols + nField;
	return m_paszResults[nIndex];
}

//根据字段名称来访问某一列的数据  
const char* CCQLite3DBTable::ValueOfField(const char *szField)
{
	if (!CheckResluts())
	{
		return NULL;
	}

	if (szField)
	{
		for (int nField = 0; nField < m_nCols; nField++)
		{
			if (strcmp(szField, m_paszResults[nField]) == 0)
			{
				int nIndex = m_nCurrentRow*m_nCols + m_nCols + nField;
				return m_paszResults[nIndex];
			}
		}
	}
	return NULL;
}


bool CCQLite3DBTable::FieldIsNull(int nField)
{
	if (!CheckResluts())
	{
		return true;
	}

	return (ValueOfField(nField) == 0);
}

bool CCQLite3DBTable::FieldIsNull(const char* szField)
{
	if (!CheckResluts())
	{
		return true;
	}

	return (ValueOfField(szField) == 0);
}

double CCQLite3DBTable::GetDoubleField(const char *szField)
{
	double dRet = 0;
	if (!FieldIsNull(szField))
	{
		dRet = atof(ValueOfField(szField));
	}
	return dRet;
}

unsigned int CCQLite3DBTable::GetUIntField(const char *szField)
{
	unsigned int uRet = 0;
	if (!FieldIsNull(szField))
	{
		uRet = atol(ValueOfField(szField));
	}
	return uRet;
}
int CCQLite3DBTable::GetIntField(const char *szField)
{
	int uRet = 0;
	if (!FieldIsNull(szField))
	{
		uRet = atol(ValueOfField(szField));
	}
	return uRet;
}
const char* CCQLite3DBTable::GetStringField(const char *szField)
{
	if (!FieldIsNull(szField))
	{
		return const_cast<char *>(ValueOfField(szField));
	}
	return "";
}

//在每一次需要获取数据的时候都要设置要访问的行值  
bool CCQLite3DBTable::SetRow(int nRow)
{
	if (!CheckResluts())
	{
		return false;
	}

	if (nRow < 0 || nRow > m_nRows - 1)
	{
		return false;
	}

	m_nCurrentRow = nRow;
	return true;
}

bool CCQLite3DBTable::CheckResluts()
{
	if (m_paszResults == NULL)
	{
		return false;
	}
	return true;
}
void CCQLite3DBTable::finalizeClose()
{
	if (m_paszResults)
	{
		sqlite3_free_table(m_paszResults);  //利用库函数销毁表存储内容  
		m_paszResults = 0;
	}
}
void CCQLite3DBTable::InitTable(char **paszResults, int nRows, int nCols)
{
	m_paszResults = paszResults; //给出一个一维指针数组，初始化一个表  
	m_nCols = nCols;
	m_nRows = nRows;
	m_nCurrentRow = 0;
}

