
#include <boost/asio.hpp>
#include <string>

#include "sched/sim.hpp"
#include "db/database.hpp"
#include "process/remote.hpp"
#include "sched/common.hpp"
#include "process/machine.hpp"
#include "utils/utils.hpp"

using namespace db;
using namespace vm;
using namespace process;
using boost::asio::ip::tcp;

//#ifdef USE_SIM

// when running in thread mode, the VM waits this milliseconds to instantiate all neighbor facts
static const int TIME_TO_INSTANTIATE = 500;

//#define SET_ID 1
//#define SEND_MESSAGE 12

// debug messages for simulation
// #define DEBUG

namespace sched
{
typedef uint64_t message_type;
int sim_sched::PORT(0);
//utils::unix_timestamp sim_sched::start_time(0);
//queue::push_safe_linear_queue<sim_sched::message_type*> sim_sched::socket_messages;

using namespace std;
//-----------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------
/*sim_sched::~sim_sched(void)
{
	if(socket != NULL) {
		//socket->close();
		//delete socket;
	}
}
*/
//---------------------------------------------------------------------------------
message_type
sim_sched::init_tcp(const size_t num_threads)
{
if(slave)
return 0;
// Only happens once.
assert(num_threads == 1);

  // database::map_nodes::iterator it(state.all->DATABASE->get_node_iterator(remote::self->find_first_node(id)));
  // database::map_nodes::iterator end(state.all->DATABASE->get_node_iterator(remote::self->find_last_node(id)));

// no nodes
//assert(it == end);

//state::SIM = true;

try {
    // add socket
boost::asio::io_service io_service;
message_type *reply;

bool x=true;
tcp::resolver resolver(io_service);
tcp::resolver::query query(tcp::v4(), "127.0.0.1", utils::to_string(PORT));
tcp::resolver::iterator iterator = resolver.resolve(query);

socket = new tcp::socket(io_service);
socket->connect(*iterator);
	while(x)
	{
		if(socket->available())
		{
			size_t length =
				socket->read_some(boost::asio::buffer(reply, sizeof(message_type)));
			assert(length == sizeof(message_type));
			length = socket->read_some(boost::asio::buffer(reply+1,  reply[0]));

			assert(length == (size_t)reply[0]);
			x=false;		
					
		}
	
	}
cout<<"node id = "<<reply[3]<<endl;
 return reply[3];
} catch(std::exception &e) {
throw machine_error("can't connect to simulator");
return 0;
}
  // start_time = utils::get_timestamp();
return 0;
}

//-----------------------------------------------------------------------------------
/*void
sim_sched::send_pending_messages(void)
{
   while(!socket_messages.empty()) {
      message_type *data(socket_messages.pop());
      boost::asio::write(*socket, boost::asio::buffer(data, data[0] + sizeof(message_type)));
      delete []data;
   }
}
*/
/*//-----------------------------------------------------------------------------------
void
sim_sched::send_send_message(const work_info& info, const deterministic_timestamp ts)
{
   message_type reply[MAXLENGTH];

   db::simple_tuple *stpl(info.work.get_tuple());
   const size_t stpl_size(stpl->storage_size());
   const size_t msg_size = 4 * sizeof(message_type) + stpl_size;
   sim_node *no(dynamic_cast<sim_node*>(info.work.get_node()));
   size_t i = 0;
   reply[i++] = (message_type)msg_size;
   reply[i++] = SEND_MESSAGE;
   reply[i++] = (message_type)ts;
   reply[i++] = (message_type)info.src->get_id();
   reply[i++] = (message_type)info.src->get_face(no->get_id());

   cout << info.src->get_id() << " Send " << *stpl << endl;

   int pos = i * sizeof(message_type);
   stpl->pack((utils::byte*)reply, msg_size + sizeof(message_type), &pos);

   assert((size_t)pos == msg_size + sizeof(message_type));

   simple_tuple::wipeout(stpl);

   boost::asio::write(*socket, boost::asio::buffer(reply, reply[0] + sizeof(message_type)));
}
*/
/*void 
sim_sched::send_send_message(message_type *reply)
{

boost::asio::write(*socket, boost::asio::buffer(reply, reply[0] + sizeof(message_type)));

}
*/
void 
sim_sched::send_send_message(message_type *reply)
{

boost::asio::write(*socket, boost::asio::buffer(reply, reply[0] + sizeof(message_type)));

}



//-----------------------------------------------------------------------------------
/*
void
sim_sched::handle_receive_message(const deterministic_timestamp ts, db::node::node_id node,
      const face_t face, utils::byte *data, int offset, const int limit)
{
   assert(!thread_mode);
   sim_node *origin(dynamic_cast<sim_node*>(state.all->DATABASE->find_node(node)));
   sim_node *target(NULL);

   if(face == INVALID_FACE)
      target = origin;
   else
      target = dynamic_cast<sim_node*>(state.all->DATABASE->find_node((db::node::node_id)*(origin->get_node_at_face(face))));

   simple_tuple *stpl(simple_tuple::unpack(data, limit,
            &offset, state.all->PROGRAM));

#ifdef DEBUG
   cout << "Receive message " << origin->get_id() << " to " << target->get_id() << " " << *stpl << " with priority " << ts << endl;
#endif

   heap_priority pr;
   pr.int_priority = ts;
   if(stpl->get_count() > 0)
      target->tuple_pqueue.insert(stpl, pr);
   else
      target->rtuple_pqueue.insert(stpl, pr);
}
*/
void *
sim_sched::poll()
{
	/* code 1: */
	size_t bytes_available = 0;
	size_t length = 0;
	message_type *reply = NULL;

	if((bytes_available = socket->available()) > 0)
	{
		length += socket->read_some(boost::asio::buffer(reply, bytes_available));
	}

	return reply;
	

	/*  code 2:
	std::vector<char> data(socket_available());
	boost::asio::read(socket, boost::asio::buffer(data));
	*/

	/* code 3:
	boost::asio::streambuf data;
	boost::asio::read(socket,data,boost::asio::transfer_at_least(socket->available()));
		
	*/
}
//-----------------------------------------------------------------------------------
/*void
sim_sched::check_delayed_queue(void)
{
   const utils::unix_timestamp now(utils::get_timestamp());

   while(!delay_queue.empty()) {
      const utils::unix_timestamp best(delay_queue.top_priority());

      if(best < now) {
         work_info info(delay_queue.pop());
         sim_node *target(dynamic_cast<sim_node*>(info.work.get_node()));
         send_send_message(info, max(dynamic_cast<sim_node*>(info.src)->timestamp, target->timestamp + 1));
      } else {
         return;
      }
   }
}
*/
//-----------------------------------------------------------------------------------
/*void
sim_sched::schedule_new_message(message_type *data)
{
	boost::asio::write(*socket, boost::asio::buffer(data, data[0] + sizeof(message_type)));
	delete []data;
}
*/
void
sim_sched::schedule_new_message(message_type *data)
{
	boost::asio::write(*socket, boost::asio::buffer(data, data[0] + sizeof(message_type)));
	delete []data;
}

//-----------------------------------------------------------------------------------



int 
sim_sched::main()
{  
 	message_type id;
	id=init_tcp(1);
	message_type *data = new message_type[8];
    size_t i(0);	
	data[i++] = 7 * sizeof(message_type);
	data[i++] = 8;
    data[i++] = 0;
	data[i++] = id; 
	data[i++] = 255;
	data[i++] = 255;
	data[i++] = 255;
    data[i++] = 0; // intensity
	
	schedule_new_message(data);
	
	return 0;
}
}
//#endif
