/*API TO HANDLE BREAKPOINTS, DUMPS, AND CONTINUES*/


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

using namespace std;
using namespace vm;
using namespace debugger;

namespace debugger {

#define SIZE (sizeof(api::message_type))
#define BROADCAST true

    /*****************************************************************/

    /*GLOBAL STORED VARS: debug handling specific*/

    /****************************************************************/

    std::queue<api::message_type*> *messageQueue;
    /*global variables to controll main thread*/
    static bool isSystemPaused = true;
    static bool isDebug = false;
    static bool isSimDebug = false;
    static bool isMpiDebug = false;

    /*the pointer to the list of break points*/
    static debugList factBreakList = NULL;

    /*pointer to the system state class*/
    static state *systemState;

    int numberExpected = 0;

    vm::all* all;

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
        messageQueue = new std::queue<api::message_type*>();
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
            display("Please Enter a Type",PRINTCONTENT);
            return;
        }

        //parse for different specification formats
        string type = getType(specification);
        string name = getName(specification);
        string nodeID = getNode(specification);

        //if this type of break point is not valid
        if (type!="block"&&type!="action"&&type!="factDer"&&type!="sense"&&
            type!="factCon"&&type!="factRet"){
            display("Please Enter a Valid Type-- type help for options"
                    ,PRINTCONTENT);
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

        /*if normal- pass on the normal cout*/
        if (isInDebuggingMode())
            cout << msg;

        else if (isInSimDebuggingMode())
            return;

        /*if is in MPI debugging mode, send to master to display/handle
         *the message*/
        else if (isInMpiDebuggingMode()){
            cout << "child sending mesage" << endl;
            sendMsg(MASTER,type,msg);
        }
    }



    /*initiate the system to wait until further notice
     *--> to be inserted in the code of the actual VM
     *    at specific breakpoints*/
    void runBreakPoint(char* type, string msg, char* name, int nodeID){

        ostringstream MSG;


        /*if is not in any debugging mode- don't care, keep going*/
        if (!isInDebuggingMode()&&!isInSimDebuggingMode()&&
            !isInMpiDebuggingMode())
            return;

        /*check to see if any messages are available
         *(the system could be paused)*/
        if (isInMpiDebuggingMode())
            receiveMsg();

        /*if the system was paused, then pause it*/
        if (isTheSystemPaused())
            pauseIt();

        //if the specifications are a hit, then pause the system
        if (isInBreakPointList(factBreakList,type,name,nodeID)){
            MSG << "Breakpoint-->";
            MSG << type << ":" << name << "@" << nodeID << endl;
            MSG <<  msg;
            display(MSG.str(),BREAKFOUND);
            pauseIt();
        }
    }


    /*pause the VM until further notice*/
    void pauseIt(){

        isSystemPaused = true;
            while(isSystemPaused) {

                /*if is in MPI mode, recieve messages*/
                /*will breakout of loop if CONTINUE message is
                 * specified which is handled by debugController*/
                if (isInMpiDebuggingMode()){
                    api::debugWaitMsg();
                    receiveMsg();

                } else {
                    sleep(1);
                }
            }
    }



    /*display the contents of VM*/
    void dumpSystemState(int nodeNumber){

        ostringstream msg;

        msg << "Memory Dump:" << endl;
        msg << endl;
        msg << endl;

        //if a node is not specified by the dump command
        if (nodeNumber == -1)
            all->DATABASE->print_db(msg);
        else
            //print out only the given node
            all->DATABASE->print_db_debug(msg,(unsigned int)nodeNumber);
        msg  << endl;
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
    string getContent(api::message_type* msg){

        char* content = (char*)&msg[3];
        std::string str(content);
        return str;


    }


    /*exrtact the intruction encoding from a message*/
    api::message_type getInstruction(api::message_type* msg){
        return msg[2];
    }


    /*execute instruction based on encoding and specification
      call from the debug_prompt*/
    void debugController(int instruction, string specification){

        string type;
        string name;
        string node;

        /*for numberExpected see debug_prompt.cpp, run()*/

        /*if MPI debugging and the master process:
         *send a  message instead of changing the system state
         *as normally done in normal debugging*/
        if (isInMpiDebuggingMode() && api::world->rank()==MASTER){

            /*process of master debugger in MPI DEBUGGINGMODE*/
            if (instruction == CONTINUE || instruction == UNPAUSE){

                /*continue a paused system by broadcasting an UNPAUSE signal*/
                sendMsg(-1,CONTINUE,"",BROADCAST);
                numberExpected = 1;

            } else if (instruction == DUMP) {

                /*broadcast the message to all VMs*/
                if (specification == "all"){

                    sendMsg(-1,DUMP,"",BROADCAST);
                    /*wait for all VMs to receive (not counting the debugger
                     *itself*/
                    numberExpected = (int)api::world->size()-1;

                } else {

                    /*send to a specific VM to dump content*/
                    sendMsg(atoi(specification.c_str()),DUMP,"");
                    numberExpected = 1;
                }

                /*handle the breakpoints in the lists*/
            } else if (instruction == REMOVE||instruction == BREAKPOINT) {

                /*extract the destination*/
                node = getNode(specification);
                if (node == ""){

                    /*broadcast the message if the node is not specified*/
                    sendMsg(-1,instruction,specification,BROADCAST);
                    numberExpected = (int)api::world->size()-1;

                } else {

                    /*send break/remove to a specific node */
                    sendMsg(atoi(node.c_str()),instruction,specification);
                    numberExpected = 1;
                }


            } else if (instruction == PAUSE) {

                /*broadcast  a pause message*/
                sendMsg(-1,PAUSE,"",BROADCAST);
                numberExpected = 0;
            }


            /*this section pertains to the slave VMs that respond to the
             *master debugging process-- also run in normal execution of
             *a single VM (no MPI debugging mode, but normal debugging mode)*/
        } else {

            switch(instruction){

                case DUMP:
                    if (specification == "all"){
                        dumpSystemState(-1);
                    } else {
                        dumpSystemState(atoi(specification.c_str()));
                    }
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
    }


    /*the controller for the master MPI debugger-called when messages are
    *received*/
    void debugMasterController(int instruction, string specification){

        /*print the output and then tell all other VMs to pause*/
        if (instruction == BREAKFOUND){
            cout << specification;
            sendMsg(-1,PAUSE,"",BROADCAST);

        /*print content from a VM*/
        } else if (instruction == PRINTCONTENT){
            cout << specification;
        }
    }


    /***************************************************************************/

    /*DEBUG MESSAGE SENDING*/

    /***************************************************************************/


    /*return the size of a packed array stored with content of an unkown size*/
    inline int getSize(string content){

        /*will always return an integer*/
        return  3 + ((content.size()+1) + (SIZE-(content.size()+1)%SIZE))/SIZE;
    }


    /*given the type encoding and content, pack the information into
     *a sendable messeage*/
    api::message_type* pack(int msgEncode, string content){

        utils::byte* msg = new utils::byte[api::MAXLENGTH*SIZE];
        int pos = 0;
        int debugFlag =  DEBUG;
        int size;

        switch(msgEncode){


            case DUMP:
            case UNPAUSE:
            case PAUSE:
                size = 3;

                /*pack the size of the array*/
                utils::pack<size_t>(&size,1,msg,api::MAXLENGTH*SIZE,&pos);

                /*pack the debug indicator*/
                utils::pack<int>(&debugFlag,1,msg,api::MAXLENGTH*SIZE,&pos);

                /*pack the message encoding*/
                utils::pack<int>(&msgEncode,1,msg,api::MAXLENGTH*SIZE,&pos);

                return (api::message_type*)msg;
                break;

            case PRINTCONTENT:
            case BREAKFOUND:
            case BREAKPOINT:
                size = getSize(content);

                /*same as above for first three fields*/
                utils::pack<size_t>(&size,1,msg,api::MAXLENGTH*SIZE,&pos);
                utils::pack<int>(&debugFlag,1,msg,api::MAXLENGTH*SIZE,&pos);
                utils::pack<int>(&msgEncode,1,msg,api::MAXLENGTH*SIZE,&pos);

                /*add the content into the buffer*/
                utils::pack<char>((char*)content.c_str(),content.size()+1,
                                 msg,api::MAXLENGTH*SIZE,&pos);
                return (api::message_type*)msg;
                break;
        }

        return NULL;
    }



    /*the desination specified is the process*/
    /*send a debug message to another process through the MPI layer*/
    /*if send to all, specify BROADCAST (see top)*/
    void sendMsg(int destination, int msgType,
              string content, bool broadcast)  {

        /*pack the message*/
        api::message_type* msg = pack(msgType,content);

        /*extract the size in bytes*/
        size_t msgSizeInBytes = ((size_t)msg[0])*SIZE;

        if (broadcast)
            api::debugBroadcastMsg(msg,msgSizeInBytes);
        else {
            int desinationNodeId = TranslateUserIdToNodeId(destination);
            api::debugSendMsg(msg,msgSizeInBytes);

        }

    }


    /*populate the queue of messages from the mpi layer and handle them
     *until there are no more messages*/
    void receiveMsg(void){

        utils::byte *msg;
        int instruction;
        char specification[api::MAXLENGTH*SIZE];
        int pos = 0;
        int size;
        int debugFlag;

        /*load the message queue with messages*/
        api::debugGetMsgs();

        if (api::world->rank() == MASTER){
            cout << "master" << endl;
        } else {
            cout << "child" << endl;
        }

        /*process each message until empty*/
        while(!messageQueue->empty()){
            /*extract the message*/
            msg = (utils::byte*)messageQueue->front();
            cout << "sup" << endl;

            /*unpack the message into readable form*/
            utils::unpack<size_t>(msg,api::MAXLENGTH*SIZE,&pos,&size,1);
            utils::unpack<int>(msg,api::MAXLENGTH*SIZE,&pos,&debugFlag,1);
            utils::unpack<int>(msg,api::MAXLENGTH*SIZE,&pos,&instruction,1);
            utils::unpack<char>(msg,api::MAXLENGTH*SIZE,&pos,
                                &specification,1);
            string spec(specification);

            /*if the controlling process is recieving a message*/
            if (api::world->rank()==MASTER){
                cout << "slave" << endl;
                debugMasterController(instruction,spec);
                numberExpected--;

                /*if a slave process (any vm) is receiving the message*/
            } else {
                cout << "master" << endl;
                debugController(instruction,spec);
            }

            /*set up the variables and buffers for next message*/
            memset(specification,0,api::MAXLENGTH*SIZE);
            memset(msg,0,api::MAXLENGTH*SIZE);
            cout << "hello" << endl;
            messageQueue->pop();
            pos = 0;
        }
    }

}
