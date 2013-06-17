#ifndef API_H_
#define API_H_

/*
 * Header file for API
 */
#include "db/tuple.hpp"
#include "db/node.hpp"
#include "sched/base.hpp"
#include <boost/mpi.hpp>

using namespace std;

namespace api {
    /* Type representing the message between interprocess communications */
    typedef uint64_t message_type;
    extern boost::mpi::communicator world;

    /* Given a node destination, compute the process id that the node
     * belongs to, serialize the data for MPI and send the data
     */
    void send_message(const db::node::node_id id, message_type *msg);

    message_type *create_message();

    /* Check for any pending messages waiting to be received and add all of
     * the pending messages to the scheduler queue
     */
    void poll(const sched::base *sched);

    /* Return whether or not the node with id `id` belongs to the current
     * process
     */
    bool on_current_process(const db::node::node_id id);

    /*
     * Initialize the API layer
     */
    void init(int argc, char **argv);
} // namespace api

#endif
