#include <stdlib.h>
#include <map>
#include <basic.h>
#include "headdefine.h"
#include "net/nettest.h"
//#include "misc/fastdelegatetest.h"
//#include "exception/stackwalkcheck.h"
#include "util/containerexttest.h"
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
    PrintSuccessOrFail(StartCoroutineTest);
	//TestCoroutine();
	//TestThread();
	//TestStackWalk();
	//TestFastDelegate();
	//TestContainExt();
	//NetServerTest();
	//TestFunctionXiaolvTest();
    PrintSuccessOrFail(TestRSA);

	getchar();
	return 0;
}

