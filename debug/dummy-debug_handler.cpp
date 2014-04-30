#include <string.h>
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "db/database.hpp"
#include "debug/debug_prompt.hpp"
#include "debug/debug_handler.hpp"
#include "utils/serialization.hpp"
#include "utils/types.hpp"
#ifdef SIMD
#include "vm/determinism.hpp"
#endif

using namespace std;
using namespace vm;
using namespace debugger;


namespace debugger 
{
  bool isPausedInDeterministicPollLoop = false;
  std::queue<api::message_type*> *messageQueue = NULL;

  void runBreakPoint(char* type, string msg, char* name, int nodeID)
  {
  }
  
  bool isTheSystemPaused(void)
  {
  }
  
  bool isInSimDebuggingMode(void)
  {
  }
  
  void receiveMsg(bool poll)
  {
  }
  
  void pauseIt(void)
  {
  }

  bool isDebuggerQueueEmpty(void)
  {
  }

  void display(string msg, int type)
  {
  }
}














