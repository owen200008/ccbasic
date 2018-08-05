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

    void InitUint(uint32_t nIndex, uint32_t nValue){
        m_nValue = nValue;
        m_nIndex = nIndex;
    }
    uint32_t GetCheckIndex(){
        return m_nIndex;
    }
    uint32_t GetCheckReceiveNumber(){
        return m_nValue;
    }
    uint32_t m_nValue = 0;
    uint32_t m_nIndex = 0;
};

basiclib::CCLockfreeStack* pStacklockfree = nullptr;
typedef CCContainUnit<CCLockfreeValueNode, basiclib::CCLockfreeStack> CCContainUnitLockfreeStack;
class CCContainUnitThreadLockfreeStack : public CCContainUnitThread<CCLockfreeValueNode, basiclib::CCLockfreeStack>{
public:
    // 通过 CCContainUnitThread 继承
    virtual CCContainUnitLockfreeStack* createCCContainUnit(uint32_t nCount) override{
        return new CCContainUnitLockfreeStack[nCount];
    }
};

class CContainUnitThreadRunModeLockfreeStack : public CContainUnitThreadRunMode<CCLockfreeValueNode, basiclib::CCLockfreeStack>{
public:
    CContainUnitThreadRunModeLockfreeStack(basiclib::CCLockfreeStack* p, uint32_t nMaxCountTimeFast, uint32_t nRepeatTimes) : CContainUnitThreadRunMode<CCLockfreeValueNode, basiclib::CCLockfreeStack>(p, nMaxCountTimeFast, nRepeatTimes){
    }
    // 通过 CContainUnitThreadRunMode 继承
    virtual CCContainUnitThreadLockfreeStack* createUnitThread() override{
        return new CCContainUnitThreadLockfreeStack();
    }
};

bool CCLockfreeStackTest(){
    bool bRet = true;
    pStacklockfree = new basiclib::CCLockfreeStack();
    CContainUnitThreadRunModeLockfreeStack* pMode = new CContainUnitThreadRunModeLockfreeStack(pStacklockfree, TIMES_FAST, 5);
    pMode->PowerOfTwoThreadCountTest(PushFunc<CCLockfreeValueNode, basiclib::CCLockfreeStack>, PopFunc<CCLockfreeValueNode, basiclib::CCLockfreeStack>, 8);

    delete pMode;
    delete pStacklockfree;
    return bRet;
}
