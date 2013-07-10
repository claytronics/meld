
#ifndef DEBUG_HANDLER_HPP
#define DEBUG_HANDLER_HPP

#include <string>
#include "vm/state.hpp"
#include "debug/debug_list.hpp"

using namespace std;


const int DUMP = 1;
const int CONTINUE = 7;
const int BREAKPOINT = 2;
const int NOTHING = 8;
const int PAUSE = 4;
const int UNPAUSE = 3;
const int BREAKFOUND = 6;
const int PRINTCONTENT = 5;
const int FACTDER = 1;
const int FACTCON = 2;
const int FACTRET = 3;
const int ACTION  = 4;
const int SENSE = 5;
const int BLOCK = 6;


namespace debugger {


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
void setupFactList();
debugList getFactList();
void initSimDebug();
void setSimDebuggingMode(bool setting);
void handleDebugMessage(uint64_t *msg, vm::state& st);
void display(string msg,int type);
  int getInstruction(uint64_t* msg);
  string getSpec(uint64_t* msg, int instruction);
  string typeInt2String(int type);
  string getNode(string specification);
  string getName(string specification);
  string getType(string specification);
  int characterInStringIndex(string str, char character);

}

#endif
