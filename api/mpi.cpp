/*
 * The MPI communication API
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

namespace api {
    boost::mpi::communicator *world;
    int world_sum;
    vector<pair<boost::mpi::request, message_type *> > msgs;
    vector<pair<boost::mpi::request, pair<int, message_type *> > > recv_q;

    /* Dijkstra & Safra algorithm */
    int c = 0;
    int BLACK = 0, WHITE = 1, DONE = 2;
    int color = WHITE;
    bool token_sent = false;

//#define DEBUG_MPI
#define TOKEN_OFFSET 3

    void free_msgs(void) {
        for (vector<pair<boost::mpi::request, message_type*> >::iterator it = msgs.begin();
             it != msgs.end(); ) {
            boost::mpi::request req(it->first);
            if (req.test()) {
                // Completed Transmission, save to delete message
                delete[] (it->second);
                it = msgs.erase(it);
                c++;
            } else
                ++it;
        }
    }

    /* Send data to the dest */
    void send_message(const db::node::node_id id,
                      const db::simple_tuple *stpl) {
        int dest = id % world->size();

        message_type *msg = new message_type[MAXLENGTH];
        size_t msg_length = MAXLENGTH * sizeof(message_type);
        int p = 0;
        boost::mpi::request req;

        stpl->pack((utils::byte *) msg, msg_length, &p);

        /* Dijkstra's Algorithm */
        // SEND MESSAGE

#ifdef DEBUG_MPI
        cout << "[" << world->rank() << "==>" << dest << "] "
             << "@" << id << " " << *stpl << endl;
#endif

        req = world->isend(dest, id + TOKEN_OFFSET, *msg);

        msgs.push_back(make_pair(req, msg));
        free_msgs();
    };

    bool poll(sched::base *sched, vm::all *all) {
        /* Receive the message if there is a message waiting to be received,
         * deserialize the message, and then add the message to the
         * scheduler's queue as new work
         */
        boost::optional<boost::mpi::status> status_opt;
        boost::mpi::status status;
        int q, token_color = -1;
        bool received_tag = false;

        if (world->rank() == 0 && !token_sent) {
            // Begin Token collection
            // Token should only be sent once, and retransmitted below
            world->isend(1, WHITE, 0);
            token_sent = true;
        }

        while ((status_opt = world->iprobe(boost::mpi::any_source,
                                           boost::mpi::any_tag))) {
            status = status_opt.get();

            if (status.tag() >= TOKEN_OFFSET) {
                /* a tuple message that needs to be processed by scheduler */
                /* Since the messages are received async, we need to queue up
                   the messages and process them outside of the while loop.
                */

                message_type *msg = new message_type[MAXLENGTH];

                boost::mpi::request req;
                req = world->irecv(boost::mpi::any_source, boost::mpi::any_tag, *msg);

                recv_q.push_back(make_pair(req, make_pair(status.tag() - TOKEN_OFFSET, msg)));
                color = BLACK; // need to mark black even if message not processed
            } else {
                world->irecv(boost::mpi::any_source, boost::mpi::any_tag, q);
                assert(token_color < 0); // Should only ever receive 1 token
                token_color = status.tag();
                assert(received_tag == false);
                received_tag = true;
            }
        }

        /* Process the queued up messages */
        for (vector<pair<boost::mpi::request, pair<int, message_type *> > >::iterator
                 it=recv_q.begin();
             it != recv_q.end(); ) {
            boost::mpi::request req = it->first;
            if (req.test()) {
                // Communication is complete
                /* Extract tuple from message */
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
                c--;
                color = BLACK;

                delete[] msg;
                it = recv_q.erase(it);
            } else
                ++it;
        }

        if (received_tag) {
            int dest = (world->rank() + 1) % world->size();

            if (world->rank() == 0) { // The initializing process
                if (q + c == 0 && status.tag() == WHITE && color == WHITE) {
                    // System Termination detected, notify other processes
                    assert(!sched->get_work());
                    assert(msgs.empty());
                    assert(recv_q.empty());
                    world->isend(dest, DONE, 0);
                    return false;
                } else {
                    /* RETRANSMIT TOKEN */
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
                world->isend(dest, token_color, q + c);
            }
        }
        return true;
    }

    message_type *create_message(const db::simple_tuple *tuple) {
        /* create a message by serializing the tuple */
        message_type *msg = new message_type[MAXLENGTH];
        int pos = 0;

        tuple->pack((utils::byte *)msg, MAXLENGTH, &pos);
        return msg;
    }

    bool on_current_process(const db::node::node_id id) {
        return (id % world->size()) == world->rank();
    }

    void init(int argc, char **argv) {
    }

} /* namespace api */
