
/* Interface for debugging*/

#include <pthread.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "debug_handler.hpp"

using namespace std;


void *run_debugger();
void *run_debugger_wrapper(void* vargp);
void parseline(string line);
int handle_command(string command);
void help();

int paused = 1;



/*spawn the debbugging prompt*/
void debug(){

  pthread_t tid;
  pthread_create(&tid,NULL,run_debugger_wrapper, NULL);

}


void *run_debugger_wrapper(void *vargp){
  (void)vargp;
  return run_debugger();
}

//continuously attend command line prompt
void *run_debugger(){ 
  string inpt;
  while(true){
    if (isTheSystemPaused()){
      cout << ">";
      getline(cin,inpt);
      parseline(inpt);
    }
  }
  return NULL;
}




/*parses the command line*/
void parseline(string line){

  string build = "";
  int wordCount = 1;
  
  int command = NOTHING;

  /*empty input*/
  if (line == ""){
    return;
  }

  for (unsigned int i = 0; i < line.length(); i++){
    

    /*parse line for words*/
    if (line[i]!=' '){
      build += line[i];
    } else {
      if (wordCount == 1)
	command = handle_command(build);
      if (wordCount == 2){
	if (command == BREAKPOINT)
	  inputInstruction(command,build);
	else 
	  inputInstruction(command,"");
	break;
      }
      wordCount++;
      build = "";
    }
    
  }

  /*no whitespace at all*/
  if (wordCount == 1){
      command = handle_command(build);
      inputInstruction(command, build);
  }
  
  if (command == BREAKPOINT && wordCount == 1)
      cout << "Please specify breakpoint type" << endl;

  if (wordCount == 2){
	if (command == BREAKPOINT)
	  inputInstruction(command,build);
	else 
	  inputInstruction(command,"");
      }

}


/*recognizes and sets different modes for the debugger*/
int handle_command(string command){

  int retVal;

  if (command == "breakpoint"){
    retVal = BREAKPOINT;
  } else if (command == "help"){
    help();
    retVal = NOTHING;
  } else if (command == "run") {
    cout << "Resuming VM ... " << endl;
    continueExecution();
    retVal = NOTHING;
  } else if (command == "dump") {
    cout << "Memory Dump" << endl;
    retVal = DUMP;
  } else if (command == "continue"){
    retVal = CONTINUE;
  } else if (command == "quit"){
    exit(0);
  } else {
    cout << "unknown command: type 'help' for options " << endl;
    retVal = NOTHING;
  }
  return retVal;
}

void help(){
  cout << "DEBUGGER HELP" << endl;
  cout << "\tbreakpoint- set break point" << endl;
  cout << "\tdump - dump the state of the system" << endl;
  cout << "\tcontinue - continue execution" << endl;
  cout << "\trun - start the program" << endl;
  cout << "\tquit - exit debugger" << endl;
}
  



  
    

  
