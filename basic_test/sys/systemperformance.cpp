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

void SystemPerformace(){
    {
        StartCalcUseTime(begin, PrintUseTime("funccall+spinlocknosame", TIMES_FAST));
        for(int i = 0; i < TIMES_FAST; i++){
            TestCallFunc(i);
        }
    }
    {
        StartCalcUseTime(begin, PrintUseTime("funccall+spinlock", TIMES_FAST));
        for(int i = 0; i < TIMES_FAST; i++){
            TestCallSameFunc(i);
        }
    }
    {
        StartCalcUseTime(begin, PrintUseTime("mutex", TIMES_FAST/10));
        for(int i = 0; i < TIMES_FAST / 10; i++){
            TestMutexCallFunc(i);
        }
    }
    {
        StartCalcUseTime(begin, PrintUseTime("malloc", TIMES_FAST / 10));
        for(int i = 0; i < TIMES_FAST / 10; i++){
            pFreeCreate(pCreate());
        }
    }
}
