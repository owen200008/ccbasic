#include <stdlib.h>
#include <map>
#include <basic.h>
#include "headdefine.h"
#include "net/nettest.h"
//#include "misc/fastdelegatetest.h"
//#include "exception/stackwalkcheck.h"
#include "util/containerexttest.h"
#include "util/cclockfreestacktest.hpp"
#include "thread/threadtest.h"
#include "coroutine/coroutinetest.h"
#include "scbasic/encode/rsatest.h"
#include "comm/functionxiaolvtest.h"
#include "sys/systemperformance.h"


int main(int argc, char* argv[]){
	if (!IsSupportBasiclib()){
		printf("Not SupportBasiclib!!!\n");
		getchar();
		return 0;
	}
	srand(time(NULL) + basiclib::BasicGetTickTime());

    SystemPerformace();
    //PrintSuccessOrFail(StartCoroutineTest);
	//TestThread();
	//TestStackWalk();
	//TestFastDelegate();
    for(int i = 0;i < 10;i++)
        PrintSuccessOrFail(CCLockfreeStackTest);
	//NetServerTest();
	//TestFunctionXiaolvTest();
    //PrintSuccessOrFail(TestRSA);

	getchar();
	return 0;
}

