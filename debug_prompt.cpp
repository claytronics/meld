
/* Interface for debugging- spawns a  prompt that will controll the main thread
 if it hits a specifies break point*/

#include <pthread.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "debug_handler.hpp"

using namespace std;
using namespace vm;

/*function prototypes*/
void *run_debugger(void * curState);
void parseline(string line, state& st);
int handle_command(string command);
void help();

/*to store the last input in the debugger*/
int lastInstruction = 0;
string lastBuild = "";




/*spawn the debbugging prompt as a separate thread to
  controll the main one*/
void debug(state& st){

  pthread_t tid;
  pthread_create(&tid,NULL,run_debugger, &st);

}



//continuously attend command line prompt
void *run_debugger(void * curState){
 
  string inpt;
  state st = *(state *)curState;

  while(true){
    if (isTheSystemPaused()){
      cout << ">";
      getline(cin,inpt);
      parseline(inpt,st);
    }
  }
  return NULL;
}




/*parses the command line*/
void parseline(string line, state& st){

  string build = "";
  int wordCount = 1;
  
  int command = NOTHING;

  /*empty input*/
  if (line == ""){
    //enterlast stored command
    debugController(st,lastInstruction, lastBuild);
    return;
  }

  /*loop through input line*/
  for (unsigned int i = 0; i < line.length(); i++){
    

    /*parse line for words*/
    if (line[i]!=' '){
      build += line[i];
    } else {
      if (wordCount == 1)
	command = handle_command(build);
    wordCount++;
    build = "";
    }
  }
    

  /*no whitespace at all*/
  if (wordCount == 1){
      command = handle_command(build);
      debugController(st,command, build);
      lastInstruction = command;
      lastBuild = build;
      return;
  }
  
  /*if not enough info*/
  if (command == BREAKPOINT && wordCount == 1){
      cout << "Please specify breakpoint type" << endl;
      return;
  }

  /*handle breakpoints*/
  if (wordCount == 2){
	if (command == BREAKPOINT)
	  debugController(st,command,build);
	else 
	  debugController(st,command,"");
      lastInstruction = command;
      lastBuild = build;
  }

}


/*recognizes and sets different modes for the debugger*/
int handle_command(string command){

  int retVal;

  if (command == "break"){
    retVal = BREAKPOINT;
  } else if (command == "help"){
    cout << endl;
    cout << "*******************************************************************" << endl;
    cout << endl;
    help();
    cout << endl;
    cout << "*******************************************************************" << endl;
    cout << endl;
    retVal = NOTHING;
  } else if (command == "run"|| command == "r") {
    retVal = CONTINUE;
  } else if (command == "dump"||command == "d") {
    cout << endl;
     cout << "*******************************************************************" << endl; 
    cout << "Memory Dump:" << endl;
    cout << endl;
    retVal = DUMP;
  } else if (command == "continue"||command == "c"){
    retVal = CONTINUE;
  } else if (command == "quit"||command == "q"){
    exit(0);
  } else {
    cout << "unknown command: type 'help' for options " << endl;
    retVal = NOTHING;
  }
  return retVal;
}


/*prints the help screen*/
void help(){
  cout << "DEBUGGER HELP" << endl;
  cout << "\t-break fact|action|sense - set break point at specified type" << endl;
  cout << "\t-dump or d - dump the state of the system" << endl;
  cout << "\t-continue or c - continue execution" << endl;
  cout << "\t-run or r - start the program" << endl;
  cout << "\t-quit - exit debugger" << endl;
  cout << endl;
  cout << "\t-Press Enter to use last Input" << endl;
}
  



  
    

  
