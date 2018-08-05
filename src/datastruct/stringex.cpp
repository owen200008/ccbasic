#include "../inc/basic.h"
#include "stringex.h"

using namespace basiclib;

__NS_BASIC_START

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBasicStringSource{
    friend class CParseSource;
    friend class CBasicString;
    friend bool BasicLoadStringSource(const char* lpszFile, const char* lpszModuleName);
    friend const char* BasicLoadString(const char* lpszFormatID, const char* lpszModuleName);
public:
    CBasicStringSource(void);
    ~CBasicStringSource(void);
protected:
    bool LoadProfile(const char* lpszFile, const char* lpszModuleName = "");

    // 通过模块名从配置文件中找到该资源并返回
    const char* SetIDToSource(const char* lpszFormatID, const char* lpszID, const char* lpszModuleName);

    const char* LoadString(const char* lpszFormatID, const char* lpszModuleName = "");

    //设置进map缓存
    const char* FindStringSource(const char* lpszID);

    //设置进map缓存
    void	SetToMapCache(const char* lpszID, const char* lpszSource);
protected:
    typedef basiclib::basic_map<basiclib::tstring_s, basiclib::tstring_s>	IDToSourceContainer;
    IDToSourceContainer		m_conIDToSource;
    CCriticalSection		m_csIDToSource;

    typedef basiclib::basic_map<basiclib::tstring_s, basiclib::CBasicIniOp*> ModuleToIniFileContainer;
    ModuleToIniFileContainer m_conModuleToIniFile;
};


CBasicStringSource::CBasicStringSource(void){
}


CBasicStringSource::~CBasicStringSource(void){
    for_each(m_conModuleToIniFile.begin(), m_conModuleToIniFile.end(), DeleteObjectAux());
}


bool CBasicStringSource::LoadProfile(const char* lpszFile, const char* lpszModuleName){
    if(IsStringEmpty(lpszModuleName)){
        return false;
    }

    if(lpszFile && lpszModuleName){
        ModuleToIniFileContainer::iterator iter = m_conModuleToIniFile.find(lpszModuleName);
        if(m_conModuleToIniFile.end() != iter){
            return false;
        }

        CBasicIniOp* pIniFile = new CBasicIniOp();
        if(pIniFile->InitFromFile(lpszFile) > 0){
            m_conModuleToIniFile[lpszModuleName] = pIniFile;
            return true;
        }
        else{
            delete pIniFile;
            pIniFile = NULL;
        }
    }
    return false;
}

void CBasicStringSource::SetToMapCache(const char* lpszID, const char* lpszSource){
    CSingleLock lock(&m_csIDToSource, true);
    m_conIDToSource[lpszID] = lpszSource;
}

const char* CBasicStringSource::FindStringSource(const char* lpszID){
    CSingleLock lock(&m_csIDToSource, true);
    IDToSourceContainer::iterator iter = m_conIDToSource.find(lpszID);
    if(m_conIDToSource.end() != iter){
        return iter->second.c_str();
    }
    return NULL;
}

const char* CBasicStringSource::SetIDToSource(const char* lpszFormatID, const char* lpszID, const char* lpszModuleName){
    CBasicString strSource;
    ModuleToIniFileContainer::iterator iter = m_conModuleToIniFile.find(lpszModuleName);
    if(m_conModuleToIniFile.end() == iter){
        return NULL;
    }
    else{
        CBasicIniOp* pIniFile = iter->second;
        strSource = pIniFile->GetData(lpszModuleName, lpszFormatID, "");
        if(strSource.IsEmpty())
            return NULL;
    }

    //特殊符号转义
    char *p = (char*)strSource.c_str();
    char* pOriginal = p;
    while(*p != '\0'){
        if(*p == '\\'){
            bool bMatch = true;
            switch(*(p + 1)){
            case 'n':
                *p = '\n';
                break;
            case 'r':
                *p = '\r';
                break;
            case 't':
                *p = '\t';
                break;
            default:
                bMatch = false;
                break;
            }
            if(bMatch){
                int nLen = strlen(p + 2);
                memmove(p + 1, p + 2, nLen * sizeof(char));
                *(p + 1 + nLen) = '\0';
            }
        }
        p++;
    }
    SetToMapCache(lpszID, pOriginal);

    return FindStringSource(lpszID);
}

const char* CBasicStringSource::LoadString(const char* lpszFormatID, const char* lpszModuleName){
    CBasicString strID;
    strID.Format("%s%s", lpszModuleName, lpszFormatID);
    const char* lpszRet = FindStringSource(strID.c_str());
    if(NULL == lpszRet){
        // 找不到则从配置文件中读取
        lpszRet = SetIDToSource(lpszFormatID, strID.c_str(), lpszModuleName);
    }
    return lpszRet;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef basiclib::CBasicSingleton<CBasicStringSource> SingletonString_sSource;

bool BasicLoadStringSource(const char* lpszFile, const char* lpszModuleName/* = _T("")*/){
    return (&SingletonString_sSource::Instance())->LoadProfile(lpszFile, lpszModuleName);
}


const char* BasicLoadString(const char* lpszFormatID, const char* lpszModuleName){
    return (&SingletonString_sSource::Instance())->LoadString(lpszFormatID, lpszModuleName);
}

const char* BasicLoadString(unsigned long ulID, const char* lpszModuleName){
    char szBuf[32];
    memset(szBuf, 0, 32);
    sprintf(szBuf, "%d", ulID);
    return BasicLoadString(szBuf, lpszModuleName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CWBasicString类支持字符串资源加载
void CBasicString::FormatS(const char* lpszModuleName, const char* lpszFormatID, ...){
    const char* lpszForamt = BasicLoadString(lpszFormatID, lpszModuleName);
    if(lpszForamt != NULL){
        va_list argList;
        va_start(argList, lpszFormatID);
        FormatV(lpszForamt, argList);
        va_end(argList);
    }
    else{
        ASSERT(false);
    }
}
void CBasicString::FormatS(const char* lpszModuleName, DWORD dwFormatID, ...){
    char szBuf[16] = { 0 };
    sprintf(szBuf, "%X", dwFormatID);
    const char* lpszForamt = BasicLoadString(szBuf, lpszModuleName);
    if(lpszForamt != NULL){
        va_list argList;
        va_start(argList, dwFormatID);
        FormatV(lpszForamt, argList);
        va_end(argList);
    }
    else{
        ASSERT(false);
    }
}

bool CBasicString::LoadString(const char* lpszFormatID, const char* lpszModuleName/* = _T("")*/){
    const char* lpszForamt = BasicLoadString(lpszFormatID, lpszModuleName);
    if(lpszForamt){
        assign(lpszForamt);
        return true;
    }
    return false;
}
bool CBasicString::LoadString(DWORD dwFormatID, const char* lpszModuleName){
    char szBuf[16] = { 0 };
    sprintf(szBuf, "%X", dwFormatID);
    return LoadString(szBuf, lpszModuleName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


__NS_BASIC_END

