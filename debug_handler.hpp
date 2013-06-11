
#ifndef DEBUG_HANDLER_HPP
#define DEBUG_HANDLER_HPP

#include <string>
#include "vm/state.hpp"

using namespace std;
using namespace vm;

#define DUMP 0
#define CONTINUE 1
#define BREAKPOINT 2
#define NOTHING 3


void activateBreakPoint(string type);
void runBreakPoint(string type, string msg);
void pauseIt();
void inputInstruction(int instr_encoding, string specification);
void dumpSystemState();
void continueExecution();
void *debugController(void* st);
bool isTheSystemPaused();
void initiateDebugController();
void setDebuggingMode(bool setting);
bool isInDebuggingMode();

#endif
