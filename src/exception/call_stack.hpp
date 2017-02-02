/* Copyright (c) 2009, Fredrik Orderud
   License: BSD licence (http://www.opensource.org/licenses/bsd-license.php) */

#pragma once
#include <string>
#include <vector>
#include <sstream>
#include "../inc/basic_def.h"

namespace stacktrace {

/** Call-stack entry datastructure. */
struct entry {
    /** Default constructor that clears all fields. */
    entry () : line(0) {
    }

    std::string file;     ///< filename
    size_t      line;     ///< line number
    std::string function; ///< name of function or method

    /** Serialize entry into a text string. */
    std::string to_string () const {
        std::ostringstream os;
		os << file << " (" << line << "): " << function << "\r\n";
        return os.str();
    }
};

/** Stack-trace base class, for retrieving the current call-stack. */
#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 4275)
class _BASIC_DLL_API call_stack {
public:
	call_stack(){
	}
    /** Stack-trace consructor.
     \param num_discard - number of stack entries to discard at the top. */
    call_stack (const size_t num_discard);

    virtual ~call_stack () throw();

    /** Serializes the entire call-stack into a text string. */
    std::string to_string () const {
        std::ostringstream os;
        for (size_t i = 0; i < stack.size(); i++)
            os << stack[i].to_string();
        return os.str();
    }
	void SwapStack(call_stack& dtStack){
		swap(stack, dtStack.stack);
	}
    /** Call stack. */
    std::vector<entry> stack;
};
#pragma warning (pop)
} // namespace stacktrace
