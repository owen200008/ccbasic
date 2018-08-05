#include "../inc/basic.h"

basiclib::CBasicString strPBZKChar = " %";

CPBZK::CPBZK(){
}

CPBZK::~CPBZK(){
    for(auto& iter : m_mapPBZK){
        delete (HashMapPBZK*)iter.second;
    }
    m_mapPBZK.clear();
}

void CPBZK::AddPBZKToMap(basiclib::CBasicStringArray& ayItems){
    int nSize = ayItems.GetSize();
    for(int i = 0; i < nSize; i++){
        if(ayItems[i].IsEmpty())
            continue;
        int nWordLength = ayItems[i].GetLength();

        HashMapPBZK* pMap = &m_mapPBZK;
        for(int j = 0; j < nWordLength; j++){
            char cKey = ayItems[i].GetAt(j);
            if(pMap->find(cKey) == pMap->end()){
                (*pMap)[cKey] = new HashMapPBZK();
            }
            pMap = (HashMapPBZK*)(*pMap)[cKey];
            if(j == nWordLength - 1){
                (*pMap)[PBZK_END_STRING] = NULL;
            }
        }
    }
}

void CPBZK::ReadPBZKFileBuffer(const char* pBuffer, int nLength){
    basiclib::CBasicStringArray ayItems;
    basiclib::BasicSpliteString(pBuffer, "\r\n", basiclib::IntoContainer_s<basiclib::CBasicStringArray>(ayItems));
    AddPBZKToMap(ayItems);
}

//! 判断是否有非法字符
int CPBZK::CheckPBZKExist(const char* txt, int nLength, int nBeginIndex, bool bDeep, bool bCheckSpecialZF){
    if(bCheckSpecialZF){
        if(strPBZKChar.Find(txt[nBeginIndex]) >= 0)
            return 1;
    }
    int matchFlag = 0;
    HashMapPBZK* pMap = &m_mapPBZK;
    bool bFind = false;
    for(int i = nBeginIndex; i < nLength; i++){
        char cKey = txt[i];
        if(pMap->find(cKey) == pMap->end()){
            break;
        }
        matchFlag++;
        pMap = (HashMapPBZK*)(*pMap)[cKey];
        if(pMap->find(PBZK_END_STRING) != pMap->end()){
            bFind = true;
            if(!bDeep){
                break;
            }
        }
    }
    return bFind ? matchFlag : 0;
}

bool CPBZK::IsContainPBZK(const char* txt, int nLength, bool bDeep, bool bCheckSpecialZF){
    for(int i = 0; i < nLength; i++){
        int nMatch = CheckPBZKExist(txt, nLength, i, bDeep, bCheckSpecialZF);
        if(nMatch > 0){
            return true;
        }
    }
    return false;
}

//发现直接替换
void CPBZK::ReplacePBZK(char* txt, int nLength, char cReplace, bool bDeep, bool bCheckSpecialZF){
    for(int i = 0; i < nLength; i++){
        int nMatch = CheckPBZKExist(txt, nLength, i, bDeep, bCheckSpecialZF);
        if(nMatch > 0){
            int nEnd = i + nMatch;
            for(; i < nEnd && i < nLength; i++){
                txt[i] = cReplace;
            }
        }
    }
}