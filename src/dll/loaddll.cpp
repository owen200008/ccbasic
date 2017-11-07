#include "../inc/basic.h"
#include <stdint.h>
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
	m_strLoadFileName = lpszLibFileName;
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

#ifdef __BASICWINDOWS
PIMAGE_EXPORT_DIRECTORY GetExportDirectoryByDllModule(void* pModule)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pModule;
	if (IsBadReadPtr(pDosHeader, sizeof(IMAGE_DOS_HEADER)))
		return nullptr;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pModule + pDosHeader->e_lfanew);
	if (IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) )
		return nullptr;
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		return nullptr;
	return (PIMAGE_EXPORT_DIRECTORY)((PBYTE)pModule + pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
}

long GetOldIndexBinerySearch(DWORD dwIndex, PCSTR pstrFunctionName, void* pModule, PIMAGE_EXPORT_DIRECTORY pOldExportDirectory, PDWORD aryOldAddressOfNames)
{
	long lMid = 0;
	long lLow = 0;
	long lHigh = pOldExportDirectory->NumberOfNames - 1;
	int nCompareResult = 0;
	if (dwIndex > lHigh)
	{
		dwIndex = (lHigh + lLow) >> 1;
	}
	nCompareResult = strcmp(pstrFunctionName, (PCSTR)pModule + aryOldAddressOfNames[dwIndex]);
	if (nCompareResult == 0)
		return dwIndex;
	else if (nCompareResult > 0)
		lLow = dwIndex + 1;
	else
		lHigh = dwIndex - 1;

	while (lLow <= lHigh)
	{
		lMid = (lHigh + lLow) >> 1;
		nCompareResult = strcmp(pstrFunctionName, (PCSTR)pModule + aryOldAddressOfNames[lMid]);
		if (nCompareResult >= 0)
		{
			if (nCompareResult <= 0)
				return lMid;
			lLow = lMid + 1;
		}
		else
		{
			lHigh = lMid - 1;
		}
	}
	return -1;
}

#endif

#define LOGMAXSIZE 1024

