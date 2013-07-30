/*INTERFACE TO HANDLE BREAKPOINTS, DUMPS, AND CONTINUES*/


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


    /*message Queue to handle incomming messages*/
    /*the api layer should use this when handling with debuf functions*/
    std::queue<api::message_type*> *messageQueue;


    /*>>> THE FOLLOWING FOUR ARE USED FOR SYCHRONIZATION <<< */
    /*is in the pauseIt loop (see pauseIt)*/
    static bool isSystemPaused = true;
    /*used to only broadcast one pause message
     *when a breakpoint is hit*/
    static bool okayToBroadcastPause = false;
    /*the program is suspended in the do_loop function
     *see (sched/base.cpp)*/
    bool isPausedInWorkLoop = false;
    /*the program has reached a breakpoint*/
    static bool isPausedAtBreakpoint = false;


    /*different debugging modes*/
    static bool isDebug = false;
    static bool isSimDebug = false;
    static bool isMpiDebug = false;


    /*different mode settings for debugger*/
    bool verboseMode = false;
    bool serializationMode = false;

    /*the pointer to the list of break points*/
    static debugList factBreakList = NULL;

    /*number of messages the Master expects to recieve*/
    int numberExpected = 0;

    /*used to store the state of the system to dump/print system*/
    vm::all* all;

    /******************************************************************/

    /*DEBUG INITIALIZERS*/

    /******************************************************************/

    /*setup simulation debugging mode*/
    void initSimDebug(vm::all *debugAll){
        all = debugAll;
        setupFactList();
        messageQueue = new std::queue<api::message_type*>();
    }

    /*setup MPI debugging mode*/
    void initMpiDebug(vm::all *debugAll){
        messageQueue = new std::queue<api::message_type*>();
        all = debugAll;
        if (api::world->rank()!=MASTER){
            setupFactList();
        }
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


    /**********************************************************************/

    /* I/0 SPECIFICATION PARSING */

    /*********************************************************************/


    /*CHARACTERINSTRINGINDEX--returns the index of a character in a string,
     * if it is not there it returns -1*/
    int characterInStringIndex(string str, char character){
        for(unsigned int i = 0; i < str.length(); i++){
            if (str[i] == character)
                return (int)i;
        }
        return -1;
    }


    /*GETTYPE--extracts the type from the specification from input line
     * returns the type of breakpoint from the specification
     * invariant-- the type is always specified*/
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


    /*GETNAME--extracts the name from the specification
     * returns the name from the specification
     * returns "" if name is not present*/
    string getName(string specification){
        string build = "";
        /*find index of colon*/
        int index = characterInStringIndex(specification, ':');
        /*if colon not there*/
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


    /*GETNODE--extracts the node from the specification
     * returns the node from the specification
     * returns "" if node is not given*/
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

    /*TYPEINT2STRING--given the encoding return the corresponding string for
     * break point types
     * returns "" if not a correct type*/
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




    /*ACTIVATEBREAKBOINT--given the specification, turn the breakPoint on by inserting
     * it into the breakpoint list*/
    void activateBreakPoint(string specification){

        ostringstream msg;

        /*to follow a format that a type must be presented first*/
        if (specification[0] == ':'|| specification[0] == '@'){
            display("Please Enter a Type\n",PRINTCONTENT);
            return;
        }

        /*parse for different specification formats*/
        string type = getType(specification);
        string name = getName(specification);
        string nodeID = getNode(specification);

        /*if this type of break point is not valid*/
        if (type!="block"&&type!="action"&&type!="factDer"&&type!="sense"&&
            type!="factCon"&&type!="factRet"){
            display("Please Enter a Valid Type-- type help for options\n"
                    ,PRINTCONTENT);
            return;
        }


        /*create mempory on heap to store break point information*/
        char* type_copy = (char*)malloc(strlen(type.c_str())+1);
        char* name_copy = (char*)malloc(strlen(name.c_str())+1);
        int node_copy;

        /*move the memory over*/
        memcpy(type_copy, (char*)type.c_str(),strlen(type.c_str())+1);
        memcpy(name_copy, (char*)name.c_str(),strlen(name.c_str())+1);

        if (nodeID != "")
            node_copy = atoi(nodeID.c_str());
        else
            node_copy = -1;

        /*insert the information in the breakpoint list*/
        insertBreak(factBreakList,type_copy,name_copy, node_copy);

        msg << "-->Breakpoint set with following conditions:" << endl;
        msg  << "\tType: " << type << endl;
        if (name!="")
            msg << "\tName: " << name << endl;
        if (nodeID!="")
            msg <<  "\tNode: " << nodeID << endl;


        display(msg.str(),PRINTCONTENT);

    }


    /*DISPLAY--divert where the output goes: either cout or over
     * message passing*/
    void display(string msg, int type){

        ostringstream MSG;

        /*if normal- pass on the normal cout*/
        if (isInDebuggingMode())
            cout << msg;
        else if (isInSimDebuggingMode()) {
           MSG << "<=======VM#" <<
                api::getNodeID()
                << "===================================================>"
                << endl << msg;
            sendMsg(api::getNodeID(),type,MSG.str());
        /*if is in MPI debugging mode, send to master to display/handle
         *the message*/
        } else if (isInMpiDebuggingMode()){
            MSG << "<=======VM#" <<
                api::world->rank()
                << "===================================================>"
                << endl << msg;
            sendMsg(MASTER,type,MSG.str());
        }
    }



    /*RUNBREAKPOINT--initiate the system to wait until further notice
     *--> to be inserted in the code of the actual VM
     *    at specific breakpoints
     *-->acts as a filter to check if a breakpoint has been reached or not*/
    void runBreakPoint(char* type, string msg, char* name, int nodeID){

        ostringstream MSG;


        /*if is not in any debugging mode- don't care, keep going*/
        if (!isInDebuggingMode()&&!isInSimDebuggingMode()&&
            !isInMpiDebuggingMode())
            return;

        /*if the specifications are a hit, then pause the system
         *and notify user*/
        if (isInBreakPointList(factBreakList,type,name,nodeID)){
            isPausedAtBreakpoint = true;
            MSG << "Breakpoint-->";
            MSG << type << ":" << name << "@" << nodeID << endl;
            MSG <<  msg;
            display(MSG.str(),BREAKFOUND);
            pauseIt();
        }
    }


    /*PAUSEIT--pause the VM until further notice
     * if in Sim or MPI debugging mode check for messages
     * to tell it what to do*/
    void pauseIt(){

        isSystemPaused = true;
        while(isSystemPaused) {

            /*if is in MPI mode, recieve messages*/
            /*will breakout of loop if CONTINUE message is
             * specified which is handled by debugController*/
            if (isInMpiDebuggingMode()){
                    api::debugWaitMsg();
                    receiveMsg();
            } else if (isInSimDebuggingMode()){
                receiveMsg();

            /*for normal debugging mode*/
            } else {
                sleep(1);
            }
        }
    }



    /*DUMPSYSTEMSTATE--display the contents of VM
     * by calling VM database functions */
    void dumpSystemState(int nodeNumber){

        ostringstream msg;

        /*if a node is not specified by the dump command*/
        if (nodeNumber == -1)
            all->DATABASE->print_entire_db_debug(msg);
        else
            /*print out only the given node*/
            all->DATABASE->print_db_debug(msg,nodeNumber);

        display(msg.str(),PRINTCONTENT);
    }


    /*CONTINUEEXECUTION--resume a paused system*/
    void continueExecution(){
        /*setting this will break it out of a while loop
         *from pauseIt function*/
        isSystemPaused = false;
    }


    /**********************************************************************/

    /*SIMULATION DEBUGGING FUNCTIONS*/

    /*********************************************************************/


    /*SETFLAGS -- parse input for different debugging mode flags.
     * the order of the flags does not matter
     * if a valid flag, turn mode on and notify user*/
    void setFlags(string specification){
        ostringstream msg;
        for (int i = 0; i < specification.length(); i++){
            if ((uint)specification[i] == 'V'){
                verboseMode = true;
                msg << "-verbose mode set" << endl;
            } else if ((uint)specification[i] == 'S'){
                msg << "-serialzation mode set" << endl;
                serializationMode = true;
            }
        }
        if (isInMpiDebuggingMode()&&api::world->rank()!=MASTER)
            display(msg.str(),PRINTCONTENT);
    }


    /*DEBUGCONTROLLER -- main controller of pausing/unpausing/dumping VMs*/
    /* execute instruction based on encoding and specification
     * call from the debug_prompt -- There are two different sides to
     * this function:  There is one side that handles sending messages to
     * processes in which these process will recieve that message
     * The other side pertains to the processes that are controlled by the
     * the master process.  They will change their system state and give
     * feed back to the master process (see debugger::display())
     * When the master sends a message, it will expect to see a certain
     * amount of messages sent back*/
    void debugController(int instruction, string specification){

        string type;
        string name;
        string node;

        /*for use of numberExpected see debug_prompt.cpp, run()*/

        /*if MPI debugging and the master process (process zero):
         *send a  message instead of changing the system state
         *as normally done in normal debugging*/
        if (isInMpiDebuggingMode() && api::world->rank()==MASTER){

            /*process of master debugger in MPI DEBUGGINGMODE*/
            if (instruction == CONTINUE || instruction == UNPAUSE){

                okayToBroadcastPause = true;
                /*continue a paused system by broadcasting an UNPAUSE signal*/
                sendMsg(-1,CONTINUE,"",BROADCAST);
                numberExpected = api::world->size()-1;

            } else if (instruction == RUN) {

                okayToBroadcastPause = true;
                sendMsg(-1,RUN,"",BROADCAST);
                numberExpected = api::world->size()-1;

            } else if (instruction == MODE) {

                setFlags(specification);
                sendMsg(-1,MODE,specification,BROADCAST);
                numberExpected = api::world->size()-1;

            } else if (instruction == DUMP) {

                /*broadcast the message to all VMs*/
                if (specification == "all"){

                    sendMsg(-1,DUMP,specification,BROADCAST);
                    /*wait for all VMs to receive (not counting the debugger
                     *itself*/
                    numberExpected = (int)api::world->size()-1;

                } else {

                    /*send to a specific VM to dump content*/
                    sendMsg(atoi(specification.c_str()),DUMP,specification);
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


            } else if (instruction == PRINTLIST) {

                /*broadcast  a print message*/
                sendMsg(-1,PRINTLIST,"",BROADCAST);
                numberExpected = (int)api::world->size()-1;

            }


        /*this section pertains to the slave VMs that respond to the
         *master debugging process-- also run in normal execution of
         *a single VM (no MPI debugging mode, but normal debugging mode)
         * also run by VMs if in Sim debugging mode*/
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
                    /*sychronization conditions -- if already paused
                     *don't do anything, the master already knows
                     *they are paused*/
                    if (isPausedInWorkLoop)
                        break;
                    if (isPausedAtBreakpoint){
                        break;
                    }
                    /*if it never left a pause loop (continue would have been called
                     * and pause message repaused it without leaving loop)*/
                    if (isSystemPaused){
                        display("CURRENTLY PAUSED",PAUSE);
                        break;
                    }
                    /*pause it otherwise*/
                    isSystemPaused = true;
                    break;
                case UNPAUSE:
                case CONTINUE:
                    /*unleash all bounds to system*/
                    isPausedAtBreakpoint = false;
                    isPausedInWorkLoop = false;
                    continueExecution();
                    break;
                case REMOVE:
                    /*remove breakpoint from list*/
                    type = getType(specification);
                    name = getName(specification);
                    node = getNode(specification);
                    if (removeBreakPoint(getFactList(),(char*)type.c_str(),
                                         (char *)name.c_str(),
                                         atoi(node.c_str())) < 0){
                        display("Breakpoint is not in List\n",PRINTCONTENT);
                    } else {
                        display("Breakpoint removed\n",PRINTCONTENT);
                    }
                    break;
                case BREAKPOINT:
                    activateBreakPoint(specification);
                    break;
                case TERMINATE:
                    /*if quit command was specified*/
                    api::end();
                    listFree(getFactList());
                    delete messageQueue;
                    exit(0);
                    break;
                case PRINTLIST:
                {
                    /*print the debugging breakpoint list*/
                    std::ostringstream printListMsg;
                    printList(printListMsg,getFactList());
                    display(printListMsg.str(),PRINTCONTENT);
                }
                    break;
                case RUN:
                    /*similar to continue--set aside if
                     *want to run a certain way*/
                    isPausedAtBreakpoint = false;
                    isPausedInWorkLoop = false;
                    continueExecution();
                    break;
                case MODE:
                    /*turn on different modes*/
                    setFlags(specification);
                    break;


            }
        }
    }


    /*DEBUGMASTERCONTROLLER-the controller for the master MPI debugger-called when messages are
    *received*/
    void debugMasterController(int instruction, string specification){

        /*print the output and then tell all other VMs to pause*/
        if (instruction == BREAKFOUND){

            printf("%s",specification.c_str());
            /*allows for only one pause massage to be sent
             *per CONTINUE instruction*/
            if (okayToBroadcastPause){
                sendMsg(-1,PAUSE,"",BROADCAST);
                okayToBroadcastPause = false;
            }

        } else if (instruction == PRINTCONTENT){

            printf("%s",specification.c_str());

        } else if (instruction == TERMINATE){

            printf("PROGRAM FINISHED\n");
            api::end();
            exit(0);

        } else if (instruction == PAUSE){

            /*prints more information*/
            //if (verboseMode){
                printf("%s",specification.c_str());
            //}

        }
    }


    /***************************************************************************/

    /*DEBUG MESSAGE SENDING*/

    /***************************************************************************/


    /*GETSIZE--return the size of a packed array stored with content of an unkown size*/
    inline int getSize(string content){

        /*will always return an integer*/
        return  content.length()+1+
            sizeof(utils::byte)+2*sizeof(api::message_type)+sizeof(size_t)
            +sizeof(int);
    }


    /*PACK--given the type encoding and content, pack the information into
     *a sendable messeage*/
    std::list<api::message_type*> pack(int msgEncode, string content){

        utils::byte* msg = (utils::byte*)new api::message_type[api::MAXLENGTH];
        int pos = 0;

        std::list<api::message_type*> msgList;

        api::message_type debugFlag =  DEBUG;
        size_t contentSize = content.length() + 1;
        size_t bufSize = api::MAXLENGTH*SIZE;//bytes
        api::message_type msgSize = bufSize-SIZE;
        utils::byte anotherIndicator = 0;
        api::message_type timeStamp = 0;
        api::message_type nodeId = api::getNodeID();

        int currentSize = 0;

        /*message size in bytes*/
        utils::pack<api::message_type>(&msgSize,1,msg,bufSize,&pos);

        /*debug indicator*/
        utils::pack<api::message_type>(&debugFlag,1,msg,bufSize,&pos);

        /*timestamp*/
        utils::pack<api::message_type>(&timeStamp,1,msg,bufSize,&pos);

        /*VM id*/
        utils::pack<api::message_type>(&nodeId,1,msg,bufSize,&pos);

        /*indicate if another message is coming*/
        utils::pack<utils::byte>(&anotherIndicator,1,msg,bufSize,&pos);

        /*debug command encoding*/
        utils::pack<int>(&msgEncode,1,msg,bufSize,&pos);

        /*size of content*/
        utils::pack<size_t>(&contentSize,1,msg,bufSize,&pos);

        /*content*/
        utils::pack<char>((char*)content.c_str(),content.size()+1,
                                 msg,bufSize,&pos);

        msgList.push_back((api::message_type*)msg);

        return msgList;

    }



    /*SENDMSG--the desination specified is the user input node ID*/
    /* send a debug message to another process through the API layer*/
    /* if send to all, specify BROADCAST (see top)*/
    void sendMsg(int destination, int msgType,
              string content, bool broadcast)  {

         /*length of array*/
            size_t msgSize = api::MAXLENGTH;
            /*pack the message*/
            std::list<api::message_type*> msgList = pack(msgType,content);
            api::message_type* msg;

            msg = msgList.front();
            msgList.pop_front();

            if (broadcast){

                /*send to all*/
                api::debugBroadcastMsg(msg,msgSize);

            } else {

                /*send to the master debugging process*/
                if (destination == MASTER){
                    api::debugSendMsg(MASTER,msg,
                                      msgSize);
                    return;
                }

                /*send the message through api layer*/
                api::debugSendMsg(destination,msg,
                                  msgSize);
            }

    }


    /*RECIEVEMSG--populate the queue of messages
     * from the mpi layer and handle them
     * until there are no more messages*/
    void receiveMsg(void){

        utils::byte *msg;
        int instruction;
        char specification[api::MAXLENGTH*SIZE];
        int pos = 0;
        api::message_type size,debugFlag,timeStamp,NodeId;
        size_t specSize;
        utils::byte anotherIndicator;

        /*load the message queue with messages*/
        api::debugGetMsgs();

        while(!messageQueue->empty()){
            /*process each message until empty*/
            /*extract the message*/
            msg = (utils::byte*)messageQueue->front();

            /*==>UNPACK the message into readable form*/
            /*msgsize in bytes*/
            utils::unpack<api::message_type>(msg,api::MAXLENGTH*SIZE,
                                             &pos,&size,1);
            /*debugFlag*/
            utils::unpack<api::message_type>(msg,api::MAXLENGTH*SIZE,
                                             &pos,&debugFlag,1);
            /*timestamp*/
            utils::unpack<api::message_type>(msg,api::MAXLENGTH*SIZE,
                                             &pos,&timeStamp,1);
            /*place msg came from*/
            utils::unpack<api::message_type>(msg,api::MAXLENGTH*SIZE,
                                             &pos,&NodeId,1);
            /*if another message is coming*/
            utils::unpack<utils::byte>(msg,api::MAXLENGTH*SIZE,
                                       &pos,&anotherIndicator,1);
            /*command encoding*/
            utils::unpack<int>(msg,api::MAXLENGTH*SIZE,&pos,&instruction,1);
            /*content size*/
            utils::unpack<size_t>(msg,api::MAXLENGTH*SIZE,&pos,&specSize,1);
            /*content*/
            utils::unpack<char>(msg,api::MAXLENGTH*SIZE,&pos,
                                &specification,specSize);

            string spec(specification);
			cout << "spec: " << specification << endl;
            /*if the controlling process is recieving a message*/
            if (isInMpiDebuggingMode()&&api::world->rank()==MASTER){

                debugMasterController(instruction,spec);
                if (!anotherIndicator)
                    numberExpected--;

            /*if a slave process (any vm) is receiving the message*/
            } else {

                debugController(instruction,spec);
            }

            /*set up the variables and buffers for next message*/
            memset(specification,0,api::MAXLENGTH*SIZE);
            messageQueue->pop();
            memset(msg,0,api::MAXLENGTH*SIZE);
            pos = 0;
        }
    }


}//namespace debugger















