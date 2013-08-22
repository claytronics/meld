/*
  The MPI communication API
  =========================
  @author: Xing Zhou
  @email: xingz@andrew.cmu.edu
  @date: 13 July 2013

  @library: boost::mpi v.1.54.0

  All nodes in the database is partitioned to each VM according to specific
  policy.

  Ensemble termination uses Dijkstra and Safra's token ring termination
  detection.
*/

#include <vector>
#include <utility>
#include <boost/mpi/collectives.hpp>
#include <sstream>
#include "utils/types.hpp"
#include "vm/predicate.hpp"
#include "process/machine.hpp"
#include "debug/debug_handler.hpp"
#include "debug/debug_prompt.hpp"
#include "utils/serialization.hpp"
#include "api/api.hpp"
#include "debug/debug_handler.hpp"

#define RANK 0
using namespace std;
//namespace mpi = boost::mpi;

namespace api {

    // Function Prototypes
    static void freeSendMsgs(void);
    static void processRecvMsgs(sched::base *sched, vm::all *all);
    static string _printNode(const db::node::node_id, const db::database::map_nodes&);
    static string _dumpNode(const db::node::node_id, const db::database::map_nodes&);
    static void serialDBOutput(std::ostream &out, const db::database::map_nodes &nodes,
                               string (*format)(
                                   const db::node::node_id,
                                   const db::database::map_nodes&
                                   ));

    static void freeDebugSendMsgs(void);

     // token tags to tag messages in MPI
    enum tokens {
        DEBUG,
        TERM,
        BLACK,
        WHITE,
        DONE,
        TUPLE,
        EXEC,
        INIT,
        PRINT,
        PRINT_DONE
    };


    static int RING_ORIGIN = 0;

    /* Functions to calculate adjacent process ids in a ring structure */
    inline int prevProcess(void) {

        return 0;


    }

    inline int nextProcess(void) {
        return 0;
    }

    struct request{
        bool test;
    };

    struct message_container{
        int token;
        message_type* msg;
    };

    std::list<struct message_container*>* msgList;

    struct request *send(int destination, int token,message_type* msg, int size){
        struct message_container* cont = new struct message_container;
        cont->token = token;
        cont->msg = msg;
        msgList->push_back(cont);
        struct request* req = new struct request;
        req->test = true;
        return req;
    }

    struct request *recv(int from, int token, message_type* msg, int size){

            std::list<struct message_container*>::iterator it;
            struct message_container* container;
            for (it = msgList->begin(); it!= msgList->end(); it++){
                container = *it;
                if (container->token == token){
                    memcpy(msg,container->msg,size*8);
                    msgList->remove(container);
                    delete container->msg;
                    delete container;
                    struct request* req = new struct request;
                    req->test = true;
                    return req;
                }
            }
            return NULL;
    }

    bool probe(int from, int token){
        (void)from;
        std::list<struct message_container*>::iterator it;
        struct message_container* container;
            for (it = msgList->begin(); it!= msgList->end(); it++){
                container = *it;
                if (container->token == token){
                    return true;
                }
            }
            return false;
    }


    // Global MPI variables
    //mpi::communicator *world;
    //static mpi::environment *env;

    // Vectors to hold created messages in order to free them after the
    // asynchronous MPI calls are complete
    static vector<pair<struct request*, message_type *> > sendMsgs, recvMsgs,
        debugSendMsgs, debugRecvMsgs;



    // Dijkstra and Safra's token ring termination detection algorithm
    // Default states
    static int counter = 0;
    static int color = WHITE;
    static bool tokenSent = false;
    bool hasToken = false;

    void init(int argc, char **argv, sched::base *sched) {
        /* Initialize the MPI.

           In order to accomodate the bbsimapi implementation, init needs to be
           called multiples times. The value of sched, whether NULL or non-NULL
           works as a switch so that the MPI init is only run once at most.
        */
        if (sched == NULL) {
            //env = new mpi::environment(argc, argv);
            //world = new mpi::communicator();
            if (debugger::isInMpiDebuggingMode()) {
                /*api master defined in debug_handler.hpp*/
                RING_ORIGIN = api::MASTER;
            }
            if (RANK == RING_ORIGIN)
                hasToken = true;
        }
        msgList = new std::list<message_container*>();
    }

    void end(void) {
        delete msgList;
        //delete env;
        //delete world;
    }

