#ifndef DEBUG_HANDLER_HPP
#define DEBUG_HANDLER_HPP

#include <string>
#include "vm/state.hpp"
#include "debug/debug_list.hpp"
#include <queue>
#include "api/api.hpp"
#include "vm/all.hpp"
#include "db/node.hpp"

const int DUMP = 1;
const int CONTINUE = 7;
const int BREAKPOINT = 2;
const int NOTHING = 8;
const int REMOVE = 9;
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
const int DEBUG = 16;
const int DEBUGMPI = 0;

namespace api {
    const int MASTER = 1;
}

namespace debugger {

    extern std::queue<api::message_type*> *messageQueue;
    extern int numberExpected;
    extern vm::all *all;

    const int MASTER = 0;

    void activateBreakPoint(std::string specification);
    void runBreakPoint(char* type, std::string msg, char* name, int nodeID);
    void pauseIt(void);
    void dumpSystemState(int nodeNumber);
    void continueExecution(void);
    void debugController(int instruction, std::string specification);
    bool isTheSystemPaused(void);
    void setDebuggingMode(bool setting);
    bool isInDebuggingMode(void);
    bool isInSimDebuggingMode(void);
    void setupFactList(void);
    debugList getFactList(void);
    void initSimDebug(void);
    void setSimDebuggingMode(bool setting);
    void handleDebugMessage(uint64_t *msg);
    void display(std::string msg,int type);
    api::message_type getInstruction(api::message_type* msg);
    std::string getSpec(api::message_type* msg, int instruction);
    std::string typeInt2String(int type);
    std::string getNode(std::string specification);
    std::string getName(std::string specification);
    std::string getType(std::string specification);
    int characterInStringIndex(std::string str, char character);
    void initMpiDebug(vm::all *debugAll);
    bool isInMpiDebuggingMode(void);
    void setState(vm::state& st);
    void setMpiDebuggingMode(bool setting);
    vm::state *getState(void);
    void receiveMsg(void);
    void sendMsg(int destination, int msgType,
                 std::string content, bool broadcast = false);
}

#endif
