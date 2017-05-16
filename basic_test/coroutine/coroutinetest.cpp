#include "coroutinetest.h"
#define TIMES_FORTEST 1000000
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

void DieDaiTimes(CCorutinePlus* pCorutine)
{
    CCorutinePlusPool* pRunPool = pCorutine->GetRunPool();
    int nTimes = pCorutine->GetResumeParam<int>(0) + 1;
    int nCreateTimes = pCorutine->GetResumeParam<int>(1);
    int nLimitTimes = pCorutine->GetResumeParam<int>(2);
    if (nTimes > nLimitTimes){
        return;
    }
    for (int i = 0; i < nCreateTimes; i++){
        CCorutinePlus* pPlus = pRunPool->GetCorutine();
        pPlus->ReInit(DieDaiTimes);
        pPlus->Resume(pRunPool, nTimes, nCreateTimes, nLimitTimes);
        if (nTimes == 1){
            printf("Check %d tiems %d/%d StackSize:%d/%d\r\n", i, pRunPool->GetVTCorutineSize(), pRunPool->GetCreateCorutineTimes(), pRunPool->GetVTShareStackCount(), pRunPool->GetCreateTimesShareStackCount());
        }
    }
}

void TestCoroutine(int nType){
	CCorutinePlusPool* S = new CCorutinePlusPool();
	S->InitCorutine();
	
    if (nType == 0){
        {
            CCorutinePlus* pCorutine = S->GetCorutine();
#define CHECKCORUTINETIMES 10000
            pCorutine->ReInit([](CCorutinePlus* pCorutine)->void{
					CCorutinePlusPool* pRunPool = pCorutine->GetRunPool();
                for (int i = 0; i <= CHECKCORUTINETIMES; i++){
                    if (i % (CHECKCORUTINETIMES / 10) == 0){
                        printf("Check YieldCorutine: %d\r\n", i);
                    }
                    pCorutine->YieldCorutine();
                }
            });
            printf("Start Check %d tiems %d/%d StackSize:%d/%d\r\n", CHECKCORUTINETIMES, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
            pCorutine->Resume(S);
            for (int i = 0; i <= CHECKCORUTINETIMES; i++){
                if (i % (CHECKCORUTINETIMES / 10) == 0){
                    printf("Check Resume: %d\r\n", i);
                }
                pCorutine->Resume(S);
            }
            printf("end Check %d tiems %d/%d StackSize:%d/%d\r\n", CHECKCORUTINETIMES, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        }
        {
#define CHECKCORUTINETIMES 1000
            CCorutinePlus* pCorutine = S->GetCorutine();
            pCorutine->ReInit([](CCorutinePlus* pCorutine)->void{
                CCorutinePlusPool* pRunPool = pCorutine->GetRunPool();
                for (int i = 0; i <= CHECKCORUTINETIMES; i++){
                    CCorutinePlus* pPlus = pRunPool->GetCorutine();
                    if (i % (CHECKCORUTINETIMES / 10) == 0){
                        printf("Check YieldCorutine: %d\r\n", i);
                    }
                    pPlus->ReInit([](CCorutinePlus* pCorutine)->void{
                        for (int i = 0; i <= CHECKCORUTINETIMES; i++){
                            pCorutine->YieldCorutine();
                        }
                    });
                    pPlus->Resume(pRunPool);
                    for (int j = 0; j <= CHECKCORUTINETIMES; j++){
                        pPlus->Resume(pRunPool);
                    }
                    pCorutine->YieldCorutine();
                }
            });
            printf("Start Check %d tiems %d/%d StackSize:%d/%d\r\n", CHECKCORUTINETIMES, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
            pCorutine->Resume(S);
            for (int i = 0; i <= CHECKCORUTINETIMES; i++){
                if (i % (CHECKCORUTINETIMES / 10) == 0){
                    printf("Check Resume: %d\r\n", i);
                }
                pCorutine->Resume(S);
            }
            printf("end Check %d tiems %d/%d StackSize:%d/%d\r\n", CHECKCORUTINETIMES, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        }
        {
#define CHECKCORUTINETIMES 1000
            CCorutinePlus* pCorutine = S->GetCorutine();
            pCorutine->ReInit(DieDaiTimes);
            printf("Start Check %d tiems %d/%d StackSize:%d/%d\r\n", CHECKCORUTINETIMES, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
            int nTime = 0;
            int nCreateTimes = 5;
            int nLimitTimes = 10;
            pCorutine->Resume(S, nTime, nCreateTimes, nLimitTimes);
            printf("end Check %d tiems %d/%d StackSize:%d/%d\r\n", CHECKCORUTINETIMES, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        }
    }
    else if (nType == 1){
        //≤‚ ‘ÀŸ∂»
        for (int i = 0; i < 10; i++)
        {
            CCorutinePlus* pCorutine = S->GetCorutine();
            pCorutine->ReInit([](CCorutinePlus* pCorutine)->void{
                CCorutinePlusPool* pRunPool = pCorutine->GetRunPool();
                for (int i = 0; i <= TIMES_FORTEST; i++){
                    pCorutine->YieldCorutine();
                }
            });
            clock_t begin = clock();
            pCorutine->Resume(S);
            for (int i = 0; i <= TIMES_FORTEST; i++){
                pCorutine->Resume(S);
            }
            clock_t end = clock();
            printf("TestCoroutine RY %d:%d items %d/%d StackSize:%d/%d\n", TIMES_FORTEST, end - begin, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        }
        for (int i = 0; i < 10; i++)
        {
            clock_t begin = clock();
            for (int i = 0; i <= TIMES_FORTEST; i++){
                CCorutinePlus* pCorutine = S->GetCorutine();
                pCorutine->ReInit([](CCorutinePlus* pCorutine)->void{
                    return;
                });
                pCorutine->Resume(S);
            }
            clock_t end = clock();
            printf("TestCoroutine R %d:%d items %d/%d StackSize:%d/%d\n", TIMES_FORTEST, end - begin, S->GetVTCorutineSize(), S->GetCreateCorutineTimes(), S->GetVTShareStackCount(), S->GetCreateTimesShareStackCount());
        }
        /*int nThread = 1;
        CCorutinePlus* pCorutine = S->GetCorutine();
        for (int i = 0; i < 10;i++)
        {
            clock_t begin = clock();
            for (int i = 0; i < TIMES_FORTEST; i++){
                char szBuf[16384] = { 0 };
                int nLength = 16384;

                pCorutine->ReInit(Foo);
                pCorutine->Resume(S, nThread, szBuf, nLength);
                pCorutine->Resume(S, nThread);
            }

            clock_t end = clock();
            printf("TestCoroutine %d:%d\n", TIMES_FORTEST, end - begin);
        }
        {
            basiclib::CMutex spinlock;
            basiclib::CSingleLock lock(&spinlock);
            clock_t begin = clock();
            for (int i = 0; i < TIMES_FORTEST; i++){
                lock.Lock();
                lock.Unlock();
            }

            clock_t end = clock();
            printf("mutex %d:%d\n", TIMES_FORTEST, end - begin);
        }
        {
            clock_t begin = clock();
            for (int i = 0; i < TIMES_FORTEST; i++){
                void* pBuffer = basiclib::BasicAllocate(16384);
                basiclib::BasicDeallocate(pBuffer);
            }

            clock_t end = clock();
            printf("malloc %d:%d\n", TIMES_FORTEST, end - begin);
        }*/
    }
	
	getchar();
	delete S;
}

