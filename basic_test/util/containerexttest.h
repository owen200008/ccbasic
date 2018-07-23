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
/*
 LONG g_TotalPush = 0;
 LONG g_TotalPop = 0;
 
 const int g_CheckMaxValues = 100000000;
 DWORD g_ayCheckMap[g_CheckMaxValues] = { 0 };
 uint32_t g_ayCheckMapIndex[g_CheckMaxValues] = { 0 };
 CBasicQueueArray<uint32_t> basicQueueInt;
 THREAD_RETURN CBasicQueueThreadPush2(void* arg){
 clock_t begin = clock();
 int nIndex = 0;
 while (true){
 uint32_t lRet = basiclib::BasicInterlockedIncrement(&g_TotalPush);
 basicQueueInt.Push(lRet);
 if (lRet % (TIMES_FAST * 4) == 0){
 clock_t end = clock();
 printf("BasicQueuePush2(%d) %d empty(%d)\n", lRet, end - begin, basicQueueInt.IsEmpty());
 }
 if (g_TotalPush > g_TotalPop + TIMES_FAST * 4)
 basiclib::BasicSleep(1000);
 }
 return 0;
 }
 THREAD_RETURN CBasicQueueThreadPop2(void* arg){
 DWORD dwThreadID = basiclib::BasicGetCurrentThreadId();
 clock_t begin = clock();
 uint32_t nIndex = 0;
 uint32_t nGetIndex = 0;
 while (true){
 if (!basicQueueInt.Pop(nIndex)){
 basiclib::BasicSleep(1);
 continue;
 }
 if (nIndex == 0){
 return 0;
 }
 uint32_t lRet = basiclib::BasicInterlockedIncrement(&g_TotalPop);
 if (g_ayCheckMap[nIndex % g_CheckMaxValues])
 return 0;
 if (g_TotalPop != nIndex)
 basiclib::BasicSleep(1);
 
 g_ayCheckMap[nIndex % g_CheckMaxValues] = dwThreadID;
 g_ayCheckMapIndex[nIndex % g_CheckMaxValues] = nGetIndex;
 
 if (lRet % (TIMES_FAST * 4) == 0){
 clock_t end = clock();
 printf("BasicQueuePop2(%d) %d empty(%d)\n", lRet, end - begin, basicQueueInt.IsEmpty());
 }
 }
 return 0;
 }
 THREAD_RETURN CBasicQueueThreadCheck(void* arg){
 uint32_t nCheckList = 1;
 while (true){
 if (!g_ayCheckMap[nCheckList % g_CheckMaxValues]){
 basiclib::BasicSleep(500);
 continue;
 }
 
 g_ayCheckMap[nCheckList % g_CheckMaxValues] = 0;
 nCheckList++;
 if (nCheckList % (TIMES_FAST * 4) == 0){
 clock_t end = clock();
 printf("CheckNumber(%d)\n", nCheckList);
 }
 }
 return 0;
 }
 
 
 THREAD_RETURN ConcurrentQueueThreadPushToken(void* arg){
 moodycamel::ProducerToken token(conMsgQueue);
 clock_t begin = clock();
 int nIndex = 0;
 while (true){
 ctx_message* pRet = GetPushCtx();
 if (pRet == nullptr)
 break;
 conMsgQueue.enqueue(token, std::move(*pRet));
 nIndex++;
 }
 clock_t end = clock();
 g_addC += end - begin;
 printf("LockFreeMQPushToken(%d) %d\n", nIndex, end - begin);
 return 0;
 }
 THREAD_RETURN ConcurrentQueueThreadPop(void* arg){
 clock_t begin = clock();
 int nIndex = 0;
 while (true){
 //moodycamel::ConsumerToken tokenCon(conMsgQueue);
 if (!conMsgQueue.try_dequeue(msg)){
 break;
 }
 nIndex++;
 }
 clock_t end = clock();
 g_delC += end - begin;
 printf("LockFreeMQPop(%d) %d\n", nIndex, end - begin);
 return 0;
 }
 THREAD_RETURN ConcurrentQueueThreadPopToken(void* arg){
 moodycamel::ConsumerToken tokenCon(conMsgQueue);
 clock_t begin = clock();
 int nIndex = 0;
 while (true){
 if (!conMsgQueue.try_dequeue(tokenCon, msg)){
 break;
 }
 nIndex++;
 }
 clock_t end = clock();
 g_delC += end - begin;
 printf("LockFreeMQPopToken(%d) %d\n", nIndex, end - begin);
 return 0;
 }
 
 THREAD_RETURN TestSeq(void* arg){
 ctx_message* pMsg = (ctx_message*)arg;
 int nIndex = 0;
 for(int i = 0;i < 10;i++){
 {
 moodycamel::ProducerToken token(conMsgQueue);
 pMsg->m_session = i;
 if (!conMsgQueue.enqueue(token, *pMsg)){
 break;
 }
 }
 printf("TestSeq %d %d enqueue\n", pMsg->m_nCtxID, pMsg->m_session);
 basiclib::BasicSleep(1000);
 }
 return 0;
 }
 THREAD_RETURN TestSeq2(void* arg){
 ctx_message* pMsg = (ctx_message*)arg;
 int nIndex = 0;
 for (int i = 0; i < 10; i++){
 {
 moodycamel::ProducerToken token(conMsgQueue);
 pMsg->m_session = i * 2;
 if (!conMsgQueue.enqueue(*pMsg)){
 break;
 }
 printf("TestSeq %d %d enqueue\n", pMsg->m_nCtxID, pMsg->m_session);
 pMsg->m_session = i * 2 + 1;
 if (!conMsgQueue.enqueue(*pMsg)){
 break;
 }
 }
 printf("TestSeq %d %d enqueue\n", pMsg->m_nCtxID, pMsg->m_session);
 basiclib::BasicSleep(1000);
 }
 return 0;
 }
 */

