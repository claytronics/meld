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

using namespace std;

namespace api {
    boost::mpi::communicator *world;
    int world_sum;
    bool reset_token;

    /* Send data to the dest */
    void send_message(const db::node::node_id id,
		      const db::simple_tuple *stpl) {
	int dest = id % world->size();

	message_type *msg = new message_type[512];
	size_t msg_length = 512 * sizeof(message_type);
	int p = 0;

	stpl->pack((utils::byte *) msg, msg_length, &p);
	world->isend(dest, id, *msg);
    };

    bool poll(sched::base *sched, vm::all *all) {
	/* Receive the message if there is a message waiting to be received,
	 * deserialize the message, and then add the message to the
	 * scheduler's queue as new work
	 */
	boost::optional<boost::mpi::status> status_opt;
	boost::mpi::status status;
	bool received_self = false;
	bool consume_token = false;
	int token;
	int dest = (world->rank() + 1) % world->size();

	/* TODO Token_tag needs to be the same across process */
	int TOKEN_TAG = all->DATABASE->max_id() + world->size();

	world->isend(dest, TOKEN_TAG, world->rank());

	while ((status_opt = world->iprobe(boost::mpi::any_source,
					   boost::mpi::any_tag))) {
	    status = status_opt.get();

	    if (status.tag() < TOKEN_TAG) {
		/* a tuple message that needs to be processed by scheduler */
		consume_token = true;

		message_type *msg = new message_type[512];
		size_t msg_length = 512 * sizeof(message_type);
		world->irecv(boost::mpi::any_source, boost::mpi::any_tag, *msg);

		/* Extract tuple from message */
		int pos = 0;
		db::simple_tuple *stpl = db::simple_tuple::unpack(
		    (utils::byte *) msg, msg_length, &pos, all->PROGRAM);


		all->MACHINE->route(NULL, sched, status.tag(),
				    stpl, 0);
	    } else {
		world->irecv(boost::mpi::any_source, TOKEN_TAG, token);
		assert(token >= 0);

		if (token != world->rank()) {
		    world->isend(dest, TOKEN_TAG, token);
		} else {
		    received_self = true;
		}
	    }
	    break;
	}

	if (received_self && !consume_token) {
	    /* Terminate */
	    return false;
	} else {
	    return true;
	}
	// if (consume_token || sched->get_work())
	//     return true;
	// else {
	//     int dest = (world->rank()+1) % world->size();

	//     if (reset_token) {
	// 	/* start off a new token */
	// 	world->isend(dest, TOKEN_TAG, world->rank());
	// 	cout << "Proc: " << world->rank() << " new token" << endl;
	// 	api::reset_token = false;
	//     } else {
	// 	if(token >= 2 * world_sum) {
	// 	    /* token has traveled around the ring at least twice,
	// 	     * should be safe to terminate the process */

	// 	    world->isend(dest, TOKEN_TAG, token);
	// 	    cout << "Proc: " << world->rank() << " end token" << endl;
	// 	    assert(!sched->get_work());
	// 	    return false;
	// 	} else {
	// 	    if (token < 0) {
	// 		world->isend(dest, TOKEN_TAG, world->rank());
	// 	    } else {
	// 		/* Accumulate into token */
	// 		world->isend(dest, TOKEN_TAG, world->rank() + token);
	// 		cout << "Proc: " << world->rank() << " acc token" << endl;
	// 	    }
	// 	}

	//     }

	//     boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	//     return true;
	// }
    }

    message_type *create_message(const db::simple_tuple *tuple) {
	/* create a message by serializing the tuple */
	message_type *msg = new message_type[MAXLENGTH];
	int pos = 0;

	tuple->pack((utils::byte *)msg, MAXLENGTH, &pos);
	return msg;
    }

    bool on_current_process(const db::node::node_id id) {
	return id % api::world->size() == api::world->rank();
    }

    void init(int argc, char **argv) {
	world_sum = (world->size() - 1) * (world->size()) / 2;
	reset_token = true;
    }

} /* namespace api */
