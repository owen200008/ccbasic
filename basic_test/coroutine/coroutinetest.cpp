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
    template<class F>
    CCoroutineTestMgr(int nRepeatTimes, LONG maxPushTimes, F func){
        m_nRepeatTimes = nRepeatTimes;
       
        m_maxPushCount = maxPushTimes;
        for(int i = 0; i < nRepeatTimes; i++){
            func(this, i);
        }
    }
    ~CCoroutineTestMgr(){
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


class CCoroutineThreadMgr{
public:
    CCoroutineThreadMgr(){

    }
    void Init(LONG maxPushTimes){
        m_maxPushCount = maxPushTimes;
    }
    LONG m_maxPushCount;
    CCoroutineThreadMgr* m_pNext = nullptr;
    basiclib::SpinLock m_lock;
    vector<CCorutinePlusBase*> m_vtWaitCo;
    bool m_bFinish = false;
    void AddCorutine(CCorutinePlusBase* pCorutine){
        if(m_bFinish){
            m_pNext->AddCorutine(pCorutine);
            return;
        }
        basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lock, true);
        m_vtWaitCo.push_back(pCorutine);
    }
    void getVTWaitCo(vector<CCorutinePlusBase*>& vtWaitCo){
        basiclib::CSpinLockFuncNoSameThreadSafe lock(&m_lock, true);
        swap(m_vtWaitCo, vtWaitCo);
    }
    void SetFinish(){
        m_bFinish = true;
    }
};
class CCoroutineThreadMgrMulti{
public:
    template<class F>
    CCoroutineThreadMgrMulti(int nThreadCount, LONG maxPushTimes, F func){
        m_p = new CCoroutineThreadMgr[nThreadCount];
        m_nThreadCount = nThreadCount;
        for(int i = 0; i < nThreadCount; i++){
            m_p[i].Init(maxPushTimes);
        }
        for(int i = 0; i < nThreadCount; i++){
            if(i + 1 == nThreadCount){
                m_p[i].m_pNext = &m_p[0];
            }
            else{
                m_p[i].m_pNext = &m_p[i + 1];
            }
        }
        func(this, nThreadCount);
    }
    ~CCoroutineThreadMgrMulti(){
        if(m_p){
            delete[]m_p;
        }
    }
    int m_nThreadCount = 0;
    CCoroutineThreadMgr* m_p = nullptr;
    clock_t m_begin = 0;
    clock_t m_use = 0;

    void StartClock(){
        m_begin = clock();
    }
    void EndClock(){
        m_use += clock() - m_begin;
    }
    //print info
    void printData(){
        uint32_t nTotal = 0;
        for(int i = 0; i < m_nThreadCount; i++){
            nTotal += m_p[i].m_maxPushCount * TIMES_FAST / 10;
        }
        printf("push usetime(%d) push %.4f/ms\n", m_use,
            (double)nTotal / m_use);
    }
};


//
THREAD_RETURN CreateThreadCotoutine(void* arg){
    CCorutinePlusPoolBase* S = new CCorutinePlusPoolBase();
    S->InitCorutine();
    CCoroutineThreadMgr* pTest = (CCoroutineThreadMgr*)arg;
    for(int i = 0; i < pTest->m_maxPushCount; i++){
        CCorutinePlusBase* pCorutine = S->GetCorutine();
        pCorutine->Create([](CCorutinePlusBase* pCorutine)->void{
            for(int i = 0; i <= TIMES_FAST / 10; i++){
                pCorutine->YieldCorutine();
            }
        });
        pCorutine->Resume(S);
        //push到另外线程
        pTest->m_pNext->AddCorutine(pCorutine);
    }
    uint32_t nDeathCount = 0;
    while(true){
        vector<CCorutinePlusBase*> vtWaitCo;
        pTest->getVTWaitCo(vtWaitCo);
        if(vtWaitCo.size() != 0){
            for(CCorutinePlusBase* p : vtWaitCo){
                p->Resume(S);
                if(p->GetCoroutineState() == CoroutineState_Death){
                    nDeathCount++;
                }
                else{
                    //push到另外线程
                    pTest->m_pNext->AddCorutine(p);
                }
            }
            if(nDeathCount == pTest->m_maxPushCount){
                break;
            }
        }
        else{
            //basiclib::BasicSleep(1);
        }
    }
    return 0;
}

bool StartCoroutineTest(){
    CCorutinePlusPoolBase* S = new CCorutinePlusPoolBase();
    S->InitCorutine();

    //single corutine test
    printf("single corutine test\n");
    {
        CCoroutineTestMgr* pDelete = new CCoroutineTestMgr(5, TIMES_FAST, [&](CCoroutineTestMgr* pSelf, int nIndex){
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
    printf("create release test\n");
    {
        CCoroutineTestMgr* pDelete = new CCoroutineTestMgr(TIMES_FAST, 1, [&](CCoroutineTestMgr* pSelf, int nIndex){
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
    printf("deep iter create test\n");
    {
        g_nTotalCorutineCount = 0;
        g_nTotalCorutineCreate = 0;
        CCoroutineTestMgr* pDelete = new CCoroutineTestMgr(5, 1, [&](CCoroutineTestMgr* pSelf, int nIndex){
            int nTime = 0;
            int nCreateTimes = 3;
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
    //cross thread wake up
    printf("cross thread wake up test\n");
    {
        DWORD dwThreadServerID = 0;
#define CREATE_THREAD 8
        HANDLE g_thread[CREATE_THREAD];

        CCoroutineThreadMgrMulti* pDeleteTest = new CCoroutineThreadMgrMulti(3, 10, [&](CCoroutineThreadMgrMulti* pSelf, int nThreadCount){
            pSelf->StartClock();
            for(int j = 0; j < nThreadCount; j++){
                g_thread[j] = basiclib::BasicCreateThread(CreateThreadCotoutine, &pSelf->m_p[j], &dwThreadServerID);
            }
            for(int j = 0; j < nThreadCount; j++){
                basiclib::BasicWaitThread(g_thread[j], -1);
            }
            pSelf->EndClock();
        });
        pDeleteTest->printData();
        printf("Check %d tiems %d/%d StackSize:%d/%d\r\n", TIMES_FAST, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        delete pDeleteTest;
    }
    delete S;
    return true;
}


