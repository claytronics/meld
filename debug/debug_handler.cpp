
/*API TO HANDLE BREAKPOINTS, DUMPS, AND CONTINUES*/


#include <pthread.h>
#include <iostream>
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
static bool isSystemPaused = true;
static bool isDebug = false;
static debugList factBreakList = NULL;



//starts the new fact list
void setupFactList(){
  factBreakList = newBreakpointList();
}

//returns the pointer to the list of break points
debugList getFactList(){
  return factBreakList;
};


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
void activateBreakPoint(string specification){

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
  if (type!="block"&&type!="action"&&type!="fact"&&type!="sense"){
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

  cout << "-->Breakpoint set with following conditions:" << endl;
  cout << "\tType: " << type << endl;
  if (name!="")
    cout << "\tName: " << name << endl;
  if (nodeID!="")
    cout <<  "\tNode: " << nodeID << endl;
    
  
}


/*initiate the system to wait until further notice*/
void runBreakPoint(char* type, string msg, char* name, int nodeID){
  
  if (!isInDebuggingMode())
    return;
  
  //if the specifications are a hit, then pause the system
  if (isInBreakPointList(factBreakList,type,name,nodeID)){
    pauseIt();
    cout << "Breakpoint-->";
    cout << type << ":" << name << "@" << nodeID << endl;
    cout << "\t-" <<  msg << endl;
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
  

