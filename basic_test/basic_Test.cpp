#include <stdlib.h>
#include <map>
#include <basic.h>
#include "net/nettest.h"
//#include "misc/fastdelegatetest.h"
//#include "exception/stackwalkcheck.h"
//#include "util/containerexttest.h"
//#include "thread/threadtest.h"
#include "coroutine/coroutinetest.h"

int main(int argc, char* argv[])
{
	if (!IsSupportBasiclib()){
		printf("Not SupportBasiclib!!!");
		getchar();
		return 0;
	}
	TestCoroutine(1);
	TestCoroutine();
	//TestThread();
	//TestStackWalk();
	//TestFastDelegate();
	//TestContainExt();
	NetServerTest();
	getchar();
	return 0;
}

