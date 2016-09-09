/* Copyright (c) 2009, Fredrik Orderud
   License: BSD licence (http://www.opensource.org/licenses/bsd-license.php)
   Based on: http://stupefydeveloper.blogspot.com/2008/10/cc-call-stack.html */

/* Liux/gcc implementation of the call_stack class. */
#ifdef __GNUC__

#include <stdio.h>
#ifdef __ANDROID
#include <unwind.h>
#else
#include <execinfo.h>
#endif
#include <cxxabi.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "call_stack.hpp"

#define MAX_DEPTH 32

namespace stacktrace {
#ifdef __ANDROID
using namespace abi;
struct android_backtrace_state
{
	void **current;
	void **end;
};

_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context, void* arg)
{
    android_backtrace_state* state = (android_backtrace_state *)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) 
    {
        if (state->current == state->end) 
        {
            return _URC_END_OF_STACK;
        } 
        else 
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}
#else
call_stack::call_stack (const size_t num_discard /*= 0*/) {
	_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context, 
                                            void* arg)
{
    // retrieve call-stack
    void * trace[MAX_DEPTH];
#ifdef __ANDROID

	android_backtrace_state state;
    state.current = trace;
    state.end = trace + MAX_DEPTH;
	_Unwind_Backtrace(android_unwind_callback, &state);
	
	int stack_depth = (int)(state.current - trace);
#else
    int stack_depth = backtrace(trace, MAX_DEPTH);
#endif

    for (int i = num_discard+1; i < stack_depth; i++) {
        Dl_info dlinfo;
        if(!dladdr(trace[i], &dlinfo))
            break;

        const char * symname = dlinfo.dli_sname;

        int    status;
        char * demangled = abi::__cxa_demangle(symname, NULL, 0, &status);
        if(status == 0 && demangled)
            symname = demangled;

        //printf("entry: %s, %s\n", dlinfo.dli_fname, symname);

        // store entry to stack
        if (dlinfo.dli_fname && symname) {
            entry e;
            e.file     = dlinfo.dli_fname;
            e.line     = 0; // unsupported
            e.function = symname;
            stack.push_back(e);
        } else {
            break; // skip last entries below main
        }

        if (demangled)
            free(demangled);
    }
}
#endif

call_stack::~call_stack () throw() {
    // automatic cleanup
}

} // namespace stacktrace

#endif // __GNUC__
