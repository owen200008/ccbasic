#ifndef INC_THREADTEST_H
#define INC_THREADTEST_H

#include <basic.h>
#include "../headdefine.h"

using namespace basiclib;

CBasicThreadTLS tls;
int* g_p = 0;
void SetGI(int* pN){
	g_p = pN;
}
int* GetGI(){
	return g_p;
}
int* GetTLSValue() {
	return (int*)tls.GetValue();
}

void ReadSelfOrder(int fd, short event, void *arg)
{
}

THREAD_RETURN WorkerThreadTest(void *arg)
{
	void* pRet = BasicGetBasiclibGlobalTLS(0);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	pRet = BasicGetBasiclibGlobalTLS(1);
	printf("%d\n", pRet == nullptr ? 0 : 1);
		{
				clock_t begin = clock();
				int i = 0;
				for (i = 0; i < TIMES_FAST * 10; i++){
					int n = i;
					SetGI(&n);
					int* pRet = GetGI();
				}
				clock_t end = clock();
				printf("int设置读取效率 %d:%d\n", i, end - begin);
		}
		{
				clock_t begin = clock();
				int i = 0;
				for (i = 0; i < TIMES_FAST * 10; i++){
					int n = i;
					tls.SetValue(&n);
					int* pRet = GetTLSValue();
				}

				clock_t end = clock();
				printf("tls存取效率 %d:%d\n", i, end - begin);
		}

	return 0;
}


void TestThread()
{
	void* pRet = BasicGetBasiclibGlobalTLS(0);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	pRet = BasicGetBasiclibGlobalTLS(1);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	BasicSetBasiclibGlobalTLS(0, malloc(1));
	pRet = BasicGetBasiclibGlobalTLS(0);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	pRet = BasicGetBasiclibGlobalTLS(1);
	printf("%d\n", pRet == nullptr ? 0 : 1);
	tls.CreateTLS();
	for (int i = 0; i < 4; i++)
	{
		DWORD dwThreadServerID = 0;
		basiclib::BasicCreateThread(WorkerThreadTest, nullptr, &dwThreadServerID);
	}
}


#endif