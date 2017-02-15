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
		char* pData = pCorutine->GetResumeParamPoint<char>(1);
		int n = pCorutine->GetResumeParam<int>(2);
		char szBuf[16384];
		memcpy(szBuf, pData, n);
		if (n != 16384 || szBuf[0] != '\0')
			basiclib::BasicLogEvent("1");
	}
}

void TestCoroutine(){
	CCorutinePlusPool* S = new CCorutinePlusPool();
	S->InitCorutine();
	CCorutinePlus* pCorutine = S->GetCorutine();
	pCorutine->ReInit(Foo);
	int nThread = 1;
	pCorutine->Resume(S, nThread);
	//printf("Start(%d) %d\n", nThread, i);
	{

		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++){
			char szBuf[16384] = {0};
			int nLength = 16384;
			pCorutine->Resume(S, nThread, szBuf, nLength);
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
	}
	getchar();
	delete S;
}

#endif