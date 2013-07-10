#ifndef API_H_
#define API_H_

/*
 * Header file for API
 */
#include "db/tuple.hpp"
#include "db/node.hpp"
#include "sched/base.hpp"
#include <boost/mpi.hpp>
#include <boost/serialization/binary_object.hpp>
#include "vm/all.hpp"
#include <boost/asio.hpp>
#include "utils/types.hpp"
#include "queue/safe_general_pqueue.hpp"

 namespace api {
    /* Type representing the message between interprocess communications */
 	typedef uint64_t message_type;
 	static const size_t MAXLENGTH = 512 / sizeof(message_type);
 	extern boost::mpi::communicator *world;
 	extern boost::asio::ip::tcp::socket *tcp_socket;
 	extern bool reset_token;

    /* Given a node destination, compute the process id that the node
     * belongs to, serialize the data for MPI and send the data
     */
   // extern void send_message(const db::node::node_id id, const db::simple_tuple *stpl);
     extern message_type *create_message(const db::simple_tuple *tuple);


    /* Check for any pending messages waiting to be received and add all of
     * the pending messages to the scheduler queue
     */
     extern bool pollAndProcess(sched::base *sched, vm::all *all);
     
	 /*
     * Initialize the API layer
     */
     void init(sched::base *schedular);
     extern void send_message(const db::node* from, const db::node::node_id to, const db::simple_tuple* stpl);
     
    /* Return whether or not the node with id `id` belongs to the current
     * process
     */
     extern bool on_current_process(const db::node::node_id id);

    /* Return the process id that's responsible for the node id
     */
     extern int get_process_id(const db::node::node_id id);
     
     extern void set_color(db::node *n, const int r, const int g, const int b);
} // namespace api
#endif

