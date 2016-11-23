#include "stdafx.h"
#include <stdlib.h>
#include <map>
#include <basic.h>
#include "net/nettest.h"
#include "misc/fastdelegatetest.h"
#include "exception/stackwalkcheck.h"

int _tmain(int argc, _TCHAR* argv[])
{
	//TestStackWalk();
	//TestFastDelegate();
	NetServerTest();
	getchar();
	return 0;
}

