#ifndef INC_DLLTEST_H
#define INC_DLLTEST_H

#include <basic.h>
#include "basictesthead.h"

typedef void * (*skynet_dl_create)(void);
void TestDll()
{
	basiclib::CBasicLoadDll dll;
	dll.LoadLibrary("E:/github/skynetplus/skynettest.dll");
	dll.ReplaceDll(dll);
	skynet_dl_create p = (skynet_dl_create)dll.GetProcAddress("InitFunc");
	p();
	CTestHead* pHead = GetTestHead();
	pHead->CallFunc();
}

#endif