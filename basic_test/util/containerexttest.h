#ifndef INC_CONTAINEXTTEST_H
#define INC_CONTAINEXTTEST_H

#include <basic.h>
#include "../headdefine.h"
#include "cclockfreequeue.h"
#include "cbasicqueuearray.h"

struct ctx_message{
    uint32_t        m_nCtxID;
    uint32_t        m_nIndex;
    ctx_message(){
        memset(this, 0, sizeof(ctx_message));
    }
    ctx_message(uint32_t ctxid){
        memset(this, 0, sizeof(ctx_message));
        m_nCtxID = ctxid;
    }
    ~ctx_message(){
    }
};

class CBasicQueryArrayTest{
public:
    LONG m_pushMaxTimes = 0;
    LONG m_pushtimes = 0;
    uint32_t m_nIndex = 0;
    
    LONG m_nreceive = 0;
    
    CBasicQueryArrayTest(){
        
    }
    void setIndex(uint32_t nIndex){
        m_nIndex = nIndex;
    }
    ctx_message* GetPushCtx(ctx_message& msg){
        LONG lRet = basiclib::BasicInterlockedIncrement(&m_pushtimes);
        if(lRet > m_pushMaxTimes)
            return nullptr;
        msg.m_nCtxID = lRet;
        msg.m_nIndex = m_nIndex;
        return &msg;
    }
    void receiveIndex(ctx_message& msg){
        ASSERT(m_nreceive + 1 == msg.m_nCtxID);
        m_nreceive = msg.m_nCtxID;
    }
    void receiveCount(ctx_message& msg){
        basiclib::BasicInterlockedIncrement(&m_nreceive);
    }
    bool checkIsSuccess(){
        return m_pushMaxTimes == m_nreceive;
    }
    
};

enum TestType{
    TestType_PushComplate = 0,
    TestType_Count,
    TestType_CountToken,
    TestType_Speed
};

class CBasicQueryArrayTestMgr{
public:
    CBasicQueryArrayTestMgr(){
        
    }
    void Init(int nCount, LONG maxCount, TestType testType){
        m_testType = testType;
        m_nCreateCount = nCount;
        p = new CBasicQueryArrayTest[nCount];
        for(int i = 0; i < nCount; i++){
            p[i].setIndex(i);
            p[i].m_pushMaxTimes = maxCount;
        }
    }
    ~CBasicQueryArrayTestMgr(){
        if(p){
            delete[]p;
        }
    }
    void receiveIndex(ctx_message& msg){
        p[msg.m_nIndex].receiveIndex(msg);
    }
    void receiveCount(ctx_message& msg){
        p[msg.m_nIndex].receiveCount(msg);
    }
    bool checkIsSuccess(){
        for(int i = 0; i < m_nCreateCount; i++){
            if(!p[i].checkIsSuccess())
                return false;
        }
        return true;
    }
    TestType        m_testType = TestType_PushComplate;
    int             m_nCreateCount = 0;
    CBasicQueryArrayTest* p = nullptr;
};

class CBasicQueryArrayTestMgrMulti{
public:
    template<class F>
    CBasicQueryArrayTestMgrMulti(int nRepeatTimes, int nThreadCount, TestType testType, LONG maxPushTimes, F func){
        m_nRepeatTimes = nRepeatTimes;
        m_pMgr = new CBasicQueryArrayTestMgr[nRepeatTimes];
        for(int i = 0; i < nRepeatTimes; i++){
            m_pMgr[i].Init(nThreadCount, maxPushTimes, testType);
        }
        int nReceiveThreadCount = 1;
        if(testType != TestType_PushComplate){
            nReceiveThreadCount = nThreadCount;
        }
        for(int i = 0; i < nRepeatTimes; i++){
            func(this, &m_pMgr[i], nReceiveThreadCount);
        }
    }
    ~CBasicQueryArrayTestMgrMulti(){
        if(m_pMgr){
            delete[]m_pMgr;
        }
    }
    
