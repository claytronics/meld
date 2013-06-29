
/*API TO HANDLE BREAKPOINTS, DUMPS, AND CONTINUES*/


#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vm/state.hpp"
#include "db/database.hpp"
#include "debug/debug_prompt.hpp"
#include "debug/debug_list.hpp"

using namespace std;
using namespace vm;

/*function prototypes*/
void activateBreakPoint(string type);
void runBreakPoint(char* type, string msg, char* name, int nodeID);
void pauseIt();
void dumpSystemState(vm::state& st);
void continueExecution();
void debugController(vm::state& currentState, 
		     int instruction, string specification);
bool isTheSystemPaused();
void setDebuggingMode(bool setting);
void setSimDebuggingMode(bool setting);
bool isInDebuggingMode();
bool isInSimDebuggingMode();
debugList getFactList();
void setupFactList();
void debugSimController(state& currentState, 
			int instruction, string specification);
void display(string msg,int type);
/*command encodings*/

#define DUMP 1
#define CONTINUE 7
#define BREAKPOINT 2
#define NOTHING 8
#define PAUSE 4
#define UNPAUSE 3 
#define BREAKFOUND 6
#define PRINTCONTENT 5


#define FACTDER 1
#define FACTCON 2
#define FACTRET 3
#define ACTION 4
#define SENSE 5
#define BLOCK 6


/*global variables to controll main thread*/
static bool isSystemPaused = true;
static bool isDebug = false;
static bool isSimDebug = false;

/*the pointer to the list of break points*/
static debugList factBreakList = NULL;


void initSimDebug(){
  setupFactList();
  pauseIt();
}


//starts the new fact list
void setupFactList(){
  factBreakList = newBreakpointList();
}

//returns the pointer to the list of break points
debugList getFactList(){
  return factBreakList;
}


/*returns if the VM is paused for debugging or not*/
bool isTheSystemPaused(){
  return isSystemPaused;
}

//returns the index of a character in a string, 
//if it is not there it returns -1
int characterInStringIndex(string str, char character){
  for(unsigned int i = 0; i < str.length(); i++){
    if (str[i] == character)
      return (int)i;
  }
  return -1;
}


//extracts the type from the specification
//returns the type of breakpoint from the specification
string getType(string specification){
  string build = "";
    for (unsigned int i = 0; i < specification.length(); i++){
      if(specification[i] == ':' || specification[i] == '@') 
	return build;
      else 
	build += specification[i];
    }
    return build;
}


//extracts the name from the specification
//returns the name from the specification
//returns "" if name is not present
string getName(string specification){
  string build = "";
  //find index of colon
  int index = characterInStringIndex(specification, ':');
  // if colon not there
  if (index == -1)
    return "";
  for (unsigned int i = index +1; 
       i < specification.length(); i++){
    if (specification[i] == '@')
      return build;
    else
      build += specification[i];
  }
  return build;
}


//extracts the node from the specification
//returns the node from the specification
//returns "" if node is not given
string getNode(string specification){
  string build = "";
  int index = characterInStringIndex(specification, '@');
  if (index == -1)
    return "";
  for (unsigned int i = index+1; i < specification.length(); i++){
    build+=specification[i];
  }
  return build;
}



/*given the type, turn the breakPoint on*/
/*given the type, turn the breakPoint on by inserting
 it into the breakpoint list*/
void activateBreakPoint(string specification){

  ostringstream msg;

  //to follow a format that a type must be presented first
  if (specification[0] == ':'|| specification[0] == '@'){
    cout << "Please Enter a Type" << endl;
    return;
  }
  
  //parse for different specification formats
  string type = getType(specification);
  string name = getName(specification);
  string nodeID = getNode(specification);

  //if this type of break point is not valid
  if (type!="block"&&type!="action"&&type!="factDer"&&type!="sense"&&
      type!="factCon"&&type!="factRet"){
    cout << "Please Enter a Valid Type-- type help for options" << endl;
    return;
  }

  
  //create mempory on heap to store break point information
  char* type_copy = (char*)malloc(strlen(type.c_str())+1);
  char* name_copy = (char*)malloc(strlen(name.c_str())+1);
  int node_copy;

  //move the memory over
  memcpy(type_copy, (char*)type.c_str(),strlen(type.c_str())+1);
  memcpy(name_copy, (char*)name.c_str(),strlen(name.c_str())+1);
  
  if (nodeID != "") 
    node_copy = atoi(nodeID.c_str());
  else 
    node_copy = -1;

  //insert the information in the breakpoint list
  insertBreak(factBreakList,type_copy,name_copy, node_copy);
    

  msg << "-->Breakpoint set with following conditions:" << endl;
  msg  << "\tType: " << type << endl;
  if (name!="")
    msg << "\tName: " << name << endl;
  if (nodeID!="")
    msg <<  "\tNode: " << nodeID << endl;

  display(msg.str(),PRINTCONTENT);
  
}


