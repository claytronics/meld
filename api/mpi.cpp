/*
  The MPI communication API
  =========================
  @author: Xing Zhou
  @email: xingz@andrew.cmu.edu
  @date: 13 July 2013

  This is the MPI implementation of the API interface (api/api.hpp) using
  boost::mpi (v.1.54.0). Each process with id `x` is responsible for handling
  the nodes with ids `i % n == x`, where n is the size of the MPI world.

  There is a barrier after machine invocation (db/database.cpp) to synchronize
  message passing, and the messages are sent from `machine::route`
  (process/machine.cpp) and are received in the `do_loop` of the scheduler
  implementation (currently in `sched/serial.cpp`).  These functions call
  api::send_message and api::poll respectively.

  The most difficult issue with the MPI implemention is termination detection,
  in order to safely terminate each process.  The current functional
  implementation uses Safra's termination algorithm for a token ring. While this
  works, we feel that the algorithm is still too costly due to excessive message
  passing, and a more lightweight algorithm would be good.
*/

#include <vector>
#include <utility>
#include <boost/mpi/collectives.hpp>
#include "utils/types.hpp"
#include "db/database.hpp"
#include "vm/predicate.hpp"
#include "process/work.hpp"
#include "process/machine.hpp"
#include "debug/debug_handler.hpp"
#include "debug/debug_prompt.hpp"
#include "utils/serialization.hpp"
#include "api/api.hpp"

using namespace std;
namespace mpi = boost::mpi;

namespace api {
//#define DEBUG_MPI
//#define DEBUG_MPI_TERM
//#define DEBUG_MPI_SERIAL

    const int MASTER = 0;

    // Function Prototypes
    static void freeSendMsgs(void);
    static void processRecvMsgs(sched::base *sched, vm::all *all);

    /* Functions to calculate adjacent process ids in a ring structure */
    inline int prevProcess(void) {
        if (world->rank() == 0) {
            return world->size() - 1;
        } else {
            return (world->rank() - 1) % world->size();
        }
    }

    inline int nextProcess(void) {
        return (world->rank() + 1) % world->size();
    }

	// token tags to tag messages in MPI
    enum tokens {
        DEBUG,
        TERM,
        BLACK,
        WHITE,
        DONE,
        TUPLE,
        EXEC,
        INIT
    };

    // Global MPI variables
    mpi::environment *env;
    mpi::communicator *world;

    // Vectors to hold created messages in order to free them after the
    // asynchronous MPI calls are complete
    vector<pair<mpi::request, message_type *> > sendMsgs, debugMsgs, debugRecvMsgs;

    // recvQ stores the `status` only for DEBUGGING purposes.
    vector<pair<mpi::request, pair<mpi::status, message_type *> > > recvQ;


    // Dijkstra and Safra's token ring termination detection algorithm
    // Default states
    int counter = 0;
    int color = WHITE;
    bool token_sent = false;
    bool hasToken = false;

    void init(int argc, char **argv, sched::base *sched) {
        /* Initialize the MPI.

           In order to accomodate the bbsimapi implementation, init needs to be
           called multiples times. The value of sched, whether NULL or non-NULL
           works as a switch so that the MPI init is only run once at most.
        */

        if (sched == NULL) {
            env = new mpi::environment(argc, argv);
            world = new mpi::communicator();

            if (world->rank() == MASTER)
                hasToken = true;
        }
    }

    void end(void) {
#ifdef DEBUG_MPI
        printf("%d MPI END\n", world->rank());
#endif

        delete env;
        delete world;
    }

