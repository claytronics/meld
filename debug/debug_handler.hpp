
#ifndef DEBUG_HANDLER_HPP
#define DEBUG_HANDLER_HPP

#include <string>
#include "vm/state.hpp"
#include "debug/debug_list.hpp"

using namespace std;

#define DUMP 1
#define CONTINUE 7
#define BREAKPOINT 2
#define NOTHING 8
#define PAUSE 4
#define UNPAUSE 3 
#define BREAKFOUND 6
#define PRINT 5


#define FACTDER 1
#define FACTCON 2
#define FACTRET 3
#define ACTION 4
#define SENSE 5
#define BLOCK 6



void activateBreakPoint(string specification);
void runBreakPoint(char* type, string msg, char* name, int nodeID);
void pauseIt();
void dumpSystemState(vm::state& st);
void continueExecution();
void debugController(vm::state& currentState, int instruction, string specification);
bool isTheSystemPaused();
void setDebuggingMode(bool setting);
bool isInDebuggingMode();
bool isInSimDebuggingMode();
bool setSimDebuggingMode();
void setupFactList();
debugList getFactList();
void initSimDebug();
bool setSimDebuggingMode(bool setting);
void handleDebugMessage(uint64_t *msg, state& st);


#endif
