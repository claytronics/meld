#include <iostream>
#include <boost/asio.hpp>
#include <string>

#include "db/database.hpp"
#include "process/remote.hpp"
#include "sched/common.hpp"
#include "process/machine.hpp"
#include "utils/utils.hpp"
#include "api/api.hpp"
#include "sched/nodes/sim.hpp"
#include "sched/sim.hpp"

using namespace db;
using namespace vm;
using namespace process;
using boost::asio::ip::tcp;
using sched::sim_node;
using sched::sim_sched;
using sched::base;
using namespace sched;


#define SETID 1
#define STOP 4
#define ADD_NEIGHBOR 5
#define REMOVE_NEIGHBOR 6
#define TAP 7
#define SET_COLOR 8
#define SEND_MESSAGE 12
#define RECEIVE_MESSAGE 13
#define ACCEL 14
#define SHAKE 15


// debug messages for simulation
// #define DEBUG

namespace api
{
static boost::asio::ip::tcp::socket *my_tcp_socket;
static void process_message(message_type* reply);
static void add_received_tuple(sim_node *no, size_t ts, db::simple_tuple *stpl);
static void add_neighbor(const size_t ts, sim_node *no, const node_val out, const face_t face, const int count);
static void add_neighbor_count(const size_t ts, sim_node *no, const size_t total, const int count);
static void add_vacant(const size_t ts,  sim_node *no, const face_t face, const int count);
static void handle_setid(deterministic_timestamp ts, node::node_id node_id);
static void handle_receive_message(const deterministic_timestamp ts, db::node::node_id node,
      const face_t face, db::node::node_id dest_id, utils::byte *data, int offset, const int limit);
static void handle_add_neighbor(const deterministic_timestamp ts, const db::node::node_id in,
      const db::node::node_id out, const face_t face);
static void handle_remove_neighbor(const deterministic_timestamp ts,
      const db::node::node_id in, const face_t face);
static void handle_tap(const deterministic_timestamp ts, const db::node::node_id node);
static void handle_accel(const deterministic_timestamp ts, const db::node::node_id node,
      const int_val f);
static void handle_shake(const deterministic_timestamp ts, const db::node::node_id node,
      const int_val x, const int_val y, const int_val z);

static message_type *tcp_poll();
static void init_tcp();
static void send_message_tcp(message_type *msg);

static bool ready(false);


static sched::base *sched_state(NULL);
vm::predicate* neighbor_pred(NULL);
vm::predicate* tap_pred(NULL);
vm::predicate* neighbor_count_pred(NULL);
vm::predicate* accel_pred(NULL);
vm::predicate* shake_pred(NULL);
vm::predicate* vacant_pred(NULL);
bool stop_all(false);
utils::unix_timestamp start_time(0);
/*Queues to be used.*/
//queue::push_safe_linear_queue<sim_sched::message_type*> bbsimapi::socket_messages;

using namespace std;

	
void init(sched::base *schedular)
{	
	try{
   	/* Calling the connect*/
 	init_tcp();
	check_pre(schedular);
	while(!isReady())
		poll();
	} catch(std::exception &e) {
		throw machine_error("can't connect to simulator");
	}
}

void check_pre(sched::base *schedular){

	sched_state=schedular;

	cout<<"Setting the predicates"<<endl;	
	// find neighbor predicate
	neighbor_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("neighbor");
	if(neighbor_pred) {
	  assert(neighbor_pred->num_fields() == 2);
	} else {
	  cerr << "No neighbor predicate found" << endl;
	}

	tap_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("tap");
	if(tap_pred) {
	  assert(tap_pred->num_fields() == 0);
	} else {
	  cerr << "No tap predicate found" << endl;
	}

	neighbor_count_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("neighborCount");
	if(neighbor_count_pred) {
	  assert(neighbor_count_pred->num_fields() == 1);
	} else {
	  cerr << "No neighbor_count predicate found" << endl;
	}

	accel_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("accel");
	if(accel_pred) {
	  assert(accel_pred->num_fields() == 1);
	} else {
	  cerr << "No accel predicate found" << endl;
	}

	shake_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("shake");
	if(shake_pred) {
	  assert(shake_pred->num_fields() == 3);
	} else {
	  cerr << "No shake predicate found" << endl;
	}

	vacant_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("vacant");
	if(vacant_pred) {
	  assert(vacant_pred->num_fields() == 1);
	} else {
	  cerr << "No vacant predicate found" << endl;
	}

	start_time = utils::get_timestamp();
}

/*earlier master_get_work()*/
bool poll(void)
{
	message_type *reply;
  // size_t length;	

	
	/*Change the name of the poll function here*/
		if((reply =(message_type*)tcp_poll()) == NULL) {
      			return false;
		}
		process_message(reply);
		return true;
		
}


void set_color(db::node *n, const int r, const int g, const int b)
{
message_type *data=new message_type[8];
    size_t i(0);
cout<<"In setcolor"<<endl;
data[i++] = 7 * sizeof(message_type);
data[i++] = SET_COLOR;
    data[i++] = 0;
data[i++] = (message_type)n->get_id();
data[i++] = (message_type)r; // R
data[i++] = (message_type)g; // G
data[i++] = (message_type)b; // B
    data[i++] = 0; // intensity

    send_message_tcp(data);
delete []data;

}


/*Sends the "SEND_MESSAGE" command*/
void send_message(db::node* from,const db::node::node_id to, db::simple_tuple* stpl)
{
	message_type reply[MAXLENGTH];

	const size_t stpl_size(stpl->storage_size());
	const size_t msg_size = 5 * sizeof(message_type) + stpl_size;
	//sim_node *no(dynamic_cast<sim_node*>(info.work.get_node()));
	//Something to represent destination node.
	size_t i = 0;
	reply[i++] = (message_type)msg_size;
	reply[i++] = SEND_MESSAGE;
	reply[i++] = 0;//(message_type)ts;
	reply[i++] = from->get_id();
	reply[i++] = 0; //(dynamic_cast<sim_node*>(from))->get_face(to);
	reply[i++] = to;
	//cout << info.src->get_id() << " Send " << *stpl << endl;

	int pos = i * sizeof(message_type);
	stpl->pack((utils::byte*)reply, msg_size + sizeof(message_type), &pos);

	assert((size_t)pos == msg_size + sizeof(message_type));

	simple_tuple::wipeout(stpl);

	send_message_tcp(reply);
}

/*Flags if VM can run now*/
bool isReady(){
	return ready;
}



/*tcp helper functions begin*/
static void init_tcp()
    {
        try {
            boost::asio::io_service io_service;
            tcp::resolver resolver(io_service);
	/*Change the arguments from hard-coded to variables*/
            tcp::resolver::query query(tcp::v4(), "127.0.0.1", "5000");
            tcp::resolver::iterator iterator = resolver.resolve(query);

            my_tcp_socket = new tcp::socket(io_service);
            my_tcp_socket->connect(*iterator);
        } catch(std::exception &e) {
            cout<<"Could not connect!"<<endl;
        }
    }
	
static message_type *tcp_poll()
    {
		/*cout<<"in tcp poll, ready="<<ready<<endl;*/
        message_type msg[1024];
        try {
            if(my_tcp_socket->available())
            {
                size_t length = my_tcp_socket->read_some(boost::asio::buffer(msg, sizeof(message_type)));
                length = my_tcp_socket->read_some(boost::asio::buffer(msg + 1,  msg[0]));
				cout<<"Returning message of length "<< msg[0]<<endl;
                return msg;		
            }
        } catch(std::exception &e) {
            cout<<"Could not recieve!"<<endl;
            return NULL;
        }
        return NULL;
    }

static void send_message_tcp(message_type *msg)
{
   boost::asio::write(*my_tcp_socket, boost::asio::buffer(msg, msg[0] + sizeof(message_type)));
}

/*TCP helper functions end*/


/*Helper functions*/

static void process_message(message_type* reply){
		
		assert(reply!=NULL);
		
		switch(reply[1]) {
			case SETID: /*Adding the setid command to the interface _ankit*/
				handle_setid((deterministic_timestamp) reply[2], (db::node::node_id) reply[3]);
				ready=true;
				break;
			case RECEIVE_MESSAGE:
            	handle_receive_message((deterministic_timestamp)reply[2],
                   (db::node::node_id)reply[3],
                   (face_t)reply[4],
					(db::node::node_id)reply[5],
                   (utils::byte*)reply,
                   6 * sizeof(message_type),
                   (int)(reply[0] + sizeof(message_type)));
            	break;
			case ADD_NEIGHBOR:
            	handle_add_neighbor((deterministic_timestamp)reply[2],
                  (db::node::node_id)reply[3],
                  (db::node::node_id)reply[4],
                  (face_t)reply[5]);
            	break;
         	case REMOVE_NEIGHBOR:
            	handle_remove_neighbor((deterministic_timestamp)reply[2],
                  (db::node::node_id)reply[3],
                  (face_t)reply[4]);
            	break;
			case TAP:
            	handle_tap((deterministic_timestamp)reply[2], (db::node::node_id)reply[3]);
            	break;
         	case ACCEL:
            	handle_accel((deterministic_timestamp)reply[2],
                  (db::node::node_id)reply[3],
                  (int)reply[4]);
            	break;
         	case SHAKE:
            	handle_shake((deterministic_timestamp)reply[2], (db::node::node_id)reply[3],
                  (int)reply[4], (int)reply[5], (int)reply[6]);
            	break;
			case STOP:
				stop_all = true;
		        sleep(1);
		        //send_pending_messages();
		        usleep(200);
				
         	default: cerr << "Unrecognized message " << reply[1] << endl;
			}
}





static void add_received_tuple(sim_node *no, size_t ts, db::simple_tuple *stpl)
{
	heap_priority pr;
	pr.int_priority = ts;
	if(stpl->get_count() > 0)
		no->tuple_pqueue.insert(stpl, pr);
	else
		no->rtuple_pqueue.insert(stpl, pr);
}

/* ? to be kept in the api or the sim_sched*/
static void add_neighbor(const size_t ts, sim_node *no, const node_val out, const face_t face, const int count)
{
   if(!neighbor_pred)
      return;

   vm::tuple *tpl(new vm::tuple(neighbor_pred));
   tpl->set_node(0, out);
   tpl->set_int(1, static_cast<int_val>(face));
				
   db::simple_tuple *stpl(new db::simple_tuple(tpl, count));
				
   add_received_tuple(no, ts, stpl);
}

static void add_neighbor_count(const size_t ts, sim_node *no, const size_t total, const int count)
{
   if(!neighbor_count_pred)
      return;

   vm::tuple *tpl(new vm::tuple(neighbor_count_pred));
   tpl->set_int(0, (int_val)total);

   db::simple_tuple *stpl(new db::simple_tuple(tpl, count));

   add_received_tuple(no, ts, stpl);
}

static void add_vacant(const size_t ts,  sim_node *no, const face_t face, const int count)
{
   if(!vacant_pred)
      return;

   vm::tuple *tpl(new vm::tuple(vacant_pred));
   tpl->set_int(0, static_cast<int_val>(face));

   db::simple_tuple *stpl(new db::simple_tuple(tpl, count));

   add_received_tuple(no, ts, stpl);
}


/*function to set the id of the block _ankit*/
static void handle_setid(deterministic_timestamp ts, db::node::node_id node_id){
	#ifdef DEBUG
   cout << "Create node with " << start_id << endl;
#endif
	/*similar to create_n_nodes but without the loop _ankit*/
      db::node *no((sched_state)->state.all->DATABASE->create_node_id(node_id));
      sched_state->init_node(no);
      cout<<"Node id is "<<no->get_id()<<endl;
         sim_node *no_in((sim_node *)no);
         no_in->set_instantiated(true);
         for(face_t face = sim_node::INITIAL_FACE; face <= sim_node::FINAL_FACE; ++face) {
            add_vacant(ts, no_in, face, 1);
         }
         add_neighbor_count(ts, no_in, 0, 1);
      
   	
}

static void handle_receive_message(const deterministic_timestamp ts,
db::node::node_id dest_id,
      const face_t face, db::node::node_id node, utils::byte *data, int offset, const int limit)
{
   sim_node *origin(dynamic_cast<sim_node*>((sched_state->state).all->DATABASE->find_node(node)));
   sim_node *target(NULL);
	target=dynamic_cast<sim_node*>((sched_state->state).all->DATABASE->find_node(dest_id));

/*
   if(face == INVALID_FACE)
      target = origin;
   else
      target = dynamic_cast<sim_node*>((sched_state->state).all->DATABASE->find_node((db::node::node_id)*(origin->get_node_at_face(face))));
*/

   simple_tuple *stpl(simple_tuple::unpack(data, limit,
            &offset, (sched_state->state).all->PROGRAM));

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

static void handle_add_neighbor(const deterministic_timestamp ts, const db::node::node_id in,
      const db::node::node_id out, const face_t face)
{
#ifdef DEBUG
   cout << ts << " neighbor(" << in << ", " << out << ", " << face << ")" << endl;
#endif
   
   sim_node *no_in(dynamic_cast<sim_node*>((sched_state->state).all->DATABASE->find_node(in)));
   node_val *neighbor(no_in->get_node_at_face(face));

   if(*neighbor == sim_node::NO_NEIGHBOR) {
      // remove vacant first, add 1 to neighbor count
      if(no_in->has_been_instantiated()) {
         add_vacant(ts, no_in, face, -1);
         add_neighbor_count(ts, no_in, no_in->get_neighbor_count(), -1);
      }
      no_in->inc_neighbor_count();
#ifdef DEBUG
      cout << in << " neighbor count went up to " << no_in->get_neighbor_count() << endl;
#endif
      if(no_in->has_been_instantiated())
         add_neighbor_count(ts, no_in, no_in->get_neighbor_count(), 1);
      *neighbor = out;
      if(no_in->has_been_instantiated())
         add_neighbor(ts, no_in, out, face, 1);
   } else {
      if(*neighbor != out) {
         // remove old node
         if(no_in->has_been_instantiated())
            add_neighbor(ts, no_in, *neighbor, face, -1);
         *neighbor = out;
         if(no_in->has_been_instantiated())
            add_neighbor(ts, no_in, out, face, 1);
      }
   }
}

static void handle_remove_neighbor(const deterministic_timestamp ts,
      const db::node::node_id in, const face_t face)
{
#ifdef DEBUG
   cout << ts << " remove neighbor(" << in << ", " << face << ")" << endl;
#endif
   
   sim_node *no_in(dynamic_cast<sim_node*>((sched_state->state).all->DATABASE->find_node(in)));
   node_val *neighbor(no_in->get_node_at_face(face));

   if(*neighbor == sim_node::NO_NEIGHBOR) {
      // remove vacant first, add 1 to neighbor count
      cerr << "Current face is vacant, cannot remove node!" << endl;
      assert(false);
   } else {
      // remove old node
      if(no_in->has_been_instantiated())
         add_neighbor_count(ts, no_in, no_in->get_neighbor_count(), -1);
      no_in->dec_neighbor_count();
      if(no_in->has_been_instantiated())
         add_neighbor_count(ts, no_in, no_in->get_neighbor_count(), 1);
   }

   add_neighbor(ts, no_in, *neighbor, face, -1);

   *neighbor = sim_node::NO_NEIGHBOR;
}

static void handle_tap(const deterministic_timestamp ts, const db::node::node_id node)
{
   cout << ts << " tap(" << node << ")" << endl;
   
   sim_node *no(dynamic_cast<sim_node*>((sched_state->state).all->DATABASE->find_node(node)));

   if(tap_pred) {
      vm::tuple *tpl(new vm::tuple(tap_pred));
      db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));
      
      add_received_tuple(no, ts, stpl);
   }
}

static void handle_accel(const deterministic_timestamp ts, const db::node::node_id node,
      const int_val f)
{
   cout << ts << " accel(" << node << ", " << f << ")" << endl;

   sim_node *no(dynamic_cast<sim_node*>((sched_state->state).all->DATABASE->find_node(node)));

   if(accel_pred) {
      vm::tuple *tpl(new vm::tuple(accel_pred));
      tpl->set_int(0, f);

      db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));

      add_received_tuple(no, ts, stpl);
   }
}


static void handle_shake(const deterministic_timestamp ts, const db::node::node_id node,
      const int_val x, const int_val y, const int_val z)
{
   cout << ts << " shake(" << node << ", " << x << ", " << y << ", " << z << ")" << endl;

   sim_node *no(dynamic_cast<sim_node*>((sched_state->state).all->DATABASE->find_node(node)));

   if(shake_pred) {
      vm::tuple *tpl(new vm::tuple(shake_pred));
      tpl->set_int(0, x);
      tpl->set_int(1, y);
      tpl->set_int(2, z);

      db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));

      add_received_tuple(no, ts, stpl);
   }
}
/*Helper functions end*/


}