    void sendMessage(const db::node* from, const db::node::node_id to,
                      db::simple_tuple* stpl) {
        /* Given a node id and tuple, figure out the process id to send the
           tuple and id to be processed
        */
        int dest = getVMId(to);
        message_type *msg = new message_type[MAXLENGTH];
        size_t msg_length = MAXLENGTH * sizeof(message_type);
        int p = 0;

        utils::pack<message_type>((void *) &to, 1, (utils::byte *) msg,
                                  msg_length, &p);
        stpl->pack((utils::byte *) msg, msg_length - sizeof(message_type), &p);

        struct request *req = send(dest, TUPLE, msg, MAXLENGTH);
        sendMsgs.push_back(make_pair(req, msg));
        freeSendMsgs();
    }

    void freeSendMsgs(void) {
        /* free messages in the send messages collection if appropriate
           ================================================================

           since the mpi functions are asynchronous, we cannot assume that the
           messages buffer created can be removed as soon as the sents calls are
           executed.

           Instead, the messages are queued up in the sendMsgs vector along with
           their mpi status. Then this function is called periodically in order
           to free the messages where their mpi status indicates as complete.
        */
        for (vector<pair<struct request *, message_type*> >::iterator it = sendMsgs.begin();
             it != sendMsgs.end(); ) {
            struct request *req = it->first;
            if (req->test) {
                delete[] (it->second);
                it = sendMsgs.erase(it);

                delete req;
                // Safra's algorithm: SEND MESSAGE
                counter++;
            } else
                ++it;
        }
    }

    bool pollAndProcess(sched::base *sched, vm::all *all) {
        /* Call the mpi asking for messages in the receive queue.  The messages
           are handled appropriately.
           ====================================================================

           Receive the message if there is a message waiting to be received,
           deserialize the message, and then add the message to the
           scheduler's queue as new work.

           Returns true if the system cannot be determined to have terminated.
           Otherwise, returns false if the system has terminated.
        */
        freeSendMsgs();

        bool newMessage = false;

        while (probe(0, TUPLE)) {
            /* a meld message that needs to be processed by scheduler
               Since thec MPI is asynchronous, the messages are queued and
               then processed outside of the loop.
            */
            message_type *msg = new message_type[MAXLENGTH];
            struct request *req = recv(0, TUPLE,
                                            msg, MAXLENGTH);
            if (req == NULL){
                delete msg;
            } else {
                recvMsgs.push_back(make_pair(req, msg));
                newMessage = true;
            }
        }

        processRecvMsgs(sched, all);
        return newMessage;
    }

    void processRecvMsgs(sched::base *sched, vm::all *all) {
        /*
          Process the receive message queue and free the messages appropriatedly
        */
        for (vector<pair<struct request*, message_type *> > ::iterator
                 it=recvMsgs.begin(); it != recvMsgs.end(); ) {
            struct request *req = it->first;
            if (req->test) {
                delete req;
                // Communication is complete, can safely extract tuple
                message_type *msg = it->second;
                size_t msg_length = MAXLENGTH * sizeof(message_type);
                int pos = 0;
                message_type nodeID;

                utils::unpack<message_type>((utils::byte *) msg, msg_length,
                                            &pos, &nodeID, 1);
                db::node::node_id id = nodeID;

                db::simple_tuple *stpl = db::simple_tuple::unpack(
                    (utils::byte *) msg, msg_length - sizeof(message_type),
                    &pos, all->PROGRAM);

                all->MACHINE->route(NULL, sched, id, stpl, 0);

                // Safra's Algorithm for RECEIVE MESSAGE
                counter--;
                color = BLACK;

                // Free the messages
                delete[] msg;
                it = recvMsgs.erase(it);
            } else
                ++it;
        }
    }

    bool ensembleFinished(sched::base *sched) {
        /*
          Handle tokens sent using Safra's algorithm for termination detection
          ref: http://www.cse.ohio-state.edu/siefast/group/publications/da2000-otdar.pdf
          page: 3
        */
        return msgList->empty();
    }

    void dumpDB(std::ostream &out, const db::database::map_nodes &nodes){}
    void printDB(std::ostream &out, const db::database::map_nodes &nodes){}

    void debugWaitMsg(){}
    void debugInit(vm::all* all){}

    int getNodeID(void){
        return 0;
    }

    bool onLocalVM(const db::node::node_id id) {
        return getVMId(id) == 0;
    }

    int getVMId(const db::node::node_id id) {
        return 0;
    }


/*
 * Unimplemented functions in MPI
 */
    void set_color(db::node *n, const int r, const int g, const int b) {}

} /* namespace api */