    int m_nRepeatTimes = 0;
    CBasicQueryArrayTestMgr* m_pMgr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
//self define queue
CBasicQueueArray<ctx_message> basicQueue;
THREAD_RETURN CBasicQueueThreadPush(void* arg){
    ctx_message msg;
    CBasicQueryArrayTest* pTest = (CBasicQueryArrayTest*)arg;
    while (true){
        ctx_message* pRet = pTest->GetPushCtx(msg);
        if (pRet == nullptr)
            break;
        basicQueue.Push(msg);
    }
    return 0;
}

THREAD_RETURN CBasicQueueThreadPop(void* arg){
    ctx_message msg;
    CBasicQueryArrayTestMgr* pTestMgr = (CBasicQueryArrayTestMgr*)arg;
    switch(pTestMgr->m_testType){
    case TestType_PushComplate:
    {
        while(true){
            if(!basicQueue.Pop(msg)){
                break;
            }
            pTestMgr->receiveIndex(msg);
        }
        break;
    }
    case TestType_CountToken:
    case TestType_Count:
    {
        while(true){
            if(!basicQueue.Pop(msg)){
                break;
            }
            pTestMgr->receiveCount(msg);
        }
        break;
    }
    case TestType_Speed:
    {
        while(true){
            if(!basicQueue.Pop(msg)){
                break;
            }
        }
        break;
    }
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
//lockfree
basiclib::CLockFreeMessageQueue<ctx_message> conMsgQueue;
THREAD_RETURN ConcurrentQueueThreadPush(void* arg){
    ctx_message msg;
    CBasicQueryArrayTest* pTest = (CBasicQueryArrayTest*)arg;
    while(true){
        ctx_message* pRet = pTest->GetPushCtx(msg);
        if(pRet == nullptr)
            break;
        conMsgQueue.enqueue(std::move(*pRet));
    }
    return 0;
}
THREAD_RETURN ConcurrentQueueThreadPop(void* arg){
    ctx_message msg;
    CBasicQueryArrayTestMgr* pTestMgr = (CBasicQueryArrayTestMgr*)arg;
    switch(pTestMgr->m_testType){
    case TestType_PushComplate:
    {
        while(true){
            if(!conMsgQueue.try_dequeue(msg)){
                break;
            }
            pTestMgr->receiveIndex(msg);
        }
        break;
    }
    case TestType_Count:
    {
        while(true){
            if(!conMsgQueue.try_dequeue(msg)){
                break;
            }
            pTestMgr->receiveCount(msg);
        }
        break;
    }
    case TestType_CountToken:
    {
        moodycamel::ConsumerToken tokenpQ(conMsgQueue);
        while(true){
            if(!conMsgQueue.try_dequeue(tokenpQ, msg)){
                break;
            }
            pTestMgr->receiveCount(msg);
        }
        break;
    }
    case TestType_Speed:
    {
        while(true){
            if(!conMsgQueue.try_dequeue(msg)){
                break;
            }
        }
        break;
    }
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
//lock queue
basiclib::CMessageQueueLock<ctx_message> msgQueue;
THREAD_RETURN CMessageQueueThreadPush(void* arg){
    ctx_message msg;
    CBasicQueryArrayTest* pTest = (CBasicQueryArrayTest*)arg;
    while(true){
        ctx_message* pRet = pTest->GetPushCtx(msg);
        if(pRet == nullptr)
            break;
        msgQueue.MQPush(&msg);
    }
    return 0;
}
THREAD_RETURN CMessageQueueThreadPop(void* arg){
    ctx_message msg;
    CBasicQueryArrayTestMgr* pTestMgr = (CBasicQueryArrayTestMgr*)arg;
    switch(pTestMgr->m_testType){
    case TestType_PushComplate:
    {
        while(true){
            if(msgQueue.MQPop(&msg)){
                break;
            }
            pTestMgr->receiveIndex(msg);
        }
        break;
    }
    case TestType_Count:
    {
        while(true){
            if(msgQueue.MQPop(&msg)){
                break;
            }
            pTestMgr->receiveCount(msg);
        }
        break;
    }
    case TestType_Speed:
    {
        while(true){
            if(msgQueue.MQPop(&msg)){
                break;
            }
        }
        break;
    }
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

void createTimeCheck(){
    const int nThreadCount = 4;
    DWORD dwThreadServerID = 0;
    HANDLE thread[8];
    vector<CBasicQueryArrayTest*> vtArrayList;
    /*for(int j = 0; j < nThreadCount; j++){
        CBasicQueryArrayTest* pArray = new CBasicQueryArrayTest();
        pArray->setIndex(j);
        pArray->m_pushMaxTimes = TIMES_FAST;
        vtArrayList.push_back(pArray);
        thread[j] = basiclib::BasicCreateThread(lpStartAddressPush, pArray, &dwThreadServerID);
    }
    
    for(int j = nThreadCount; j < 2 * nThreadCount; j++){
        thread[j] = basiclib::BasicCreateThread(lpStartAddressPop, pMgr, &dwThreadServerID);
    }*/
}

bool createSingleCheck(const int nThreadCount, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPush, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPop, TestType testType){
    bool bRet = true;
    DWORD dwThreadServerID = 0;
    char szBuf[2][32] = { 0 };
    sprintf(szBuf[0], "ThreadCount(%d) Push", nThreadCount);
    CreateCalcUseTime(begin, PrintUseTime(szBuf[0], TIMES_FAST / nThreadCount * 5 * nThreadCount), false);
    sprintf(szBuf[1], "ThreadCount(%d) Pop", nThreadCount);
    CreateCalcUseTime(beginrece, PrintUseTime(szBuf[1], TIMES_FAST / nThreadCount * 5 * nThreadCount), false);
    CBasicQueryArrayTestMgrMulti* pDelete = new CBasicQueryArrayTestMgrMulti(5, nThreadCount, testType, TIMES_FAST / nThreadCount, [&](CBasicQueryArrayTestMgrMulti* pSelf, CBasicQueryArrayTestMgr* pMgr, int nReceiveThreadCount){
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
    delete pDelete;
    return bRet;
}

//test func
bool SpeedTest(basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPush, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPop, TestType testType, int nMaxThreadCount = 8){
    bool bRet = true;
    bRet &= createSingleCheck(1, lpStartAddressPush, lpStartAddressPop, testType);
    
    if(nMaxThreadCount >= 2){
        bRet &= createSingleCheck(2, lpStartAddressPush, lpStartAddressPop, testType);
    }
    if(nMaxThreadCount >= 4){
        bRet &= createSingleCheck(4, lpStartAddressPush, lpStartAddressPop, testType);
    }
    if(nMaxThreadCount >= 8){
        bRet &= createSingleCheck(8, lpStartAddressPush, lpStartAddressPop, testType);
    }
    return bRet;
}

bool TestContainExt(){
    bool bRet = true;
    DWORD dwThreadServerID = 0;
    //correct check，speed
    //printf("/*************************************************************************/\n");
    //printf("Test TestType_PushComplate BasicQueue\n");
    //bRet &= SpeedTest(CBasicQueueThreadPush, CBasicQueueThreadPop, TestType_PushComplate);
    //printf("Test TestType_PushComplate ConcurrentQueue\n");
    //bRet &= SpeedTest(ConcurrentQueueThreadPush, ConcurrentQueueThreadPop, TestType_PushComplate);
    //printf("Test TestType_PushComplate CMessageQueue\n");
    //bRet &= SpeedTest(CMessageQueueThreadPush, CMessageQueueThreadPop, TestType_PushComplate, 4);
    //printf("/*************************************************************************/\n");
    printf("Test TestType_Count BasicQueue\n");
    bRet &= SpeedTest(CBasicQueueThreadPush, CBasicQueueThreadPop, TestType_Count);
    //printf("Test TestType_Count ConcurrentQueue\n");
    //bRet &= SpeedTest(ConcurrentQueueThreadPush, ConcurrentQueueThreadPop, TestType_Count);
    //printf("Test TestType_Count CMessageQueue\n");
    //bRet &= SpeedTest(CMessageQueueThreadPush, CMessageQueueThreadPop, TestType_Count, 4);
    //printf("/*************************************************************************/\n");
    //printf("Test TestType_CountToken ConcurrentQueue\n");
    //bRet &= SpeedTest(ConcurrentQueueThreadPush, ConcurrentQueueThreadPop, TestType_CountToken);
    //printf("/*************************************************************************/\n");
    return bRet;
}


#endif
