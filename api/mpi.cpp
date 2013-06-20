/*
 * The MPI communication API
 */
#include "api/api.hpp"
#include "utils/types.hpp"

using namespace std;

namespace api {
  boost::mpi::communicator *world;

  /* Send data to the dest */
  void send_message(const db::node::node_id id, message_type *msg,
		    size_t msg_length, vm::all *all, const int tag) {
    int dest = id % api::world->size();

    boost::serialization::binary_object obj =
      boost::serialization::binary_object(msg, msg_length);

    api::world->isend(dest, tag, obj);
  };

  void poll(sched::base *sched, vm::all *all) {
    /* Receive the message if there is a message waiting to be received,
     * deserialize the message, and then add the message to the
     * scheduler's queue as new work
     */
    boost::optional<boost::mpi::status> status;

    if ((status = api::world->iprobe(boost::mpi::any_source, boost::mpi::any_tag))) {
      message_type *msg = new message_type[512];
      size_t msg_length = 512 * sizeof(message_type);

      boost::serialization::binary_object obj =
	boost::serialization::binary_object(msg, msg_length);

      int pos = 0;

      api::world->irecv(boost::mpi::any_source, boost::mpi::any_tag, obj);

      db::simple_tuple *stpl = db::simple_tuple::unpack((utils::byte *) msg,
							msg_length, &pos,
							all->PROGRAM);

      cout << "Process " << api::world->rank() << " <== Process "
	   << status.get().source() << " :: " << *stpl
	   << " :: " << status.get().tag() << endl;

      /* Once the message is received, add the work to the new
       * scheduler */
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
  }

} /* namespace api */
