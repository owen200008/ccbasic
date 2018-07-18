#include <stdlib.h>
#include <map>
#include <basic.h>
#include "net/nettest.h"
//#include "misc/fastdelegatetest.h"
//#include "exception/stackwalkcheck.h"
#include "util/containerexttest.h"
#include "thread/threadtest.h"
#include "coroutine/coroutinetest.h"
#include "../scbasic/encode/rsaencode.h"
#include "comm/functionxiaolvtest.h"

int main(int argc, char* argv[])
{
	if (!IsSupportBasiclib()){
		printf("Not SupportBasiclib!!!");
		getchar();
		return 0;
	}
    BasicGetModuleTitle();
	srand(time(NULL) + basiclib::BasicGetTickTime());
	

	/*char szBuf[1024] = { 0 };
	for (int i = 0; i < 1024; i++) {
		szBuf[i] = rand() % 256;
	} 
	byte szBuf2[2048] = { 0 };
	byte szBuf3[2048] = { 0 };
	{
		CSCBasicRSA rsa;
		rsa.SetPrivateFileName("e:/private.a");
		CryptoPP::RSAES_OAEP_SHA_Encryptor		priEncode;
		rsa.SetPublicFileName("e:/public.a");
		int nTotalLength = rsa.Decrypt(szBuf, 1024, szBuf2, 2048);
		int nRetLength = rsa.Encrypt((char*)szBuf2, nTotalLength, szBuf3, 2048);
		ASSERT(nRetLength == 1024);
		ASSERT(memcmp(szBuf3, szBuf, nRetLength) == 0);
	}*/


	//TestCoroutine(1);
	//TestCoroutine();
	//TestThread();
	//TestStackWalk();
	//TestFastDelegate();
	TestContainExt();
	//NetServerTest();
	//TestFunctionXiaolvTest();
	getchar();
	return 0;
}

