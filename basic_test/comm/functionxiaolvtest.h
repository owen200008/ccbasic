#ifndef INC_FUNCTIONXIAOLVTEST_H
#define INC_FUNCTIONXIAOLVTEST_H

#include <basic.h>
#include "../headdefine.h"


class CTest : public basiclib::EnableRefPtr<CTest>
{
public:
	CTest(){

	}
	~CTest(){

	}
};
typedef basiclib::CBasicRefPtr<CTest> CRefTest;

int testaa = 0;
class CA
{
public:
	CA()
	{
		m_pTest = new CTest();
	}
	~CA()
	{
		delete m_pTest;
	}
	CTest* GetTest(int a){ testaa++;  return m_pTest; }
protected:
	CTest* m_pTest;
};

typedef void(*TestFunc)();
class CB
{
public:
	CB()
	{
		m_pTest = new CTest();
	}
	CRefTest& GetTest(){ basiclib::BasicInterlockedIncrement((LONG*)&testaa); basiclib::BasicInterlockedDecrement((LONG*)&testaa); return m_pTest; }

	void DoTest(TestFunc func)
	{
		testaa++;
		//func();
	}
protected:
	CRefTest m_pTest;
};

void DoTestFunc()
{

}

static HANDLE	m_hCompletionPort = NULL;		//完成端口
static BOOL		bTimeToKill = FALSE;

unsigned __stdcall ThreadPoolFunc(void* lpWorkContext){
	DWORD dwThreadID = basiclib::BasicGetCurrentThreadId();
	ULONG ulFlags = 0;

	DWORD dwIoSize = 0;
	LPOVERLAPPED lpOverlapped = NULL;

	HANDLE hHandle = nullptr;
	while(!bTimeToKill){
		dwIoSize = 0;
		lpOverlapped = NULL;
		hHandle = nullptr;
		// Get a completed IO request.
		BOOL bIORet = GetQueuedCompletionStatus(m_hCompletionPort, &dwIoSize, (LPDWORD)&hHandle, &lpOverlapped, INFINITE);  //I know this
	}
	return 0;
}
void TestFunctionXiaolvTest()
{
#define TIMES_FORTEST 100000000
	printf("测试函数调用效率\r\n");
	CA a;
	CB b;
	{
		clock_t begin = clock();
		CTest* pTest;
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			pTest = a.GetTest(i);
		}
		clock_t end = clock();
		printf("%d:%.2f\n", TIMES_FORTEST, (double)(end - begin) / CLOCKS_PER_SEC);
	}

	{
		clock_t begin = clock();
		CRefTest pTest;
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			//b.DoTest(DoTestFunc);
			b.DoTest([]()->void{
			});
			//pTest = b.GetTest();
		}
		clock_t end = clock();
		printf("%d:%.2f\n", TIMES_FORTEST, (double)(end - begin) / CLOCKS_PER_SEC);
	}
	{
		clock_t begin = clock();
		basiclib::CSemaphore sem;
		for(int i = 0; i < TIMES_FAST; i++){
			sem.Lock();
			sem.Unlock();
		}
		clock_t end = clock();
		printf("CSemaphore %d:%.2f\n", TIMES_FAST, (double)(end - begin) / CLOCKS_PER_SEC);
	}
	{
		clock_t begin = clock();
		basiclib::CMutex sem;
		for(int i = 0; i < TIMES_FAST; i++){
			sem.Lock();
			sem.Unlock();
		}
		clock_t end = clock();
		printf("CMutex %d:%.2f\n", TIMES_FAST, (double)(end - begin) / CLOCKS_PER_SEC);
	}
	{
		clock_t begin = clock();
		basiclib::CCriticalSection sem;
		for(int i = 0; i < TIMES_FAST; i++){
			sem.Lock();
			sem.Unlock();
		}
		clock_t end = clock();
		printf("CCriticalSection %d:%.2f\n", TIMES_FAST, (double)(end - begin) / CLOCKS_PER_SEC);
	}
	{
		//创建IOCP 并和 文件关联
		m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if(NULL == m_hCompletionPort){
			DWORD dwErr = GetLastError();
			if(ERROR_ALREADY_EXISTS != dwErr){
				return;
			}
		}
		clock_t begin = clock();
		for(int i = 0; i < TIMES_FAST; i++){
			PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD)nullptr, nullptr);
		}
		clock_t end = clock();
		printf("post %d:%.2f\n", TIMES_FAST, (double)(end - begin) / CLOCKS_PER_SEC);
	}
}

#endif