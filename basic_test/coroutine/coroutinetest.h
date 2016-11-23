#ifndef INC_BASKCTESTCOROUTINE_H
#define INC_BASKCTESTCOROUTINE_H

#include <basic.h>

void Foo(CCorutinePlus* pCorutine)
{
	int nThread = pCorutine->GetResumeParam<int>(0);
	for (int i = 0; i < 10; i++)
	{
		printf("Thread(%d) %d\n", nThread, i);
		pCorutine->YieldCorutine();
	}
}

void TestCoroutine(){
	CCorutinePlusPool* S = new CCorutinePlusPool();
	S->InitCorutine();
	CCorutinePlus* pCorutine = S->GetCorutine();
	pCorutine->ReInit(Foo);
	int nThread = 1;
	pCorutine->Resume(S, nThread);
	for (int i = 0; i < 10; i++)
	{
		printf("Start(%d) %d\n", nThread, i);
		pCorutine->Resume(S, nThread);
	}
	getchar();
	delete S;
}

#endif