#ifndef INC_BASKCTESTCOROUTINE_H
#define INC_BASKCTESTCOROUTINE_H

#include <basic.h>

#define TIMES_FORTEST 1000000
void Foo(CCorutinePlus* pCorutine)
{
	int nThread = pCorutine->GetResumeParam<int>(0);
	for (int i = 0; i < TIMES_FORTEST; i++)
	{
		pCorutine->YieldCorutine();
	}
}

void TestCoroutine(){
	CCorutinePlusPool* S = new CCorutinePlusPool();
	S->InitCorutine();
	CCorutinePlus* pCorutine = S->GetCorutine();
	pCorutine->ReInit(Foo);
	int nThread = 1;
	//pCorutine->Resume(S, nThread);
	//printf("Start(%d) %d\n", nThread, i);

	{

		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++){
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
		printf("TestCoroutine %d:%d\n", TIMES_FORTEST, end - begin);
	}
	getchar();
	delete S;
}

#endif