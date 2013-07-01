/*
  The MPI communication API
  =========================
  @author: Xing Zhou
  @email: xingz@andrew.cmu.edu
  @date: 1 July 2013

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

// token tags to specify the token message being sent
#define BLACK 0
#define WHITE 1
#define DONE 2
#define TOKEN_OFFSET 3

    mpi::communicator *world;
    int world_sum;

    // Vectors to hold created messages in order to free them after the
    // asynchronous MPI calls are complete
    vector<pair<mpi::request, message_type *> > msgs;
    vector<pair<mpi::request, pair<int, message_type *> > > recv_q;

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

    void send_message(const db::node::node_id id,
                      const db::simple_tuple *stpl) {
        /* Given a node id and tuple, figure out the process id to send the
           tuple and id to be processed
           ================================================================
        */

        int dest = id % world->size();

        message_type *msg = new message_type[MAXLENGTH];
        size_t msg_length = MAXLENGTH * sizeof(message_type);
        int p = 0;

        stpl->pack((utils::byte *) msg, msg_length, &p);

        /* Dijkstra's Algorithm */
        // SEND MESSAGE

#ifdef DEBUG_MPI
        cout << "[" << world->rank() << "==>" << dest << "] "
             << "@" << id << " " << *stpl << endl;
#endif

        mpi::request req = world->isend(dest, id + TOKEN_OFFSET, *msg);

        // Store the message and request in order to free later
        msgs.push_back(make_pair(req, msg));
        free_msgs();
    };

    bool poll(sched::base *sched, vm::all *all) {
        /* Call the mpi asking for messages in the receive queue.  The messages
           are handled appropriately.
           ====================================================================

           Receive the message if there is a message waiting to be received,
           deserialize the message, and then add the message to the
           scheduler's queue as new work.

           Returns true if the system cannot be determined to have terminated.
           Otherwise, returns false if the system has terminated.
         */

        boost::optional<mpi::status> status_opt;
        mpi::status status;
        int q, token_color = -1;
        bool received_tag = false;

        if (world->rank() == 0 && !token_sent) {
            // Safra's Algorithm
            // Begin Token collection
            // Token should only be sent once, and retransmitted below
            world->isend(1, WHITE, 0);
            token_sent = true;
        }

        // Continuously ask MPI for messages in the queue.
        // The messages can be either a token or a meld message
        while ((status_opt = world->iprobe(mpi::any_source,
                                           mpi::any_tag))) {
            status = status_opt.get();

            if (status.tag() >= TOKEN_OFFSET) {
                /* a meld message that needs to be processed by scheduler
                   Since the MPI is asynchronous, the messages are queued and
                   then processed outside of the loop.
                */

                message_type *msg = new message_type[MAXLENGTH];

                mpi::request req = world->irecv(mpi::any_source, mpi::any_tag, *msg);

                recv_q.push_back(make_pair(req, make_pair(status.tag() - TOKEN_OFFSET, msg)));
                color = BLACK; // need to mark black even if message not processed
            } else {
                /* a token is received */
                world->irecv(mpi::any_source, mpi::any_tag, q);
                assert(token_color < 0); // Should only ever receive 1 token
                token_color = status.tag();
                assert(received_tag == false);
                received_tag = true;
            }
        }

        // Process the queued up message and free the messages if appropriate
        for (vector<pair<mpi::request, pair<int, message_type *> > >::iterator
                 it=recv_q.begin();
             it != recv_q.end(); ) {
            mpi::request req = it->first;

            if (req.test()) {
                // Communication is complete, can safely extract tuple

                int id = (it->second).first;
                message_type *msg = (it->second).second;
                size_t msg_length = MAXLENGTH * sizeof(message_type);
                int pos = 0;

                db::simple_tuple *stpl = db::simple_tuple::unpack(
                    (utils::byte *) msg, msg_length, &pos, all->PROGRAM);

                assert(id >= 0);

                // Let machine handle received tuple
                all->MACHINE->route(NULL, sched, id, stpl, 0);

#ifdef DEBUG_MPI
                cout << "{" << world->rank() << "<==" << status.source() << "} "
                     << "@" << id << " " << *stpl << endl;
#endif

                // Safra's Algorithm for RECEIVE MESSAGE
                counter--;
                color = BLACK;

                // Free the messages
                delete[] msg;
                it = recv_q.erase(it);
            } else
                ++it;
        }

        // Check the collected token
        if (received_tag) {
            int dest = (world->rank() + 1) % world->size();

            if (world->rank() == 0) {
                // The original process that sent the message => one ring round
                if (q + counter == 0 && status.tag() == WHITE && color == WHITE) {
                    // System Termination detected, notify other processes
                    assert(!sched->get_work());
                    assert(msgs.empty());
                    assert(recv_q.empty());
                    world->isend(dest, DONE, 0);
                    return false;
                } else {
                    /* Safra's Algorithm: RETRANSMIT TOKEN */
                    /* Reset token to white and accu to 0 */
                    world->isend(dest, WHITE, 0);
                    color = WHITE;
                }
            }

            else {
                if (status.tag() == DONE) {
                    // Propagate done message to following processes
                    assert(q == 0);
                    assert(color == WHITE);
                    assert(msgs.empty());
                    assert(recv_q.empty());
                    assert(!sched->get_work());
                    world->isend(dest, DONE, q);
                    return false;
                }
                /* PROPAGATE TOKEN */
                if (color == BLACK)
                    token_color = BLACK;
                color = WHITE;
                world->isend(dest, token_color, q + counter);
            }
        }
        return true;
    }

    bool on_current_process(const db::node::node_id id) {
        /* Test whether or not the current process should handle the node id */
        return (id % world->size()) == world->rank();
    }

    void init(int argc, char **argv) {}

} /* namespace api */
