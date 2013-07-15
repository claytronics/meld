#ifndef DEBUG_PROMPT_HPP
#define DEBUG_PROMPT_HPP

#include "vm/state.hpp"
#include "debug/debug_list.hpp"
#include "vm/all.hpp"

namespace debugger {

    void debug(vm::all* debugAll);
    void *run(void *debugAll);
    void parseline(std::string line, debugList& factBreaks);
    int handle_command(std::string command, debugList& factList);
    void help();

}

#endif
