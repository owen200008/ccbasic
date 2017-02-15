#include "stdafx.h"
#include <stdlib.h>
#include <map>
#include <basic.h>
#include "net/nettest.h"
#include "misc/fastdelegatetest.h"
#include "exception/stackwalkcheck.h"
#include "util\containerexttest.h"
#include "thread\threadtest.h"
#include "coroutine\coroutinetest.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if (!IsSupportBasiclib()){
		printf("Not SupportBasiclib!!!");
		getchar();
		return 0;
	}
	//TestCoroutine();
	//TestThread();
	//TestStackWalk();
	//TestFastDelegate();
	TestContainExt();
	getchar();
	return 0;
}