#ifdef __BASICWINDOWS
bool ReplaceAddressForJump(PBYTE pOld, PBYTE pNew, char* pBuffer, const std::function<void(const char* pLog)>& logFunc){
#ifndef _WIN64
#pragma pack(push, 1)
	struct ImportTrunkDefine
	{
		unsigned char	m_cZhiLing;
		DWORD			m_dwAddress;
		bool IsJmp(){
			return m_cZhiLing == 0xe9;
		}
	};
#pragma pack()
	ImportTrunkDefine* pOldZhiLing = (ImportTrunkDefine*)(pOld);
	ImportTrunkDefine* pNewZhiLing = (ImportTrunkDefine*)(pNew);
	if (pOldZhiLing->IsJmp() && pNewZhiLing->IsJmp()){
		DWORD dwDis = (DWORD)pNewZhiLing - (DWORD)pOldZhiLing;
		dwDis += pNewZhiLing->m_dwAddress;
		pOldZhiLing->m_dwAddress = dwDis;
	}
	else{
		_snprintf(pBuffer, LOGMAXSIZE, "ReplaceDllFuncError:not jmp(%x)(%x)", pOldZhiLing->m_cZhiLing, pNewZhiLing->m_cZhiLing);
		logFunc(pBuffer);
		return false;
	}
#else
	logFunc("目前不知道win64");
	return false;
#endif
	return true;
}
#endif
//相同动态库替换接口
bool CBasicLoadDll::ReplaceDll(CBasicLoadDll& dll, const std::function<void(const char* pLog)>& logFunc)
{
#ifdef __BASICWINDOWS
	if (dll.m_hModule == nullptr || m_hModule == nullptr)
		return false;

	char szBuf[LOGMAXSIZE] = { 0 };
	PIMAGE_EXPORT_DIRECTORY pOldExportDirectory = GetExportDirectoryByDllModule(m_hModule);
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = GetExportDirectoryByDllModule(dll.m_hModule);
	if (pOldExportDirectory == nullptr || pExportDirectory == nullptr)
		return false;

	PDWORD aryOldAddressOfFunctions = (PDWORD)((PBYTE)m_hModule + pOldExportDirectory->AddressOfFunctions);
	PDWORD aryOldAddressOfNames = (PDWORD)((PBYTE)m_hModule + pOldExportDirectory->AddressOfNames);
	LPWORD aryOldAddressOfNameOrdinals = (LPWORD)((PBYTE)m_hModule + pOldExportDirectory->AddressOfNameOrdinals);

	PDWORD aryAddressOfFunctions = (PDWORD)((PBYTE)dll.m_hModule + pExportDirectory->AddressOfFunctions);
	PDWORD aryAddressOfNames = (PDWORD)((PBYTE)dll.m_hModule + pExportDirectory->AddressOfNames);
	LPWORD aryAddressOfNameOrdinals = (LPWORD)((PBYTE)dll.m_hModule + pExportDirectory->AddressOfNameOrdinals);

	BOOL bVirtualProtectRet = FALSE;
	DWORD dwOld = 0;
	DWORD dw = 0;
	for (DWORD dwIndex = 0; dwIndex < pExportDirectory->NumberOfNames; dwIndex++)
	{
		PCSTR pstrFunctionName = (PCSTR)dll.m_hModule + aryAddressOfNames[dwIndex];
		long lOldIndex = GetOldIndexBinerySearch(dwIndex, pstrFunctionName, m_hModule, pOldExportDirectory, aryOldAddressOfNames);
		if (lOldIndex < 0)
		{
			//找不到
			_snprintf(szBuf, LOGMAXSIZE, "找不到接口%s", pstrFunctionName);
			logFunc(szBuf);
			continue;
		}
		//basiclib::BasicLogEventV("Replace:%s", pstrFunctionName);
		if (IsBadWritePtr((PBYTE)m_hModule + aryOldAddressOfFunctions[aryOldAddressOfNameOrdinals[lOldIndex]], sizeof(intptr_t)))
		{
			bVirtualProtectRet = VirtualProtect((PBYTE)m_hModule + aryOldAddressOfFunctions[aryOldAddressOfNameOrdinals[lOldIndex]], sizeof(intptr_t), PAGE_EXECUTE_READWRITE, &dwOld);
			if (!ReplaceAddressForJump(((PBYTE)m_hModule + aryOldAddressOfFunctions[aryOldAddressOfNameOrdinals[lOldIndex]]), (PBYTE)dll.m_hModule + aryAddressOfFunctions[aryAddressOfNameOrdinals[dwIndex]], szBuf, logFunc)){
				_snprintf(szBuf, LOGMAXSIZE, "ReplaceDllFuncError:%s", pstrFunctionName);
				logFunc(szBuf);
			}
			else{
				_snprintf(szBuf, LOGMAXSIZE, "ReplaceDllFuncSuccess:%s", pstrFunctionName);
				logFunc(szBuf);
			}
			bVirtualProtectRet = VirtualProtect((PBYTE)m_hModule + aryOldAddressOfFunctions[aryOldAddressOfNameOrdinals[lOldIndex]], sizeof(intptr_t), dwOld, &dw);
		}
		else
		{
			if (!ReplaceAddressForJump(((PBYTE)m_hModule + aryOldAddressOfFunctions[aryOldAddressOfNameOrdinals[lOldIndex]]), (PBYTE)dll.m_hModule + aryAddressOfFunctions[aryAddressOfNameOrdinals[dwIndex]], szBuf, logFunc)){
				_snprintf(szBuf, LOGMAXSIZE, "ReplaceDllFuncError:%s", pstrFunctionName);
				logFunc(szBuf);
			}
			else{
				_snprintf(szBuf, LOGMAXSIZE, "ReplaceDllFuncSuccess:%s", pstrFunctionName);
				logFunc(szBuf);
			}
		}
	}
	
	return true;
#endif
}

__NS_BASIC_END
