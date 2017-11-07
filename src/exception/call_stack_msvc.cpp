/* Copyright (c) 2009, Fredrik Orderud
   License: BSD licence (http://www.opensource.org/licenses/bsd-license.php) */

/* Windows (Microsoft visual studio) implementation of the call_stack class. */
#ifdef _WIN32 // also defined in 64bit

#include "call_stack.hpp"
#include "./StackWalker.h"
#include "../misc/fastdelegate.h"

typedef fastdelegate::FastDelegate2<StackWalker::CallstackEntryType /*eType*/, StackWalker::CallstackEntry&, void> pCallStackEntryFunc;
/** Adapter class to interfaces with the StackWalker project.
 *  Source: http://stackwalker.codeplex.com/ */
class StackWalkerAdapter : public StackWalker {
public:
    StackWalkerAdapter (const size_t num_discard) : 
        StackWalker(StackWalker::RetrieveVerbose | StackWalker::SymBuildPath) // do not use public Microsoft-Symbol-Server
    {
    }
    virtual ~StackWalkerAdapter () {}

protected:
	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry){
		if (m_pFunc)
			m_pFunc(eType, entry);
    }
	virtual void OnOutput(const char* /*szText*/) {
        // discard (should never be called)
    }
	virtual void OnSymInit(const char* /*szSearchPath*/, DWORD /*symOptions*/, const char* /*szUserName*/) {
        // discard
    }
	virtual void OnLoadModule(const char* img, const char* mod, DWORD64 baseAddr, DWORD size, DWORD result, const char* symType, const char* pdbName, ULONGLONG fileVersion){
	    // discard
    }
	virtual void OnDbgHelpErr(const char* /*szFuncName*/, DWORD /*gle*/, DWORD64 /*addr*/) {
        // discard
    }

public:
	pCallStackEntryFunc m_pFunc;

};
StackWalkerAdapter m_gStartAdapter(0);
struct CStackInfo{
public:
	CStackInfo(const size_t num_discard /*= 0*/) : discard_idx(static_cast<int>(num_discard)+2){
		
	}
	void Callback(StackWalker::CallstackEntryType eType, StackWalker::CallstackEntry &entry){
		if (entry.lineFileName[0] == 0)
			discard_idx = -1; // skip all entries from now on

		// discard first N stack entries
		if (discard_idx > 0) {
			discard_idx--;
		}
		else if (discard_idx == 0) {
			stacktrace::entry e;
			e.file = entry.lineFileName;
			e.line = entry.lineNumber;
			e.function = entry.name;
			stack.push_back(e);
		}
	}
	void ShowCallstack(){
		m_gStartAdapter.m_pFunc = MakeFastFunction(this, &CStackInfo::Callback);
		m_gStartAdapter.ShowCallstack();
		m_gStartAdapter.m_pFunc = nullptr;
	}
	std::vector<stacktrace::entry> stack;       ///< populated stack trace
	int                            discard_idx; ///< the number of stack entries to discard
};
namespace stacktrace {

// windows 32 & 64 bit impl.
call_stack::call_stack (const size_t num_discard /*= 0*/) {
	CStackInfo sw(num_discard);
    sw.ShowCallstack();
	swap(stack, sw.stack);
}

call_stack::~call_stack () throw() {
    // automatic cleanup
}

} // namespace stacktrace

#endif // _WIN32