void display(string msg, int type){
  if (isInDebuggingMode())
    cout << msg;
  else if (isInSimDebuggingMode())
    return;
}



/*initiate the system to wait until further notice
 *--> to be inserted in the code of the actual VM
 *    at specific breakpoints*/
void runBreakPoint(char* type, string msg, char* name, int nodeID){
  
  ostringstream MSG;

  if (!isInDebuggingMode()&&!isInSimDebuggingMode())
    return;

  if (isTheSystemPaused())
    pauseIt();
  
  //if the specifications are a hit, then pause the system
  if (isInBreakPointList(factBreakList,type,name,nodeID)){
      MSG << "Breakpoint-->";
      MSG << type << ":" << name << "@" << nodeID << endl;
      MSG <<  msg;
      display(MSG.str(),BREAKPOINT);
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
void dumpSystemState(state& st, int nodeNumber){

  ostringstream msg;
  
  msg << "*******************************************************************" << endl; 
  msg << "Memory Dump:" << endl;
  msg << endl;
  msg << endl;

  //if a node is not specified by the dump command
  if (nodeNumber == -1)
    st.all->DATABASE->print_db(msg);
  else 
    //print out only the given node
  //  st.all->DATABASE->print_db_debug(msg,(unsigned int)nodeNumber);
   msg  << endl;
  
   msg << "Facts to be consumed:" << endl;
  // st.print_local_tuples(msg);
   msg << endl << endl;

   msg << "Derived Facts:" << endl;
  // st.print_generated_tuples(msg);
   msg << endl << endl;


  msg << "*******************************************************************" << endl;
  msg << endl;

  display(msg.str(),PRINTCONTENT);
}


/*resume a paused system*/
void continueExecution(){
  //setting this will break it out of a while loop
  //from pauseIt function
  isSystemPaused = false;
}

/*turn debugging Mode on*/
void setDebuggingMode(bool setting){
  isDebug = setting;
}


void setSimDebuggingMode(bool setting){
  isSimDebug = setting;
}


bool isInSimDebuggingMode(){
  return isSimDebug;
}


/*check id debugging mode is on*/
bool isInDebuggingMode(){
  return isDebug;
}


string typeInt2String(int type){
  switch(type){
  case FACTDER:
    return "factDer";
  case FACTCON:
    return "factCon";
  case FACTRET:
    return "factRet";
  case ACTION:
    return "action";
  case SENSE:
    return "sense";
  case BLOCK:
    return "block";
  }
  return "";
}



/*returns the specification out of a message 
 *sent from the simulator*/
string getSpec(uint64_t* msg, int instruction){
  if (instruction == BREAKPOINT){
    int type = (int)msg[3];
    char* spec = (char*)msg[4];
    string str(spec);
    return typeInt2String(type) + ":" +  str;
  } else if (instruction == DUMP){
    return "all";
  } else { 
    return "";
  }
} 


int getInstruction(uint64_t* msg){ 
  return (int)msg[2];
}


//to be called when a debug message is recieved
void handleDebugMessage(uint64_t *msg, state& st){
  int instruction = getInstruction(msg);
  string specification = getSpec(msg,instruction);
  debugController(st,instruction,specification);
}
    


/*execute instruction based on encoding and specification
  call from the debug_prompt*/
void debugController(state& currentState,
		     int instruction, string specification){
    
    switch(instruction){

    case DUMP:
      if (specification == "all")
	dumpSystemState(currentState,-1);
      else 
	dumpSystemState(currentState, atoi(specification.c_str()));
      break;
    case PAUSE:
      isSystemPaused = true;
      break;
    case UNPAUSE:
    case CONTINUE:
      continueExecution();
      break;
    case BREAKPOINT:
      activateBreakPoint(specification);
      instruction = NOTHING;
      break;
    }
}
  