#define CREATE_THREAD 8
HANDLE g_thread[CREATE_THREAD];

bool createSingleCheck(const int nThreadCount, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPush, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPop, TestType testType){
    bool bRet = true;
    DWORD dwThreadServerID = 0;
    char szBuf[2][32] = { 0 };
    sprintf(szBuf[0], "ThreadCount(%d) Push", nThreadCount);
    CreateCalcUseTime(begin, PrintUseTime(szBuf[0], TIMES_FAST / nThreadCount * 5 * nThreadCount), false);
    sprintf(szBuf[1], "ThreadCount(%d) Pop", nThreadCount);
    CreateCalcUseTime(beginrece, PrintUseTime(szBuf[1], TIMES_FAST / nThreadCount * 5 * nThreadCount), false);
    CBasicQueryArrayTestMgrMulti* pDelete = new CBasicQueryArrayTestMgrMulti(5, nThreadCount, testType, TIMES_FAST / nThreadCount, [&](CBasicQueryArrayTestMgrMulti* pSelf, CBasicQueryArrayTestMgr* pMgr, int nReceiveThreadCount){
        StartCalcUseTime(begin);
        for(int j = 0; j < nThreadCount; j++){
            g_thread[j] = basiclib::BasicCreateThread(lpStartAddressPush, &pMgr->p[j], &dwThreadServerID);
        }
        for(int j = 0; j < nThreadCount; j++){
            basiclib::BasicWaitThread(g_thread[j], -1);
        }
        EndCalcUseTimeCallback(begin, nullptr);
        StartCalcUseTime(beginrece);
        for(int j = 0; j < nReceiveThreadCount; j++){
            g_thread[j] = basiclib::BasicCreateThread(lpStartAddressPop, pMgr, &dwThreadServerID);
        }
        for(int j = 0; j < nReceiveThreadCount; j++){
            basiclib::BasicWaitThread(g_thread[j], -1);
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
    if (true){
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
    }
    /*
     g_addC = 0;
     g_delC = 0;
     if (true)
     {
     //if (CREATE_THREAD <= 4)
     {
     clock_t begin = clock();
     for (int i = 0; i < 5; i++)
     {
     g_pushtimes = 0;
     g_poptimes = 0;
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     g_thread[j] = basiclib::BasicCreateThread(CMessageQueueThreadPush, nullptr, &dwThreadServerID);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     basiclib::BasicWaitThread(g_thread[j], -1);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     g_thread[j] = basiclib::BasicCreateThread(CMessageQueueThreadPop, nullptr, &dwThreadServerID);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     basiclib::BasicWaitThread(g_thread[j], -1);
     }
     }
     clock_t end = clock();
     printf("Total MQPush %d\n", g_addC);
     printf("Total MQPop %d\n", g_delC);
     printf("Total Times %d\n", end - begin);
     }
     }
     */
    /*if (true)
     {
     g_addC = 0;
     g_delC = 0;
     clock_t begin = clock();
     for (int i = 0; i < 5; i++)
     {
     g_pushtimes = 0;
     g_poptimes = 0;
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     g_thread[j] = basiclib::BasicCreateThread(ConcurrentQueueThreadPushToken, nullptr, &dwThreadServerID);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     basiclib::BasicWaitThread(g_thread[j], -1);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     g_thread[j] = basiclib::BasicCreateThread(ConcurrentQueueThreadPopToken, nullptr, &dwThreadServerID);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     basiclib::BasicWaitThread(g_thread[j], -1);
     }
     }
     clock_t end = clock();
     printf("Total MQPush %d\n", g_addC);
     printf("Total MQPop %d\n", g_delC);
     printf("Total Times %d\n", end - begin);
     }
     {
     for (int j = 0; j < 5; j++)
     {
     clock_t begin = clock();
     for (int i = 0; i < TIMES_FAST; i++)
     conMsgQueue.try_dequeue(msg);
     clock_t end = clock();
     printf("dequeue Total Times %d\n", end - begin);
     }
     }*/
    /*if (true)
     {
     g_addC = 0;
     g_delC = 0;
     clock_t begin = clock();
     for (int i = 0; i < 5; i++)
     {
     g_pushtimes = 0;
     g_poptimes = 0;
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     g_thread[j] = basiclib::BasicCreateThread(ConcurrentQueueThreadPush, nullptr, &dwThreadServerID);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     basiclib::BasicWaitThread(g_thread[j], -1);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     g_thread[j] = basiclib::BasicCreateThread(ConcurrentQueueThreadPop, nullptr, &dwThreadServerID);
     }
     for (int j = 0; j < CREATE_THREAD; j++)
     {
     basiclib::BasicWaitThread(g_thread[j], -1);
     }
     }
     clock_t end = clock();
     printf("Total MQPush %d\n", g_addC);
     printf("Total MQPop %d\n", g_delC);
     printf("Total Times %d\n", end - begin);
     }*/
    /*{
     for (int j = 0; j < 5; j++)
     {
     clock_t begin = clock();
     for (int i = 0; i < TIMES_FAST; i++)
     conMsgQueue.try_dequeue(msg);
     clock_t end = clock();
     printf("dequeue Total Times %d\n", end - begin);
     }
     moodycamel::ConsumerToken tokenCon(conMsgQueue);
     for (int j = 0; j < 5; j++)
     {
     clock_t begin = clock();
     for (int i = 0; i < TIMES_FAST; i++)
     conMsgQueue.try_dequeue(tokenCon, msg);
     clock_t end = clock();
     printf("dequeue Total Times %d\n", end - begin);
     }
     }*/
    return bRet;
}


#endif
