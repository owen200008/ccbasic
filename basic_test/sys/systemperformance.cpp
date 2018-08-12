#include <basic.h>
#include "../headdefine.h"
#include "systemperformance.h"
//computer func call per ms
basiclib::SpinLock mutexSpinLock;
void TestCallFunc(int i){
    basiclib::CSpinLockFuncNoSameThreadSafe lock(&mutexSpinLock);
    lock.Lock();
    lock.UnLock();
}
void TestCallSameFunc(int i){
    basiclib::CSpinLockFunc lock(&mutexSpinLock);
    lock.Lock();
    lock.UnLock();
}

basiclib::CBasicObject* pCreate(){
    return new basiclib::CBasicObject();
}
void pFreeCreate(basiclib::CBasicObject* p){
    delete p;
}

basiclib::CMutex mutexData;
void TestMutexCallFunc(int i){
    basiclib::CSingleLock lock(&mutexData);
    lock.Lock();
}

THREAD_RETURN FetchAdd(void* p) {
    std::atomic<uint32_t>* pA = (std::atomic<uint32_t>*)p;
    for (int i = 0; i < TIMES_FAST;i++) {
        pA->fetch_add(1,std::memory_order_relaxed);
    }
    return 0;
}

void SystemPerformace(){
    /*{
        DWORD dwThreadID = 0;
        HANDLE* pThread = new HANDLE[10];
        std::atomic<uint32_t> a(0);
        char szBuf[32];
        for (int i = 0; i < 10; i++) {
            sprintf(szBuf, "multithread+fetchadd(%d)", 10 - i);
            CreateCalcUseTime(begin, PrintUseTime(szBuf, TIMES_FAST * (10 - i)), true);
            for (int j = i; j < 10; j++) {
                pThread[j] = basiclib::BasicCreateThread(FetchAdd, &a, &dwThreadID);
            }
            for (int j = i; j < 10; j++) {
                basiclib::BasicWaitThread(pThread[j], -1);
            }
        }
    }
    {
        DWORD dwThreadID = 0;
        HANDLE* pThread = new HANDLE[10];
        char szBuf[32];
        std::atomic<uint32_t> a[10];
        for (int i = 0; i < 10; i++) {
            sprintf(szBuf, "multithread+fetchadd(%d)", 10 - i);
            CreateCalcUseTime(begin, PrintUseTime(szBuf, TIMES_FAST * (10 - i)), true);
            for (int j = i; j < 10; j++) {
                pThread[j] = basiclib::BasicCreateThread(FetchAdd, &a[i], &dwThreadID);
            }
            for (int j = i; j < 10; j++) {
                basiclib::BasicWaitThread(pThread[j], -1);
            }
        }
    }
    {
        CreateCalcUseTime(begin, PrintUseTime("funccall+spinlocknosame", TIMES_FAST), true);
        for(int i = 0; i < TIMES_FAST; i++){
            TestCallFunc(i);
        }
    }*/
    {
        CreateCalcUseTime(begin, PrintUseTime("funccall+spinlock", TIMES_FAST), true);
        for(int i = 0; i < TIMES_FAST; i++){
            TestCallSameFunc(i);
        }
    }
    {
        CreateCalcUseTime(begin, PrintUseTime("mutex", TIMES_FAST/10), true);
        for(int i = 0; i < TIMES_FAST / 10; i++){
            TestMutexCallFunc(i);
        }
    }
    {
        CreateCalcUseTime(begin, PrintUseTime("malloc", TIMES_FAST / 10), true);
        for(int i = 0; i < TIMES_FAST / 10; i++){
            pFreeCreate(pCreate());
        }
    }
}
