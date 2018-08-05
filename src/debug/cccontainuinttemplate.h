#ifndef CCCONTAINUNITTEMPLATE_H
#define CCCONTAINUNITTEMPLATE_H

#include <atomic>

template<class T, class Container>
class CCContainUnit{
public:
    virtual ~CCContainUnit(){
        if(m_pNode){
            delete[]m_pNode;
        }
        if(m_pBoolReceive){
            delete[]m_pBoolReceive;
        }
    }
    virtual void Init(Container* pContainer, uint32_t nIndex, uint32_t nCount){
        m_pContainer = pContainer;
        m_nIndex = nIndex;
        m_nCount = nCount;
        m_pNode = new T[nCount];
        m_pBoolReceive = new bool[nCount];
        for(uint32_t i = 0; i < nCount; i++){
            m_pNode[i].InitUint(nIndex, i);
            m_pBoolReceive[i] = false;
        }
    }
    virtual T* GetPushCtx(){
        uint32_t lRet = m_nPushTime.fetch_add(1, std::memory_order_relaxed);
        if(lRet >= m_nCount)
            return nullptr;
        return &m_pNode[lRet];
    }
    virtual void Receive(T* pNode){
        m_pBoolReceive[pNode->GetCheckReceiveNumber()] = true;
    }
    virtual bool CheckIsSuccess(){
        for(uint32_t i = 0; i < m_nCount; i++){
            if(!m_pBoolReceive[i])
                return false;
        }
        return true;
    }
    Container* GetContainer(){
        return m_pContainer;
    }
protected:
    uint32_t                m_nIndex = 0;
    uint32_t                m_nCount = 0;
    std::atomic<uint32_t>   m_nPushTime = 0;
    T*                      m_pNode = nullptr;
    bool*                   m_pBoolReceive = nullptr;
    Container*              m_pContainer = nullptr;
};

template<class T, class Container>
class CCContainUnitThread{
public:
    virtual ~CCContainUnitThread(){
        if(m_pUint)
            delete[]m_pUint;
    }
    virtual void Init(Container* pContainer, uint32_t nThreadCount, uint32_t nCount){
        m_pContainer = pContainer;
        m_nThreadCount = nThreadCount;
        m_pUint = createCCContainUnit(nThreadCount);
        for(uint32_t i = 0; i < nThreadCount; i++){
            m_pUint[i].Init(pContainer, i, nCount);
        }
    }
    virtual void Receive(T* pNode){
        m_pUint[pNode->GetCheckIndex()].Receive(pNode);
    }
    virtual bool CheckIsSuccess(){
        for(uint32_t i = 0; i < m_nThreadCount; i++){
            if(!m_pUint[i].CheckIsSuccess())
                return false;
        }
        return true;
    }
    Container* GetContainer(){
        return m_pContainer;
    }
    CCContainUnit<T, Container>* GetContainUintByIndex(uint32_t nIndex){
        return &m_pUint[nIndex];
    }
protected:
    virtual CCContainUnit<T, Container>* createCCContainUnit(uint32_t nCount) = 0;
protected:
    uint32_t                        m_nThreadCount = 0;
    CCContainUnit<T, Container>*    m_pUint = nullptr;
    Container*                      m_pContainer = nullptr;
};

template<class T, class Container>
THREAD_RETURN PushFunc(void* p){
    CCContainUnit<T, Container>* pTest = (CCContainUnit<T, Container>*)p;
    Container* pContainer = pTest->GetContainer();
    while(true){
        T* pRet = pTest->GetPushCtx();
        if(pRet == nullptr)
            break;
        pContainer->Push(pRet);
    }
    return 0;
}

template<class T, class Container>
THREAD_RETURN PopFunc(void* p){
    CCContainUnitThread<T, Container>* pTest = (CCContainUnitThread<T, Container>*)p;
    Container* pContainer = pTest->GetContainer();
    while(true){
        T* p = (T*)pContainer->Pop();
        if(p == nullptr)
            break;
        pTest->Receive(p);
    }
    return 0;
}

template<class T, class Container>
THREAD_RETURN PushContentFunc(void* p){
    CCContainUnit<T, Container>* pTest = (CCContainUnit<T, Container>*)p;
    Container* pContainer = pTest->GetContainer();
    while(true){
        T* pRet = pTest->GetPushCtx();
        if(pRet == nullptr)
            break;
        pContainer->Push(*pRet);
    }
    return 0;
}

