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
	bool consume_token = false;
	int token = -1;

	/* TODO Token_tag needs to be the same across process */
	int TOKEN_TAG = all->DATABASE->nodes_total;

	while ((status_opt = world->iprobe(boost::mpi::any_source,
					       boost::mpi::any_tag))) {
	    status = status_opt.get();

	    if (status.tag() != TOKEN_TAG) {
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
		assert(token >= 0 && token <= 2 * world_sum);
	    }
	}

	if (consume_token || sched->get_work())
	    return true;
	else {
	    int dest = (world->rank()+1) % world->size();

	    if(token >= world_sum) {
		/* token has traveled around the ring at least twice,
		 * should be safe to terminate the process */

		world->isend(dest, TOKEN_TAG, token);
		cout << "Proc: " << world->rank() << " end token" << endl;
		return false;
	    } else {
		if (token >= 0) {
		    /* Accumulate into token */
		    world->isend(dest, TOKEN_TAG, world->rank() + token);
		    cout << "Proc: " << world->rank() << " acc token" << endl;
		} else {
		    /* start off a new token */
		    world->isend(dest, TOKEN_TAG, world->rank());
		    cout << "Proc: " << world->rank() << " new token" << endl;
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		return true;
	    }
	}
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
    }

} /* namespace api */
