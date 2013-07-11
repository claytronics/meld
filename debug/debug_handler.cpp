/*API TO HANDLE BREAKPOINTS, DUMPS, AND CONTINUES*/


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


using namespace std;
using namespace vm;
using namespace debugger;

namespace debugger {

#define SIZE (sizeof(uint64_t))
#define BROADCAST true;

    /*****************************************************************/

    /*GLOBAL STORED VARS: debug handling specific*/

    /****************************************************************/


    /*global variables to controll main thread*/
    static bool isSystemPaused = true;
    static bool isDebug = false;
    static bool isSimDebug = false;
    static bool isMpiDebug = false;

    /*the pointer to the list of break points*/
    static debugList factBreakList = NULL;

    /*pointer to the system state class*/
    static state *systemState;

    /******************************************************************/
    
    /*DEBUG INITIALIZERS*/

    /******************************************************************/

    /*setup simulation debugging mode*/
    void initSimDebug(void){
        setupFactList();
        pauseIt();
    }

    /*setup MPI debugging mode*/
    void initMpiDebug(void){
        setupFactList();
        std::queue<api::message_type*> messageQueue = 
            *(new std::queue<api::message_type*>());
    }
    
    /*extract the pointer to the system state*/
    void setState(vm::state& st){
        systemState = &st;
    }


    /*set up the list to store break points*/
    void setupFactList(void){
        factBreakList = newBreakpointList();
    }

    /*indicate to go into VM debugging mode*/
    void setDebuggingMode(bool setting){
        isDebug = setting;
    }

    /*indicate to go into SIM debugging mode*/
    void setSimDebuggingMode(bool setting){
        isSimDebug = setting;
    }

    /*indicate to go into MPI debugging mode*/
    void setMpiDebuggingMode(bool setting){
        isMpiDebug  = setting;
    }


    /*********************************************************************/
    
    /*SYSTEM STATE FUNCTIONS*/

    /*********************************************************************/

    /*returns if the VM is paused for debugging or not*/
    bool isTheSystemPaused(void){
        return isSystemPaused;
    }
    
    /*returns if the system is in SIMULATION debugging mode*/
    bool isInSimDebuggingMode(void){
        return isSimDebug;
    }


    /*returns if system is in VM debugging mode*/
    bool isInDebuggingMode(void){
        return isDebug;
    }

    /*returns if the system is in MPI debugging mode*/
    bool isInMpiDebuggingMode(void){
        return isMpiDebug;
    }

    /*return the pointer the list of breakpoints*/
    debugList getFactList(void){
        return factBreakList;
    }
    
    vm::state *getState(void){
        return systemState;
    }
        

    /**********************************************************************/

    /* I/0 SPECIFICATION PARSING */

    /*********************************************************************/


    /*returns the index of a character in a string,
     *if it is not there it returns -1*/
    int characterInStringIndex(string str, char character){
        for(unsigned int i = 0; i < str.length(); i++){
            if (str[i] == character)
                return (int)i;
        }
        return -1;
    }


    /*extracts the type from the specification from input line
     *returns the type of breakpoint from the specification
     *invariant-- the type is always specified*/
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


    /*extracts the name from the specification
     *returns the name from the specification
     *returns "" if name is not present*/
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


    /*extracts the node from the specification
     *returns the node from the specification
     *returns "" if node is not given*/
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

    /*given the encoding return the corresponding string for 
     *break point types*/
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



    /*********************************************************************/
    
    /*DEBUGGER HANDLING FUNCTIONS*/

    /*********************************************************************/




    /*given the specification, turn the breakPoint on by inserting
     *it into the breakpoint list*/
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


    /*divert where the output goes: either cout or over 
     *message passing*/
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


    /*pause the VM until further notice*/
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
            st.all->DATABASE->print_db_debug(msg,(unsigned int)nodeNumber);
        msg  << endl;

        msg << "Facts to be consumed:" << endl;
        st.print_local_tuples(msg);
        msg << endl << endl;

        msg << "Derived Facts:" << endl;
        st.print_generated_tuples(msg);
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


    /**********************************************************************/

    /*SIMULATION DEBUGGING FUNCTIONS*/

    /*********************************************************************/

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


    /*exrtact the intruction encoding from a message*/
    int getInstruction(uint64_t* msg){
        return (int)msg[2];
    }


    /*to be called when a debug message is recieved
     *from the simulator*/
    void handleDebugMessage(uint64_t *msg, state& st){
        int instruction = getInstruction(msg);
        string specification = getSpec(msg,instruction);
        debugController(st,instruction,specification);
    }



    /*execute instruction based on encoding and specification
      call from the debug_prompt*/
    void debugController(state& currentState,
                         int instruction, string specification){

        string type;
        string name;
        string node;


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
            case REMOVE:
                type = getType(specification);
                name = getName(specification);
                node = getNode(specification);
                if (removeBreakPoint(getFactList(),(char*)type.c_str(),
                                     (char *)name.c_str(),
                                     atoi(node.c_str())) < 0){
                    display("Breakpoint is not in List\n",REMOVE);
                } else {
                    display("Breakpoint removed\n",REMOVE);
                }
                break;
            case BREAKPOINT:
                activateBreakPoint(specification);
                instruction = NOTHING;
                break;
        }
    }


    /***************************************************************************/

    /*DEBUG MESSAGE SENDING*/

    /***************************************************************************/

    inline int getSize(string content){
        return  3 + ((content.size()+1) + (SIZE-(content.size()+1)%SIZE))/SIZE;
    }

    uint64_t* pack(int msgEncode, string content){

        int size;
        api::message_type *msg;
        char * temp;

        switch(msgEncode){

            //master tells processes to dump their state
            case DUMP:
            case UNPAUSE:
            case PAUSE:
                size = 3;
                msg = new api::message_type[size];
                msg[0] = size;
                msg[1] = DEBUG;
                msg[2] = msgEncode;
                return msg;
                break;

            case PRINTCONTENT:
            case BREAKFOUND:
            case BREAKPOINT:
                size = getSize(content);
                msg = new api::message_type[size];
                msg[0] = size;
                msg[1] = DEBUG;
                msg[2] = msgEncode;
                temp = (char*)&msg[3];
                sprintf(temp,"%s",content.c_str());
                return msg;
                break;
        }
        return NULL;
    }

    void send(int destination, int msgType,
              string content, bool broadcast = false)  {
        //api::debugSendMsg(destination,pack(msgType,content),broadcast);
    }

    void getMsg(int numberExpected){
        //api::debugGetMsgs();
        
    }

}