    void send_message(const db::node* from, const db::node::node_id to,
                      db::simple_tuple* stpl) {
        /* Given a node id and tuple, figure out the process id to send the
           tuple and id to be processed
           ================================================================
        */

        int dest = getVMId(to);

        message_type *msg = new message_type[MAXLENGTH];
        size_t msg_length = MAXLENGTH * sizeof(message_type);
        int p = 0;

        // Pack node id
        utils::pack<message_type>((void *) &to, 1, (utils::byte *) msg,
                                  msg_length, &p);

        stpl->pack((utils::byte *) msg, msg_length - sizeof(message_type), &p);

#ifdef DEBUG_MPI
        printf("[%d==>%d] @%d %s\n", world->rank(), dest, to,
               (ostringstream() << *stpl).str())
#endif

        mpi::request req = world->isend(dest, TUPLE, msg, MAXLENGTH);

        // Store the message and request in order to free later
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

        for (vector<pair<mpi::request, message_type*> >::iterator it = sendMsgs.begin();
             it != sendMsgs.end(); ) {
            mpi::request req(it->first);
            if (req.test()) {
                // Completed Transmission, save to delete message
                delete[] (it->second);
                it = sendMsgs.erase(it);

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
        boost::optional<mpi::status> statusOpt;

        while ((statusOpt = world->iprobe(mpi::any_source, TUPLE))) {
            /* a meld message that needs to be processed by scheduler
               Since the MPI is asynchronous, the messages are queued and
               then processed outside of the loop.
            */

            mpi::status status = statusOpt.get();
            message_type *msg = new message_type[MAXLENGTH];

            mpi::request req = world->irecv(mpi::any_source, TUPLE,
                                            msg, MAXLENGTH);

            recvQ.push_back(make_pair(req, make_pair(status, msg)));
            newMessage = true;
        }

        processRecvMsgs(sched, all);
        return newMessage;
    }

    void processRecvMsgs(sched::base *sched, vm::all *all) {
        /*
          Process the receive message queue and free the messages appropriatedly
        */

        for (vector<pair<mpi::request, pair<mpi::status, message_type *> > >::iterator
                 it=recvQ.begin();
             it != recvQ.end(); ) {

            // Iterate through the receive queue and test whether or not the MPI
            // request is complete.  If it is, the message is unpacked and
            // processed, and then freed.

            mpi::request req = it->first;

            if (req.test()) {
                // Communication is complete, can safely extract tuple
                message_type *msg = (it->second).second;

                size_t msg_length = MAXLENGTH * sizeof(message_type);
                int pos = 0;
                message_type nodeID;

                // Extract node id
                utils::unpack<message_type>((utils::byte *) msg, msg_length,
                                            &pos, &nodeID, 1);

                db::node::node_id id = nodeID;

                // Extract the tuple
                db::simple_tuple *stpl = db::simple_tuple::unpack(
                    (utils::byte *) msg, msg_length - sizeof(message_type),
                    &pos, all->PROGRAM);

#ifdef DEBUG_MPI
                mpi::status status = (it->second).first;

                printf("{%d<==%d} @%d %s\n", world->rank(), status.source(), id,
                       (ostringstream() << *stpl).str());
#endif

                assert(onLocalVM(id));

                // Let machine handle received tuple
                all->MACHINE->route(NULL, sched, id, stpl, 0);

                // Safra's Algorithm for RECEIVE MESSAGE
                counter--;
                color = BLACK;

                // Free the messages
                delete[] msg;
                it = recvQ.erase(it);
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

        uint dest = nextProcess();

        if (world->rank() == MASTER && !token_sent) {
            /* Safra's Algorithm: Begin Token Collection  */
            world->isend(dest, WHITE, 0);
            token_sent = true;
#ifdef DEBUG_MPI_TERM
            printf("Safra's\n");
#endif
            return false;
        }

        /* Safra's Algorithm: token accumulator */
        int acc, tokenColor;
        bool done = false;

        // Receive possible tokens: WHITE, BLACK, or DONE
        if (world->iprobe(mpi::any_source, WHITE)) {
            world->irecv(mpi::any_source, WHITE, acc);
            tokenColor = WHITE;

        } else if (world->iprobe(mpi::any_source, BLACK)) {
            world->irecv(mpi::any_source, BLACK, acc);
            tokenColor = BLACK;

        } else if (world->iprobe(mpi::any_source, DONE)) {
            world->irecv(mpi::any_source, DONE);
            done = true;

        } else {
            // No token in progress
            return false;
        }

#ifdef DEBUG_MPI_TERM
        if (!done)
            assert(tokenColor == WHITE || tokenColor == BLACK);

        printf("%d [%d] %s\n", world->rank(), acc,
               (tokenColor == WHITE ? "WHITE" : "BLACK"));
#endif

        if (world->rank() == MASTER) {
            if (tokenColor == WHITE && color == WHITE && acc + counter == 0) {
                // System Termination detected, notify other processes
#ifdef DEBUG_MPI_TERM
                assert(!sched->get_work());
                assert(sendMsgs.empty());
                assert(recvQ.empty());

                printf("%d TERMINATED\n", world->rank());
#endif

                world->isend(dest, DONE);
                return true;
            }

            /* Safra's Algorithm: RETRANSMIT TOKEN */
            world->isend(dest, WHITE, 0);

#ifdef DEBUG_MPI_TERM
            printf("%d RETRANSMIT\n", world->rank());
#endif

            color = WHITE;
        }

        /* Not MASTER Process */
        else {
            if (done) {
#ifdef DEBUG_MPI_TERM
                printf("%d TERMINATED\n", world->rank());
#endif
                world->isend(dest, DONE);
                return true;
            }

            /* Safra's Algorithm: PROPAGATE TOKEN */
            if (color == BLACK)
                tokenColor = BLACK;

            color = WHITE;
            world->isend(dest, tokenColor, acc + counter);

#ifdef DEBUG_MPI_TERM
            printf("%d > %d %s\n", world->rank(), dest,
                   (tokenColor == WHITE ? " WHITE" : " BLACK"));
#endif
        }

        return false;
    }

    void serializeBeginExec(void) {
        /*
          Use a token ring algorithm to serializeExecution from process to
          process

          There should be only 1 EXEC token passed around the ring. Only the
          process holding the token can execute, every other process must wait.
        */

        int source = prevProcess();

        if (!hasToken) {
            // Block process from continuing until token is received
            world->recv(source, EXEC);
            hasToken = true;
#ifdef DEBUG_MPI_SERIAL
            printf("[%d] processing ...\n", world->rank());
#endif
        }
    }

    void serializeEndExec(void) {
        int dest = nextProcess();

        if (hasToken) {
#ifdef DEBUG_MPI_SERIAL
            printf("[%d] end\n", world->rank());
            fflush(stdout);
#endif
            hasToken = false;
            world->send(dest, EXEC);
        }
    }

    bool onLocalVM(const db::node::node_id id) {
        return getVMId(id) == world->rank();
    }

    int getVMId(const db::node::node_id id) {
        /* POLICY SPECIFIC according to the value of debugger::MASTER */
        if (debugger::isInMpiDebuggingMode())
            return (id % (world->size() - 1)) + 1;

        return id % world->size();
    }


    /* === Debugger Functions === */

    void debugInit(void) {
        /* Use MPI gather to make sure that all VMs are initiated before the
         * debugger is run
         */

        if (world->size() == 1) {
            throw "Debug must be run with at least 2 MPI processes.";
        }

        if (world->rank() == debugger::MASTER) {
            std::vector<tokens> allVMs;
            mpi::gather(*world, INIT, allVMs, debugger::MASTER);

            debugger::run(NULL);
        } else {
            mpi::gather(*world, INIT, debugger::MASTER);
        }
    }

    void debugSendMsg(const db::node::node_id dest, message_type *msg,
                      size_t msgSize, bool bcast) {
        /* Send the message through MPI and place the message and status into
           the sendMsgs vector to be freed when the request completes */

        int pid = getVMId(dest);
        mpi::request req = world->isend(pid, DEBUG, msg, msgSize);

        sendMsgs.push_back(make_pair(req, msg));
        freeSendMsgs();
    }

    void debugGetMsgs(void) {
        /* Poll the MPI to find any debug messages and populate the debug
         * message queue */

        while(world->iprobe(mpi::any_source, DEBUG)) {
            message_type *msg = new message_type[MAXLENGTH];

            mpi::request req = world->irecv(mpi::any_source, DEBUG, msg, MAXLENGTH);

            debugRecvMsgs.push_back(make_pair(req, msg));
        }

        // Iterate through the message queue and add the messages with completed
        // requests to the debugger's message queue
        for (vector<pair<mpi::request, message_type*> >::iterator it = debugRecvMsgs.begin();
             it != debugRecvMsgs.end(); ++it) {
            mpi::request req(it->first);

            if (req.test()) {
                debugger::messageQueue->push(it->second);
            }
        }
    }

/*
 * Unimplemented functions in MPI
 */
    void set_color(db::node *n, const int r, const int g, const int b) {}

} /* namespace api */
