/*
  The MPI communication API
  =========================
  @author: Xing Zhou
  @email: xingz@andrew.cmu.edu
  @date: 12 July 2013

  This is the MPI implementation of the API interface (api/api.hpp) using
  boost::mpi (v.1.53.0). Each process with id `x` is responsible for handling
  the nodes with ids `i % n == x`, where n is the size of the MPI world.

  There is a barrier after machine invocation (db/database.cpp) to synchronize
  message passing, and the messages are sent from `machine::route`
  (process/machine.cpp) and are received in the `do_loop` of the scheduler
  implementation (currently in `sched/serial.cpp`).  These functions call
  api::send_message and api::poll respectively.

  The most difficult issue with the MPI implemention is termination detection,
  in order to safely terminate each process.  The current functional
  implementation uses Safra's termination algorithm for a token ring. While this
  works, we feel that the algorithm is still too communication intensive, and a
  more lightweight algorithm would be good.
*/

#include "api/api.hpp"
#include "utils/types.hpp"
#include "utils/serialization.hpp"
#include "db/database.hpp"
#include "vm/predicate.hpp"
#include "process/work.hpp"
#include "process/machine.hpp"
#include <vector>
#include <utility>

using namespace std;
namespace mpi = boost::mpi;

namespace api {
//#define DEBUG_MPI
//#define DEBUG_MPI_TERM

    const int MASTER = 0;

    // Function Prototypes
    static void free_msgs(void);
    static void processRecvMsgs(sched::base *sched, vm::all *all);
    static bool detectTerm(sched::base *sched);

	// token tags to specify the token message being sent
    enum { DEBUG,
           TERM,
           BLACK,
           WHITE,
           DONE,
           TUPLE};

    mpi::environment *env;
    mpi::communicator *world;

    // Vectors to hold created messages in order to free them after the
    // asynchronous MPI calls are complete
    vector<pair<mpi::request, message_type *> > msgs;
    vector<pair<mpi::request, pair<mpi::status, message_type *> > > recv_q;

    // Dijkstra and Safra's token ring termination detection algorithm
    // Default states
    int counter = 0;
    int color = WHITE;
    bool token_sent = false;

    void free_msgs(void) {
        /* free the messags in the msgs vector if appropriate since the mpi
           ================================================================

           functions are asynchronous, we cannot assume that the messages buffer
           created can be removed as soon as the sents calls are executed.

           Instead, the messages are queued up in the msgs vector along with
           their mpi status. Then free_msgs are called periodically in order to
           free the messages where their mpi status indicates as complete.
        */

        for (vector<pair<mpi::request, message_type*> >::iterator it = msgs.begin();
             it != msgs.end(); ) {
            mpi::request req(it->first);
            if (req.test()) {
                // Completed Transmission, save to delete message
                delete[] (it->second);
                it = msgs.erase(it);

                // Safra's algorithm
                counter++;
            } else
                ++it;
        }
    }

    void send_message(const db::node* from, const db::node::node_id to,
                      db::simple_tuple* stpl) {
        /* Given a node id and tuple, figure out the process id to send the
           tuple and id to be processed
           ================================================================
        */

        int dest = get_process_id(to);

        message_type *msg = new message_type[MAXLENGTH];
        size_t msg_length = MAXLENGTH * sizeof(message_type);
        int p = 0;

        utils::pack<message_type>((void *) &to, 1, (utils::byte *) msg,
                                  msg_length, &p);

        stpl->pack((utils::byte *) msg, msg_length - sizeof(message_type), &p);

        /* Dijkstra's Algorithm */
        // SEND MESSAGE

#ifdef DEBUG_MPI
        cout << "[" << world->rank() << "==>" << dest << "] "
             << "@" << to << " " << *stpl << endl;
#endif

        mpi::request req = world->isend(dest, TUPLE, msg, MAXLENGTH);

        // Store the message and request in order to free later
        msgs.push_back(make_pair(req, msg));
        free_msgs();
    };

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

        free_msgs();

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

