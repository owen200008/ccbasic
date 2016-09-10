#include "../inc/basic.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__NS_BASIC_START
CBasicLoadDll::CBasicLoadDll()
{
	m_hModule = NULL;
}
CBasicLoadDll::CBasicLoadDll(const char* lpszLibFileName)
{
	m_hModule = NULL;
	LoadLibrary(lpszLibFileName);
}

CBasicLoadDll::~CBasicLoadDll()
{
	FreeLibrary();
}

long CBasicLoadDll::LoadLibrary(const char* lpszLibFileName)
{
	FreeLibrary();
	if(lpszLibFileName == NULL || lpszLibFileName[0] == '\0')
	{
		return -1;
	}
	m_hModule = BasicLoadLibrary(lpszLibFileName);
	if(m_hModule == NULL)
	{
		return -1;
	}
	return 0;
}

long CBasicLoadDll::FreeLibrary()
{
	if(m_hModule != NULL)
	{
		BasicFreeLibrary(m_hModule);
	}
	m_hModule = NULL;
	return 0;
}

void* CBasicLoadDll::GetProcAddress(const char* lpszProcName)
{
	if(lpszProcName == NULL || lpszProcName[0] == '\0')
	{
		return NULL;
	}
	return BasicGetProcAddress(m_hModule, lpszProcName);
}

__NS_BASIC_END
