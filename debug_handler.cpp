
/*FUNCTIONS TO HANDLE BREAKPOINTS, DUMPS, AND CONTINUES*/

#include <pthread.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vm/state.hpp"
#include "db/database.hpp"

using namespace std;
using namespace vm;

void *debugController(void* st);
void initiateDebugController();
void activateBreakPoint(string type);
void runBreakPoint(string type, string msg);
void pauseIt();
void dumpSystemState();
void continueExecution();
bool isInDebuggingMode();
void setDebuggingMode(bool setting);

#define DUMP 0
#define CONTINUE 1
#define BREAKPOINT 2
#define NOTHING 3

bool debug_BlockBreakPoint = false;
bool debug_ActionBreakPoint = false;
bool debug_SenseBreakPoint = false;
bool debug_FactBreakPoint = false;
bool isSystemPaused = true;
int instruction = NOTHING;
string typeOfBreak = "";
bool isDebug = false;



#define DUMP 0
#define CONTINUE 1
#define BREAKPOINT 2
#define NOTHING 3


/*returns if the VM is paused for debugging or not*/
bool isTheSystemPaused(){
  return isSystemPaused;
}


/*given the type, turn the breakPoint on*/
void activateBreakPoint(string type){
  if (type == "block"){
    cout << "Breakpoint:block set" << endl;
    debug_BlockBreakPoint = true;
  } else if (type == "sense") {
    cout << "Breakpoint:sense set" << endl;
    debug_SenseBreakPoint = true;
  } else if (type == "action") {
    cout << "Breakpoint:action set" << endl;
    debug_ActionBreakPoint = true;
  } else if (type == "fact") {
    cout << "Breakpoint:fact set" << endl;
    debug_FactBreakPoint = true;
  }
}


/*initiate the system to wait until further notice*/
void runBreakPoint(string type,string msg){

    /*Block Break Points*/
  if (type == "block" && debug_BlockBreakPoint == true){
    cout << "breakpoint: block" << endl;
    cout << msg << endl;
    pauseIt();

    /*Sense Break Points*/
  } else if (type == "sense" && debug_SenseBreakPoint == true) {
    cout << "breakpoint: sense" << endl; 
    cout << msg << endl;
    pauseIt();

    /*Action Break Points*/
  } else if (type == "action" && debug_ActionBreakPoint == true) {
    cout << "breakpoint: action" << endl;
    cout << msg << endl;
    pauseIt();
   
    /*Fact Break Points*/
  } else if (type == "fact" && debug_FactBreakPoint == true) {
    cout << "breakpoint: fact" << endl;
    cout << msg << endl;
    pauseIt();
  }
}


/*pause the VM*/
void pauseIt(){
  isSystemPaused = true;
  while(isSystemPaused == true)
    sleep(1);
}


/*display the contents of VM*/
void dumpSystemState(state& systemState){
  cout << "muahhahahhahahahahahah! No memory dump for you!" << endl;
  systemState.all->DATABASE->print_db(cout);
}

/*resume a paused system*/
void continueExecution(){
  isSystemPaused = false;
}

void setDebuggingMode(bool setting){
  isDebug = setting;
}

bool isInDebuggingMode(){
  return isDebug;
}


/*used after instruction message is recieved*/
/*the string speicification if used for a particular breakpoint*/
int inputInstruction(int instr_encoding,string specification){
  (void)specification;
  instruction = instr_encoding;
  if (instr_encoding == BREAKPOINT){
    activateBreakPoint(specification);
  }
  return instruction;
}
  

void initiateDebugController(state& st){
  pthread_t tid;
  pthread_create(&tid, NULL, debugController,&st);
}

/*ran as a separate thread*/
void *debugController(void* st){

  state currentState = *(state*)st;
    
  while(true){
    switch(instruction){
    case DUMP:
      //dumpSystemState(currentState);
      instruction = NOTHING;
      break;
    case CONTINUE:
      continueExecution();
      instruction = NOTHING;
      break;
    case BREAKPOINT:
      //activateBreakPoint("block");
      instruction = NOTHING;
      break;
    default:
      break;
    }
  }

  return NULL;

}
  