template<class T, class Container>
THREAD_RETURN PopContentFunc(void* p){
    CCContainUnitThread<T, Container>* pTest = (CCContainUnitThread<T, Container>*)p;
    Container* pContainer = pTest->GetContainer();
    T node;
    while(true){
        if(!pContainer->Pop(node))
            break;
        pTest->Receive(&node);
    }
    return 0;
}

//use time
#define PrintUseTime(calcName, totalPerformance)\
[&](DWORD dwUseTime, DWORD dwTotalUseTime){\
    printf("P:%12.3f/ms Time:%5d/%5d F:%s\n", (double)(totalPerformance)/dwTotalUseTime, dwUseTime, dwTotalUseTime, calcName);\
}

template<class T, class Container>
class CContainUnitThreadRunMode{
public:
    CContainUnitThreadRunMode(Container* p, uint32_t nMaxCountTimeFast, uint32_t nRepeatTimes){
        m_nMaxCountTimesFast = nMaxCountTimeFast;
        m_nRepeatTimes = nRepeatTimes;
        m_pContain = p;
    }

    template<class F>
    bool RepeatCCContainUnitThread(uint32_t nThreadCount, uint32_t maxPushTimes, F func){
        uint32_t nReceiveThreadCount = nThreadCount;
        for(uint32_t i = 0; i < m_nRepeatTimes; i++){
            auto p = createUnitThread();
            p->Init(m_pContain, nThreadCount, maxPushTimes);
            if(!func(p, nReceiveThreadCount)){
                delete p;
                return false;
            }
            delete p;
        }
        return true;
    }

    bool PowerOfTwoThreadCountImpl(uint32_t nThreadCount, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPush, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPop){
        bool bRet = true;
        DWORD dwThreadServerID = 0;
        char szBuf[2][32];
        sprintf_s(szBuf[0], 32, "ThreadCount(%d) Push", nThreadCount);
        CreateCalcUseTime(begin, PrintUseTime(szBuf[0], m_nMaxCountTimesFast / nThreadCount * m_nRepeatTimes * nThreadCount), false);
        sprintf_s(szBuf[1], 32, "ThreadCount(%d) Pop", nThreadCount);
        CreateCalcUseTime(beginrece, PrintUseTime(szBuf[1], m_nMaxCountTimesFast / nThreadCount * m_nRepeatTimes * nThreadCount), false);
        RepeatCCContainUnitThread(nThreadCount, m_nMaxCountTimesFast / nThreadCount, [&](CCContainUnitThread<T, Container>* pMgr, uint32_t nReceiveThreadCount){
            bool bFuncRet = true;
            HANDLE* pThread = new HANDLE[nThreadCount];
            StartCalcUseTime(begin);
            for(uint32_t j = 0; j < nThreadCount; j++){
                pThread[j] = basiclib::BasicCreateThread(lpStartAddressPush, pMgr->GetContainUintByIndex(j), &dwThreadServerID);
            }
            for(uint32_t j = 0; j < nThreadCount; j++){
                basiclib::BasicWaitThread(pThread[j], -1);
            }
            EndCalcUseTimeCallback(begin, nullptr);
            StartCalcUseTime(beginrece);
            for(uint32_t j = 0; j < nReceiveThreadCount; j++){
                pThread[j] = basiclib::BasicCreateThread(lpStartAddressPop, pMgr, &dwThreadServerID);
            }
            for(uint32_t j = 0; j < nReceiveThreadCount; j++){
                basiclib::BasicWaitThread(pThread[j], -1);
            }
            EndCalcUseTimeCallback(beginrece, nullptr);
            if(!pMgr->CheckIsSuccess()){
                bFuncRet = false;
                printf("check fail\n");
            }
            delete[]pThread;
            return bFuncRet;
        });
        CallbackUseTime(begin);
        CallbackUseTime(beginrece);
        return bRet;
    }

    bool PowerOfTwoThreadCountTest(basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPush, basiclib::LPBASIC_THREAD_START_ROUTINE lpStartAddressPop, uint32_t nMaxThreadCount = 8){
        for(uint32_t nThreadCount = 1; nThreadCount <= nMaxThreadCount; nThreadCount *= 2){
            if(!PowerOfTwoThreadCountImpl(nThreadCount, lpStartAddressPush, lpStartAddressPop))
                return false;
        }
        return true;
    }

protected:
    virtual CCContainUnitThread<T, Container>* createUnitThread() = 0;
protected:
    Container*          m_pContain;
    uint32_t            m_nRepeatTimes;
    uint32_t            m_nMaxCountTimesFast;
};

#endif