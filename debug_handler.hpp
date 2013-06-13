
#ifndef DEBUG_HANDLER_HPP
#define DEBUG_HANDLER_HPP

#include <string>
#include "vm/state.hpp"
#include "debug_list.hpp"

using namespace std;

#define DUMP 0
#define CONTINUE 1
#define BREAKPOINT 2
#define NOTHING 3


void activateBreakPoint(string type);
void runBreakPoint(string type, string msg);
void pauseIt();
void dumpSystemState(vm::state& st);
void continueExecution();
void debugController(vm::state& currentState, int instruction, string specification);
bool isTheSystemPaused();
void setDebuggingMode(bool setting);
bool isInDebuggingMode();
void setupFactList();
debugList getFactList();

#endif
