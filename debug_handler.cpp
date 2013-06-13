
/*API TO HANDLE BREAKPOINTS, DUMPS, AND CONTINUES*/


#include <pthread.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vm/state.hpp"
#include "db/database.hpp"
#include "debug_prompt.hpp"
#include "debug_list.hpp"

using namespace std;
using namespace vm;

/*function prototypes*/
void activateBreakPoint(string type);
void runBreakPoint(string type, string msg);
void pauseIt();
void dumpSystemState(vm::state& st);
void continueExecution();
void debugController(vm::state& currentState, int instruction, string specification);
bool isTheSystemPaused();
void setDebuggingMode(bool setting);
bool isInDebuggingMode();
debugList getFactList();
void setupFactList();


/*command encodings*/
#define DUMP 0
#define CONTINUE 1
#define BREAKPOINT 2
#define NOTHING 3


/*global variables to controll main thread*/
static bool debug_BlockBreakPoint = false;
static bool debug_ActionBreakPoint = false;
static bool debug_SenseBreakPoint = false;
static bool debug_FactBreakPoint = false;
static bool isSystemPaused = true;
static bool isDebug = false;
static debugList factBreakList = NULL;



void setupFactList(){
  factBreakList = newBreakpointList();
}

debugList getFactList(){
  return factBreakList;
};


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
void dumpSystemState(state& st, int nodeNumber ){
  cout << "*******************************************************************" << endl; 
  cout << "Memory Dump:" << endl;
  cout << endl;
  cout << endl;

  if (nodeNumber == -1)
    st.all->DATABASE->print_db(cout);
  else 
    st.all->DATABASE->print_db_debug(cout,(unsigned int)nodeNumber);
    
  cout << endl;
  cout << "*******************************************************************" << endl;
  cout << endl;
}


/*resume a paused system*/
void continueExecution(){
  isSystemPaused = false;
}

/*turn debugging Mode on*/
void setDebuggingMode(bool setting){
  isDebug = setting;
}


/*check id debugging mode is on*/
bool isInDebuggingMode(){
  return isDebug;
}


/*execute instruction based on encoding and specification*/
void debugController(state& currentState,int instruction, string specification){
    
    switch(instruction){
    case DUMP:
      if (specification == "all")
	dumpSystemState(currentState,-1);
      else 
	dumpSystemState(currentState, atoi(specification.c_str()));
      instruction = NOTHING;
      break;
    case CONTINUE:
      continueExecution();
      instruction = NOTHING;
      break;
    case BREAKPOINT:
      activateBreakPoint(specification);
      instruction = NOTHING;
      break;
    }
}
  