            recv_q.push_back(make_pair(req, make_pair(status, msg)));
        }

        processRecvMsgs(sched, all);

        if (recv_q.empty()) {
            if (world->rank() == MASTER && !token_sent) {
                /* Safra's Algorithm: Begin Token Collection, when MASTER is idle */
                world->isend(get_process_id(world->rank() + 1), WHITE, 0);
                token_sent = true;
#ifdef DEBUG_MPI_TERM
                cout << "Safra's" << endl;
#endif
            }

            return detectTerm(sched);
        }

        return true;
    }

    void processRecvMsgs(sched::base *sched, vm::all *all) {
        /*
          Process the receive message queue and free the messages appropriatedly
        */

        // Iterate through the receive queue and test whether or not the MPI
        // request is complete.  If it is, the message is unpacked and
        // processed, and then freed.
        for (vector<pair<mpi::request, pair<mpi::status, message_type *> > >::iterator
                 it=recv_q.begin();
             it != recv_q.end(); ) {

            mpi::request req = it->first;

            if (req.test()) {
                // Communication is complete, can safely extract tuple

#ifdef DEBUG_MPI
                mpi::status status = (it->second).first;
#endif
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
                cout << "{" << world->rank() << "<==" << status.source() << "} "
                     << "@" << id << " " << *stpl << endl;
#endif

                assert(on_current_process(id));

                // Let machine handle received tuple
                all->MACHINE->route(NULL, sched, id, stpl, 0);

                // Safra's Algorithm for RECEIVE MESSAGE
                counter--;
                color = BLACK;

                // Free the messages
                delete[] msg;
                it = recv_q.erase(it);
            } else
                ++it;
        }
    }

    bool detectTerm(sched::base *sched) {
        /*
          Handle tokens sent using Safra's algorithm
        */

        bool done = false;
        uint dest = get_process_id(world->rank() + 1);

        /* Safra's Algorithm: token accumulator */
        int acc, tokenColor;

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
            return true;
        }

#ifdef DEBUG_MPI_TERM
        if (!done)
            assert(tokenColor == WHITE || tokenColor == BLACK);

        cout << world->rank() << " [" << acc << "] " <<
            (tokenColor == WHITE ? "WHITE" : "BLACK") << endl;
#endif

        if (world->rank() == MASTER) {
            if (tokenColor == WHITE && color == WHITE && acc + counter == 0) {
                // System Termination detected, notify other processes
#ifdef DEBUG_MPI_TERM
                cout << "DONE" << endl;

                assert(!sched->get_work());
                assert(msgs.empty());
                assert(recv_q.empty());
#endif
                world->isend(dest, DONE);
                return false;
            }

            /* Safra's Algorithm: RETRANSMIT TOKEN */
            world->isend(dest, WHITE, 0);

#ifdef DEBUG_MPI_TERM
            cout << world->rank() << " > RETRANSMIT" << endl;
#endif

            color = WHITE;
        }

        /* Not MASTER Process, which originates the token */
        else {
            if (done) {
                world->isend(dest, DONE);
                return false;
            }

            /* Safra's Algorithm: PROPAGATE TOKEN */
            if (color == BLACK)
                tokenColor = BLACK;

            color = WHITE;
            world->isend(dest, tokenColor, acc + counter);

#ifdef DEBUG_MPI_TERM
            cout << world->rank() << " > " << dest <<
                (tokenColor == WHITE ? " WHITE" : " BLACK") << endl;
#endif
        }

        return true;
    }

    bool on_current_process(const db::node::node_id id) {
        return get_process_id(id) == world->rank();
    }

    int get_process_id(const db::node::node_id id) {
        return id % world->size();
    }

    void init(int argc, char **argv, sched::base *sched) {
        if (sched == NULL) {
            env = new mpi::environment(argc, argv);
            world = new mpi::communicator();
        }
    }

    void end(void) {
        delete env;
        delete world;
    }

    void debugSendMsg(const db::node::node_id dest, message_type *msg,
                      size_t msgSize, bool bcast) {

        int pid = get_process_id(dest);
        mpi::request req = world->isend(pid, DEBUG, msg, msgSize);

        msgs.push_back(make_pair(req, msg));
        free_msgs();
    }

    void debugGetMsgs(void) {

    }

    /*
     * Unimplemented functions in MPI
     */
    void set_color(db::node *n, const int r, const int g, const int b) {}

} /* namespace api */
