#include <basic.h>
#include "cclockfreequeuetest.h"
#include "cbasicqueuearray.h"
#include "../headdefine.h"

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

    void InitUint(uint32_t nIndex, uint32_t nValue){
        m_nCtxID = nValue;
        m_nIndex = nIndex;
    }
    uint32_t GetCheckIndex(){
        return m_nIndex;
    }
    uint32_t GetCheckReceiveNumber(){
        return m_nCtxID;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CCLockfreeFixQueuePushPop : public basiclib::CCLockfreeFixQueue<ctx_message, TIMES_FAST>{
public:
    CCLockfreeFixQueuePushPop(){

    }
    virtual ~CCLockfreeFixQueuePushPop(){

    }
};
CCLockfreeFixQueuePushPop lockfreeFixQueue;
typedef CCContainUnit<ctx_message, CCLockfreeFixQueuePushPop> CCContainUnitCCLockfreeFixQueue;
class CCContainUnitThreadCCLockfreeFixQueue : public CCContainUnitThread<ctx_message, CCLockfreeFixQueuePushPop>{
public:
    // 通过 CCContainUnitThread 继承
    virtual CCContainUnitCCLockfreeFixQueue* createCCContainUnit(uint32_t nCount) override{
        return new CCContainUnitCCLockfreeFixQueue[nCount];
    }
};

class CContainUnitThreadRunModeCCLockfreeFixQueue : public CContainUnitThreadRunMode<ctx_message, CCLockfreeFixQueuePushPop>{
public:
    CContainUnitThreadRunModeCCLockfreeFixQueue(CCLockfreeFixQueuePushPop* p, uint32_t nMaxCountTimeFast, uint32_t nRepeatTimes = 5) : CContainUnitThreadRunMode<ctx_message, CCLockfreeFixQueuePushPop>(p, nMaxCountTimeFast, nRepeatTimes){
    }
    // 通过 CContainUnitThreadRunMode 继承
    virtual CCContainUnitThreadCCLockfreeFixQueue* createUnitThread() override{
        return new CCContainUnitThreadCCLockfreeFixQueue();
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*CBasicQueueArray<ctx_message> basicQueue;
typedef CCContainUnit<ctx_message, CBasicQueueArray<ctx_message>> CCContainUnitCBasicQueueArray;
class CCContainUnitThreadCBasicQueueArray : public CCContainUnitThread<ctx_message, CBasicQueueArray<ctx_message>>{
public:
    // 通过 CCContainUnitThread 继承
    virtual CCContainUnitCBasicQueueArray* createCCContainUnit(uint32_t nCount) override{
        return new CCContainUnitCBasicQueueArray[nCount];
    }
};

class CContainUnitThreadRunModeCBasicQueueArray : public CContainUnitThreadRunMode<ctx_message, CBasicQueueArray<ctx_message>>{
public:
    CContainUnitThreadRunModeCBasicQueueArray(CBasicQueueArray<ctx_message>* p, uint32_t nMaxCountTimeFast, uint32_t nRepeatTimes = 5) : CContainUnitThreadRunMode<ctx_message, CBasicQueueArray<ctx_message>>(p, nMaxCountTimeFast, nRepeatTimes){
    }
    // 通过 CContainUnitThreadRunMode 继承
    virtual CCContainUnitThreadCBasicQueueArray* createUnitThread() override{
        return new CCContainUnitThreadCBasicQueueArray();
    }
};*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLockFreeMessageQueuePushPop : public basiclib::CLockFreeMessageQueue<ctx_message>{
public:
    bool Push(ctx_message& msg){
        return enqueue(msg);
    }
    bool Pop(ctx_message& msg){
        return try_dequeue(msg);
    }
};
CLockFreeMessageQueuePushPop conMsgQueue;
typedef CCContainUnit<ctx_message, CLockFreeMessageQueuePushPop> CCContainUnitCLockFreeMessageQueuePushPop;
class CCContainUnitThreadCLockFreeMessageQueuePushPop : public CCContainUnitThread<ctx_message, CLockFreeMessageQueuePushPop>{
public:
    // 通过 CCContainUnitThread 继承
    virtual CCContainUnitCLockFreeMessageQueuePushPop* createCCContainUnit(uint32_t nCount) override{
        return new CCContainUnitCLockFreeMessageQueuePushPop[nCount];
    }
};
class CContainUnitThreadRunModeCLockFreeMessageQueuePushPop : public CContainUnitThreadRunMode<ctx_message, CLockFreeMessageQueuePushPop>{
public:
    CContainUnitThreadRunModeCLockFreeMessageQueuePushPop(CLockFreeMessageQueuePushPop* p, uint32_t nMaxCountTimeFast, uint32_t nRepeatTimes) : CContainUnitThreadRunMode<ctx_message, CLockFreeMessageQueuePushPop>(p, nMaxCountTimeFast, nRepeatTimes){
    }
    // 通过 CContainUnitThreadRunMode 继承
    virtual CCContainUnitThreadCLockFreeMessageQueuePushPop* createUnitThread() override{
        return new CCContainUnitThreadCLockFreeMessageQueuePushPop();
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
basiclib::CMessageQueueLock<ctx_message> msgQueue;
typedef CCContainUnit<ctx_message, basiclib::CMessageQueueLock<ctx_message>> CCContainUnitCMessageQueueLock;
class CCContainUnitThreadCMessageQueueLock : public CCContainUnitThread<ctx_message, basiclib::CMessageQueueLock<ctx_message>>{
public:
    // 通过 CCContainUnitThread 继承
    virtual CCContainUnitCMessageQueueLock* createCCContainUnit(uint32_t nCount) override{
        return new CCContainUnitCMessageQueueLock[nCount];
    }
};
class CContainUnitThreadRunModeCMessageQueueLock : public CContainUnitThreadRunMode<ctx_message, basiclib::CMessageQueueLock<ctx_message>>{
public:
    CContainUnitThreadRunModeCMessageQueueLock(basiclib::CMessageQueueLock<ctx_message> * p, uint32_t nMaxCountTimeFast, uint32_t nRepeatTimes) : CContainUnitThreadRunMode<ctx_message, basiclib::CMessageQueueLock<ctx_message>>(p, nMaxCountTimeFast, nRepeatTimes){
    }
    // 通过 CContainUnitThreadRunMode 继承
    virtual CCContainUnitThreadCMessageQueueLock* createUnitThread() override{
        return new CCContainUnitThreadCMessageQueueLock();
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cclockfreequeuetest(){
    bool bRet = true;
    CContainUnitThreadRunModeCCLockfreeFixQueue* pFixQueue = new CContainUnitThreadRunModeCCLockfreeFixQueue(&lockfreeFixQueue, TIMES_FAST, 5);
    pFixQueue->PowerOfTwoThreadCountTest(PushContentFunc<ctx_message, CCLockfreeFixQueuePushPop>, PopContentFunc<ctx_message, CCLockfreeFixQueuePushPop>);

    //CContainUnitThreadRunModeCBasicQueueArray* pCBasicQueueArrayMode = new CContainUnitThreadRunModeCBasicQueueArray(&basicQueue, 5);
    //pCBasicQueueArrayMode->PowerOfTwoThreadCountTest(PushContentFunc<ctx_message, CBasicQueueArray<ctx_message>>, PopContentFunc<ctx_message, CBasicQueueArray<ctx_message>>);


    //correct check，speed
    //printf("/*************************************************************************/\n");
    //printf("Test TestType_PushComplate BasicQueue\n");
    //bRet &= SpeedTest(CBasicQueueThreadPush, CBasicQueueThreadPop, TestType_PushComplate);
    //printf("Test TestType_PushComplate ConcurrentQueue\n");
    //bRet &= SpeedTest(ConcurrentQueueThreadPush, ConcurrentQueueThreadPop, TestType_PushComplate);
    //printf("Test TestType_PushComplate CMessageQueue\n");
    //bRet &= SpeedTest(CMessageQueueThreadPush, CMessageQueueThreadPop, TestType_PushComplate, 4);
    //printf("/*************************************************************************/\n");
    //printf("Test TestType_Count BasicQueue\n");
    //bRet &= SpeedTest(&basicQueue, PushContentFunc<ctx_message, CBasicQueueArray<ctx_message>>, PopContentFunc<ctx_message, CBasicQueueArray<ctx_message>>);
    //printf("Test TestType_Count ConcurrentQueue\n");
    //bRet &= SpeedTest(ConcurrentQueueThreadPush, ConcurrentQueueThreadPop, TestType_Count);
    //printf("Test TestType_Count CMessageQueue\n");
    //bRet &= SpeedTest(CMessageQueueThreadPush, CMessageQueueThreadPop, TestType_Count, 4);
    //printf("/*************************************************************************/\n");
    //printf("Test TestType_CountToken ConcurrentQueue\n");
    //bRet &= SpeedTest(ConcurrentQueueThreadPush, ConcurrentQueueThreadPop, TestType_CountToken);
    //printf("/*************************************************************************/\n");
    //delete pCBasicQueueArrayMode;
    delete pFixQueue;
    return bRet;
}