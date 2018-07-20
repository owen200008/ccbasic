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

//computer func call per ms
basiclib::SpinLock mutexSpinLock;
void TestCallFunc(int i){
    basiclib::CSpinLockFuncNoSameThreadSafe lock(&mutexSpinLock);
    lock.Lock();
    lock.UnLock();
}
void TestCallSameFunc(int i){
    basiclib::CSpinLockFunc lock(&mutexSpinLock);
    lock.Lock();
    lock.UnLock();
}

basiclib::CBasicObject* pCreate(){
    return new basiclib::CBasicObject();
}
void pFreeCreate(basiclib::CBasicObject* p){
    delete p;
}

basiclib::CMutex mutexData;
void TestMutexCallFunc(int i){
    basiclib::CSingleLock lock(&mutexData);
    lock.Lock();
}

int main(int argc, char* argv[])
{
	if (!IsSupportBasiclib()){
		printf("Not SupportBasiclib!!!");
		getchar();
		return 0;
	}
    BasicGetModuleTitle();
	srand(time(NULL) + basiclib::BasicGetTickTime());
    {
        clock_t begin = clock();
        for(int i = 0; i < TIMES_FAST; i++){
            TestCallFunc(i);
        }
        printf("this computer funccall add %.4f/ms\n", (double)TIMES_FAST / (clock() - begin));
    }
    {
        clock_t begin = clock();
        for(int i = 0; i < TIMES_FAST; i++){
            TestCallSameFunc(i);
        }
        printf("this computer funccall add %.4f/ms\n", (double)TIMES_FAST / (clock() - begin));
    }
    {
        clock_t begin = clock();
        for(int i = 0; i < TIMES_FAST; i++){
            TestMutexCallFunc(i);
        }
        printf("this computer mutex %.4f/ms\n", (double)TIMES_FAST / (clock() - begin));
    }
    {
        clock_t begin = clock();
        for(int i = 0; i < TIMES_FAST / 10; i++){
            pFreeCreate(pCreate());
        }
        printf("this computer malloc add %.4f/ms\n", (double)(TIMES_FAST / 10) / (clock() - begin));
    }

    StartCoroutineTest();
	//TestCoroutine();
	//TestThread();
	//TestStackWalk();
	//TestFastDelegate();
	//TestContainExt();
	//NetServerTest();
	//TestFunctionXiaolvTest();

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

	getchar();
	return 0;
}

