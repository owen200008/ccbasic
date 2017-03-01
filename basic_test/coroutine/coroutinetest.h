#ifndef INC_BASKCTESTCOROUTINE_H
#define INC_BASKCTESTCOROUTINE_H

#include <basic.h>

#define TIMES_FORTEST 1000000
void Foo(CCorutinePlus* pCorutine)
{
	char* pData = pCorutine->GetResumeParamPoint<char>(1);
	int n = pCorutine->GetResumeParam<int>(2);
	char szBuf[16384];
	memcpy(szBuf, pData, n);
	if (n != 16384 || szBuf[0] != '\0')
		basiclib::BasicLogEvent("1");
	pCorutine->YieldCorutine();
}

void TestCoroutine(){
	CCorutinePlusPool* S = new CCorutinePlusPool();
	S->InitCorutine();
	
	int nThread = 1;
	CCorutinePlus* pCorutine = S->GetCorutine();
	//printf("Start(%d) %d\n", nThread, i);
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++){
			char szBuf[16384] = {0};
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
	}
	getchar();
	delete S;
}

#endif