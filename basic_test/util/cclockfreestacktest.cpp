//
//  cclockfreestacktest.cpp
//  basicTest
//
//  Created by 蔡振球 on 2018/8/4.
//

#include <basic.h>
#include "cclockfreestacktest.hpp"
#include "../headdefine.h"

class CCLockfreeValueNode : public basiclib::CCLockfreeStackNode{
public:
    CCLockfreeValueNode(){
        
    }
    virtual ~CCLockfreeValueNode(){
        
    }
    uint32_t nValue = 0;
    int nIndex = 0;
};

basiclib::CCLockfreeStack* pStacklockfree = nullptr;


class CCLockfreeTest{
public:
    CCLockfreeTest(){
    }
    virtual ~CCLockfreeTest(){
        delete []pStackAddNode;
    }
    void Init(int nIndex, uint32_t nCount){
        m_nCount = nCount;
        m_nReceive = m_nCount;
        pStackAddNode = new CCLockfreeValueNode[nCount];
        for(int i = 0;i < m_nCount;i++){
            pStackAddNode[i].nValue = i;
            pStackAddNode[i].nIndex = nIndex;
        }
    }
    CCLockfreeValueNode* GetPushCtx(){
        LONG lRet = basiclib::BasicInterlockedIncrement(&m_pushtimes);
        if(lRet >= m_nCount)
            return nullptr;
        return &pStackAddNode[lRet];
    }
    void Receive(CCLockfreeValueNode* p){
        
    }
    void Receiveseq(CCLockfreeValueNode* p){
        if(p->nValue + 1 != m_nReceive){
            printf("error nValue %d\n", p->nValue);
        }
        m_nReceive--;
    }
    bool checkIsSuccess(){
        return m_nReceive == 1;
    }
    CCLockfreeValueNode* pStackAddNode = nullptr;
    uint32_t m_nCount = 0;
    uint32_t m_nReceive = 0;
    LONG m_pushtimes = 0;
};
class CCLockfreeTestMgr{
public:
    CCLockfreeTestMgr(){
        
    }
    virtual ~CCLockfreeTestMgr(){
        
    }
    void Init(int nCount, LONG maxCount){
        m_nCreateCount = nCount;
        p = new CCLockfreeTest[nCount];
        for(int i = 0; i < nCount; i++){
            p[i].Init(i, maxCount);
        }
    }
    void Receive(CCLockfreeValueNode* pNode){
        p[pNode->nIndex].Receive(pNode);
    }
    void Receiveseq(CCLockfreeValueNode* pNode){
        p[pNode->nIndex].Receiveseq(pNode);
    }
    bool checkIsSuccess(){
        for(int i = 0; i < m_nCreateCount; i++){
            if(!p[i].checkIsSuccess())
                return false;
        }
        return true;
    }
    int             m_nCreateCount = 0;
    CCLockfreeTest* p = nullptr;
};

template<class F>
void CCLockfreeTestMgrRepeat(int nRepeatTimes, int nThreadCount, LONG maxPushTimes, F func){
    int nReceiveThreadCount = 1;
    for(int i = 0;i < nRepeatTimes;i++){
        CCLockfreeTestMgr* p = new CCLockfreeTestMgr();
        p->Init(nThreadCount, maxPushTimes);
        func(p, nReceiveThreadCount);
    }
}

THREAD_RETURN PushFunc(void* p){
    CCLockfreeTest* pTest = (CCLockfreeTest*)p;
    while (true){
        CCLockfreeValueNode* pRet = pTest->GetPushCtx();
        if (pRet == nullptr)
            break;
        pStacklockfree->Push(pRet);
    }
    return 0;
}

THREAD_RETURN PopFuncSeq(void* p){
    CCLockfreeTestMgr* pTest = (CCLockfreeTestMgr*)p;
    while (true){
        CCLockfreeValueNode* p = (CCLockfreeValueNode*)pStacklockfree->Pop();
        if (p == nullptr)
            break;
        pTest->Receiveseq(p);
    }
    return 0;
}

bool createCCLockfreeStackTest(const int nThreadCount, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPush, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPop){
    bool bRet = true;
    DWORD dwThreadServerID = 0;
    char szBuf[2][32];
    sprintf(szBuf[0], "ThreadCount(%d) Push", nThreadCount);
    CreateCalcUseTime(begin, PrintUseTime(szBuf[0], TIMES_FAST / nThreadCount * 5 * nThreadCount), false);
    sprintf(szBuf[1], "ThreadCount(%d) Pop", nThreadCount);
    CreateCalcUseTime(beginrece, PrintUseTime(szBuf[1], TIMES_FAST / nThreadCount * 5 * nThreadCount), false);
    
    CCLockfreeTestMgrRepeat(5, nThreadCount, TIMES_FAST / nThreadCount, [&](CCLockfreeTestMgr* pMgr, int nReceiveThreadCount){
        HANDLE thread[8];
        StartCalcUseTime(begin);
        for(int j = 0; j < nThreadCount; j++){
            thread[j] = basiclib::BasicCreateThread(lpStartAddressPush, &pMgr->p[j], &dwThreadServerID);
        }
        for(int j = 0; j < nThreadCount; j++){
            basiclib::BasicWaitThread(thread[j], -1);
        }
        EndCalcUseTimeCallback(begin, nullptr);
        StartCalcUseTime(beginrece);
        for(int j = 0; j < nReceiveThreadCount; j++){
            thread[j] = basiclib::BasicCreateThread(lpStartAddressPop, pMgr, &dwThreadServerID);
        }
        for(int j = 0; j < nReceiveThreadCount; j++){
            basiclib::BasicWaitThread(thread[j], -1);
        }
        EndCalcUseTimeCallback(beginrece, nullptr);
        if(!pMgr->checkIsSuccess()){
            bRet = false;
            printf("check fail\n");
        }
    });
    CallbackUseTime(begin);
    CallbackUseTime(beginrece);
    return bRet;
}

bool CCLockfreeStackTest(){
    bool bRet = true;
    pStacklockfree = new basiclib::CCLockfreeStack();
    DWORD dwThreadServerID = 0;
    //完整性
    bRet &= createCCLockfreeStackTest(1, PushFunc, PopFuncSeq);
    bRet &= createCCLockfreeStackTest(2, PushFunc, PopFuncSeq);
    bRet &= createCCLockfreeStackTest(4, PushFunc, PopFuncSeq);
    bRet &= createCCLockfreeStackTest(8, PushFunc, PopFuncSeq);

    delete pStacklockfree;
    return bRet;
}
