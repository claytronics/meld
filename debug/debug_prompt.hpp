#ifndef DEBUG_PROMPT_HPP
#define DEBUG_PROMPT_HPP

#include "vm/state.hpp"
#include "debug/debug_list.hpp"

namespace debugger {

    void debug(vm::state& st);
    void *run(void *curState);
    void parseline(std::string line, vm::state& st, debugList& factBreaks);
    int handle_command(std::string command, debugList& factList);
    void help();

}

#endif
