#include "stdafx.h"
#include <stdlib.h>
#include <map>
#include <basic.h>
#include "net/nettest.h"
#include "misc/fastdelegatetest.h"
#include "exception/stackwalkcheck.h"
#include "util\containerexttest.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CBasicBitstream ins;
	Net_Map<Net_Int, Net_Int> mapData;
	ins << mapData;
	//TestStackWalk();
	//TestFastDelegate();
	TestContainExt();
	getchar();
	return 0;
}

