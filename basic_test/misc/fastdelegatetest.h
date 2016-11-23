#ifndef INC_FASTDELEGATETNET_H
#define INC_FASTDELEGATETNET_H

#include <basic.h>

using namespace basiclib;
typedef fastdelegate::FastDelegate0<void> TestFast;
class A
{
public:
	void bindstd(const std::function<void(void)>& func){
		m_func = func;
	}

	void bindfast(TestFast a){
		m_fastfunc = a;
	}
	void bindfast2(TestFast& a){
		m_fastfunc = a;
	}
	void TestFunc(){

	}
	void Dostd(){
		m_func();
	}
	void Dofast(){
		m_fastfunc();
	}

	std::function<void(void)> m_func;
	TestFast	m_fastfunc;
};

void TestFastDelegate()
{
	BasicSetMemRunMemCheck(0x00000002);
	A a;
#define TIMES_FORTEST 1000000
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			void* pRet = malloc(4);
			free(pRet);
		}
		clock_t end = clock();
		printf("malloc %d:%d\n", TIMES_FORTEST, end - begin);
	}
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			void* pRet = basiclib::BasicAllocate(4);
			basiclib::BasicDeallocate(pRet);
		}
		clock_t end = clock();
		printf("basicmalloc %d:%d\n", TIMES_FORTEST, end - begin);
	}
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			a.bindfast(MakeFastFunction(&a, &A::TestFunc));
		}
		clock_t end = clock();
		printf("Fast %d:%d\n", TIMES_FORTEST, end - begin);
	}
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			a.bindfast2(MakeFastFunction(&a, &A::TestFunc));
		}
		clock_t end = clock();
		printf("Fast %d:%d\n", TIMES_FORTEST, end - begin);
	}
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			a.bindstd([&]()->void{

			});
		}
		clock_t end = clock();
		printf("&std::function %d:%d\n", TIMES_FORTEST, end - begin);
	}
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			a.bindstd([]()->void{

			});
		}
		clock_t end = clock();
		printf("std::function %d:%d\n", TIMES_FORTEST, end - begin);
	}
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			a.Dofast();
		}
		clock_t end = clock();
		printf("DoFast %d:%d\n", TIMES_FORTEST, end - begin);
	}
	{
		clock_t begin = clock();
		for (int i = 0; i < TIMES_FORTEST; i++)
		{
			a.Dostd();
		}
		clock_t end = clock();
		printf("DoStd %d:%d\n", TIMES_FORTEST, end - begin);
	}


}


#endif