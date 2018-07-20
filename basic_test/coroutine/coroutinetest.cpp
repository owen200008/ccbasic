#include "coroutinetest.h"
#include "../headdefine.h"
#define TIMES_FORTEST 1000000
/*
void Foo(CCorutinePlus* pCorutine)
{
	char* pData = pCorutine->GetResumeParamPoint<char>(1);
	int n = pCorutine->GetResumeParam<int>(2);
	char szBuf[16384];
	memcpy(szBuf, pData, n);
	if (n != 16384 || szBuf[0] != '\0')
        basiclib::BasicLogEvent(basiclib::DebugLevel_Info, "1");
	pCorutine->YieldCorutine();
}
*/


//统计迭代次数
uint32_t g_nTotalCorutineCount = 0;
uint32_t g_nTotalCorutineCreate = 0;
void DieDaiTimes(CCorutinePlusBase* pCorutine){
    g_nTotalCorutineCount++;
    int nTimes = pCorutine->GetParam<int>(0) + 1;
    int nCreateTimes = pCorutine->GetParam<int>(1);
    int nLimitTimes = pCorutine->GetParam<int>(2);
    CCorutinePlusPoolBase* pRunPool = pCorutine->GetRunPool();
    if(nTimes > nLimitTimes){
        return;
    }
    g_nTotalCorutineCreate++;
    for(int i = 0; i < nCreateTimes; i++){
        CCorutinePlusBase* pPlus = pRunPool->GetCorutine();
        pPlus->Create(DieDaiTimes);
        pPlus->Resume(pRunPool, &nTimes, &nCreateTimes, &nLimitTimes);
        if(pPlus->GetCoroutineState() != CoroutineState_Death){
            printf("DieDaiTimes check fail\n");
        }
    }
}

template<class... _Types>
int TestCreateFunc(void(*pCallbackFunc)(_Types...), _Types&&... _Args){
	pCallbackFunc(std::forward<_Types>(_Args)...);
	return 0;
}

class CCoroutineTestMgr{
public:
    void Init(LONG maxCount){
        m_maxPushCount = maxCount;
    }
    uint32_t getPushTimes(){
        return m_maxPushCount;
    }
    LONG m_maxPushCount;
};

class CCoroutineTestMgrMulti{
public:
    template<class F>
    CCoroutineTestMgrMulti(int nRepeatTimes, LONG maxPushTimes, F func){
        m_nRepeatTimes = nRepeatTimes;
       
        m_maxPushCount = maxPushTimes;
        for(int i = 0; i < nRepeatTimes; i++){
            func(this, i);
        }
    }
    ~CCoroutineTestMgrMulti(){
    }
    //print info
    void printData(){
        uint32_t nTotal = m_maxPushCount * m_nRepeatTimes;
        printf("push usetime(%d) push %.4f/ms\n", m_use,
            (double)nTotal / m_use);
    }
    //print info
    void printGlobalData(){
        uint32_t nTotal = g_nTotalCorutineCount;
        printf("push usetime(%d) push %.4f/ms create %d\n", m_use,
            (double)nTotal / m_use, g_nTotalCorutineCreate);
    }
    int m_nRepeatTimes = 0;
    clock_t m_begin = 0;
    clock_t m_use = 0;
    LONG m_maxPushCount;
    void StartClock(){
        m_begin = clock();
    }
    void EndClock(){
        m_use += clock() - m_begin;
    }
};


void StartCoroutineTest(){
    CCorutinePlusPoolBase* S = new CCorutinePlusPoolBase();
    S->InitCorutine();

    //single corutine test
    {
        CCoroutineTestMgrMulti* pDelete = new CCoroutineTestMgrMulti(5, TIMES_FAST, [&](CCoroutineTestMgrMulti* pSelf, int nIndex){
            pSelf->StartClock();
            CCorutinePlusBase* pCorutine = S->GetCorutine();
            pCorutine->Create([](CCorutinePlusBase* pCorutine)->void{
                for(int i = 0; i <= TIMES_FAST; i++){
                    pCorutine->YieldCorutine();
                }
            });
            pCorutine->Resume(S);
            for(int i = 0; i <= TIMES_FAST; i++){
                pCorutine->Resume(S);
            }
            pSelf->EndClock();
           
            if(pCorutine->GetCoroutineState() != CoroutineState_Death){
                printf("check fail\n");
            }
        });
        pDelete->printData();
        printf("Check %d tiems %d/%d StackSize:%d/%d\r\n", TIMES_FAST, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        delete pDelete;
    }
    //create release test
    {
        CCoroutineTestMgrMulti* pDelete = new CCoroutineTestMgrMulti(TIMES_FAST, 1, [&](CCoroutineTestMgrMulti* pSelf, int nIndex){
            pSelf->StartClock();
            CCorutinePlusBase* pCorutine = S->GetCorutine();
            pCorutine->Create([](CCorutinePlusBase* pCorutine)->void{
            
            });
            pCorutine->Resume(S);
            pSelf->EndClock();
            if(pCorutine->GetCoroutineState() != CoroutineState_Death){
                printf("check fail\n");
            }
        });
        pDelete->printData();
        printf("Check %d tiems %d/%d StackSize:%d/%d\r\n", TIMES_FAST, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        delete pDelete;
    }
    {
        g_nTotalCorutineCount = 0;
        g_nTotalCorutineCreate = 0;
        CCoroutineTestMgrMulti* pDelete = new CCoroutineTestMgrMulti(5, 1, [&](CCoroutineTestMgrMulti* pSelf, int nIndex){
            int nTime = 0;
            int nCreateTimes = 5;
            int nLimitTimes = 10;
            pSelf->StartClock();
            CCorutinePlusBase* pCorutine = S->GetCorutine();
            pCorutine->Create(DieDaiTimes);
            pCorutine->Resume(S, &nTime, &nCreateTimes, &nLimitTimes);
            pSelf->EndClock();

            if(pCorutine->GetCoroutineState() != CoroutineState_Death){
                printf("check fail\n");
            }

        });
        pDelete->printGlobalData();
        printf("Check %d tiems %d/%d StackSize:%d/%d\r\n", TIMES_FAST, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        delete pDelete;
    }
    delete S;
}


