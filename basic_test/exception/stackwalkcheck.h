#ifndef INC_STACKWALKTNET_H
#define INC_STACKWALKTNET_H

#include <basic.h>
#include "../exception/call_stack.hpp"
void TestStackWalk(){
	//DumpRunMemCheck();
	stacktrace::call_stack callstack;
	TRACE(callstack.to_string().c_str());
}

#endif