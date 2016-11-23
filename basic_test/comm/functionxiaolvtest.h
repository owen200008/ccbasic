#ifndef INC_FUNCTIONXIAOLVTEST_H
#define INC_FUNCTIONXIAOLVTEST_H

#include <basic.h>


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
		printf("%d:%d\n", TIMES_FORTEST, end - begin);
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
		printf("%d:%d\n", TIMES_FORTEST, end - begin);
	}
}

#endif