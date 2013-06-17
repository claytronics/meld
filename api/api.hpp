/*
 * Header file for API
 */
#include "db/tuple.hpp"
#include "db/node.hpp"
#include "sched/base.hpp"

using namespace std;

namespace api {
    /* Type representing the message between interprocess communications */
    typedef uint64_t message_type;

    /* Given a node destination, compute the process id that the node
     * belongs to, serialize the data for MPI and send the data
     */
    void send(const db::node::node_id id, db::simple_tuple* tuple);

    /* Check for any pending messages waiting to be received and add all of
     * the pending messages to the scheduler queue
     */
    void pool(const sched::base *sched);

    /* Return whether or not the node with id `id` belongs to the current
     * process
     */
    bool on_current_process(const db::node::node_id id);

    /*
     * Initialize the API layer
     */
    void init();
} // namespace api
