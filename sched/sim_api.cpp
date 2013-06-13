#include <iostream>
#include <boost/asio.hpp>
#include <string>

#include "sched/sim.hpp"
#include "sched/sim_tcp.hpp"
#include "db/database.hpp"
#include "process/remote.hpp"
#include "sched/common.hpp"
#include "process/machine.hpp"
#include "utils/utils.hpp"

using namespace db;
using namespace vm;
using namespace process;
using namespace sim_tcp;
using boost::asio::ip::tcp;


#ifdef USE_SIM

// when running in thread mode, the VM waits this milliseconds to instantiate all neighbor facts
static const int TIME_TO_INSTANTIATE = 500;

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
#define CREATE_N_NODES 18

// debug messages for simulation
// #define DEBUG

namespace sched
{

bool sim_sched::thread_mode(false);
int sim_sched::PORT(0);
vm::predicate* sim_sched::neighbor_pred(NULL);
vm::predicate* sim_sched::tap_pred(NULL);
vm::predicate* sim_sched::neighbor_count_pred(NULL);
vm::predicate* sim_sched::accel_pred(NULL);
vm::predicate* sim_sched::shake_pred(NULL);
vm::predicate* sim_sched::vacant_pred(NULL);
bool sim_sched::stop_all(false);
bool sim_sched::all_instantiated(false);
utils::unix_timestamp sim_sched::start_time(0);
queue::push_safe_linear_queue<sim_sched::message_type*> sim_sched::socket_messages;

using namespace std;
	
sim_sched::~sim_sched(void)
{
//	if(socket != NULL) {
		//socket->close();
		//delete socket;
//	}
}
	
void
sim_sched::init(const size_t num_threads)
{
	if(slave)
		return;
		
	assert(num_threads == 1);
	
//	state::SIM = true;
	
	try {
   	// add socket
 	sim_tcp::init_tcp();
	} catch(std::exception &e) {
		throw machine_error("can't connect to simulator");
	}
	
	// find neighbor predicate
	neighbor_pred = state.all->PROGRAM->get_predicate_by_name("neighbor");
	if(neighbor_pred) {
	  assert(neighbor_pred->num_fields() == 2);
	} else {
	  cerr << "No neighbor predicate found" << endl;
	}

	tap_pred = state.all->PROGRAM->get_predicate_by_name("tap");
	if(tap_pred) {
	  assert(tap_pred->num_fields() == 0);
	} else {
	  cerr << "No tap predicate found" << endl;
	}

	neighbor_count_pred = state.all->PROGRAM->get_predicate_by_name("neighborCount");
	if(neighbor_count_pred) {
	  assert(neighbor_count_pred->num_fields() == 1);
	} else {
	  cerr << "No neighbor_count predicate found" << endl;
	}

	accel_pred = state.all->PROGRAM->get_predicate_by_name("accel");
	if(accel_pred) {
	  assert(accel_pred->num_fields() == 1);
	} else {
	  cerr << "No accel predicate found" << endl;
	}

	shake_pred = state.all->PROGRAM->get_predicate_by_name("shake");
	if(shake_pred) {
	  assert(shake_pred->num_fields() == 3);
	} else {
	  cerr << "No shake predicate found" << endl;
	}

	vacant_pred = state.all->PROGRAM->get_predicate_by_name("vacant");
	if(vacant_pred) {
	  assert(vacant_pred->num_fields() == 1);
	} else {
	  cerr << "No vacant predicate found" << endl;
	}

	start_time = utils::get_timestamp();
}

void
sim_sched::new_work_delay(sched::base *target, const db::node *_src, work& new_work, const uint_val delay)
{
	work_info info;
	info.work = new_work;
	//info.timestamp = state.sim_instr_counter;
	info.src = dynamic_cast<sim_node*>((node*)_src);
	delay_queue.push(info, utils::get_timestamp() + delay);
}

void
sim_sched::new_work(const node *_src, work& new_work)
{
	sim_node *to(dynamic_cast<sim_node*>(new_work.get_node()));

	db::simple_tuple *stpl(new_work.get_tuple());

	if(current_node == NULL) {
		heap_priority pr;
		pr.int_priority = 0; // this is the init tuple...
		if(stpl->get_count() > 0)
			to->tuple_pqueue.insert(stpl, pr);
		else
			to->rtuple_pqueue.insert(stpl, pr);
	} else {
		sim_node *src(dynamic_cast<sim_node*>((node*)_src));
		work_info info;
		info.work = new_work;
		//info.timestamp = state.sim_instr_counter;
		info.src = src;
		tmp_work.push_back(info);
	}
}

void
sim_sched::send_pending_messages(void)
{
	while(!socket_messages.empty()) {
		message_type *data(socket_messages.pop());
		sim_tcp::send_message(data);
		delete []data;
	}
}

void
sim_sched::add_received_tuple(sim_node *no, size_t ts, db::simple_tuple *stpl)
{
	heap_priority pr;
	pr.int_priority = ts;
	if(stpl->get_count() > 0)
		no->tuple_pqueue.insert(stpl, pr);
	else
		no->rtuple_pqueue.insert(stpl, pr);
}

void
sim_sched::add_neighbor(const size_t ts, sim_node *no, const node_val out, const face_t face, const int count)
{
   if(!neighbor_pred)
      return;

   vm::tuple *tpl(new vm::tuple(neighbor_pred));
   tpl->set_node(0, out);
   tpl->set_int(1, static_cast<int_val>(face));
				
   db::simple_tuple *stpl(new db::simple_tuple(tpl, count));
				
   add_received_tuple(no, ts, stpl);
}

void
sim_sched::add_neighbor_count(const size_t ts, sim_node *no, const size_t total, const int count)
{
   if(!neighbor_count_pred)
      return;

   vm::tuple *tpl(new vm::tuple(neighbor_count_pred));
   tpl->set_int(0, (int_val)total);

   db::simple_tuple *stpl(new db::simple_tuple(tpl, count));

   add_received_tuple(no, ts, stpl);
}

void
sim_sched::add_vacant(const size_t ts, sim_node *no, const face_t face, const int count)
{
   if(!vacant_pred)
      return;

   vm::tuple *tpl(new vm::tuple(vacant_pred));
   tpl->set_int(0, static_cast<int_val>(face));

   db::simple_tuple *stpl(new db::simple_tuple(tpl, count));

   add_received_tuple(no, ts, stpl);
}

void
sim_sched::instantiate_all_nodes(void)
{
	assert(!all_instantiated);
	for(database::map_nodes::const_iterator
		it(state.all->DATABASE->nodes_begin()),end(state.all->DATABASE->nodes_end());
		it!=end;++it){
	node *n(it->second);
	sim_node *no((sim_node *)n);

	no->set_instantiated(true);
	add_neighbor_count(0, no, no->get_neighbor_count(), 1);
	for(face_t face = sim_node::INITIAL_FACE; face <= sim_node::FINAL_FACE; ++face) {
		vm::node_val *neighbor(no->get_node_at_face(face));
		if(*neighbor == sim_node::NO_NEIGHBOR)
			add_vacant(0, no, face, 1);
		else
			add_neighbor(0, no, *neighbor, face, 1);
		}
	}
}

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

	sim_tcp::send_message(reply);
}

void
sim_sched::handle_deterministic_computation(void)
{/*
   std::set<sim_node*> nodes; // all touched nodes

   for(list<work_info>::iterator it(tmp_work.begin()), end(tmp_work.end());
      it != end;
      ++it)
   {
      const work_info& info(*it);
      send_send_message(info, info.timestamp);
      nodes.insert(dynamic_cast<sim_node*>(info.work.get_node()));
   }

   tmp_work.clear();

   if(!current_node->pending.empty()) {
      nodes.insert(current_node);
   }
	/* Removed by Xing, need to consult _ankit*/
  // current_node->timestamp = state.sim_instr_counter;
 /*  
   size_t i(0);
	message_type reply[MAXLENGTH];
   reply[i++] = (4 + nodes.size()) * sizeof(message_type);
   reply[i++] = NODE_RUN;
   reply[i++] = (message_type)current_node->timestamp;
   reply[i++] = (message_type)current_node->get_id();
   reply[i++] = (message_type)nodes.size();
   
   for(set<sim_node*>::iterator it(nodes.begin()), end(nodes.end());
      it != end;
      ++it)
   {
      reply[i++] = (message_type)(*it)->get_id();
   }

	send_message(reply);
   current_node = NULL;*/
}

void
sim_sched::handle_create_n_nodes(deterministic_timestamp ts, size_t n, node::node_id start_id)
{
#ifdef DEBUG
   cout << "Create " << n << " nodes from " << start_id << endl;
#endif
   for(message_type i(0); i != n; ++i) {
      db::node *no(state.all->DATABASE->create_node_id(start_id + i));
      init_node(no);
      if(!thread_mode || all_instantiated) {
         sim_node *no_in((sim_node *)no);
         no_in->set_instantiated(true);
         for(face_t face = sim_node::INITIAL_FACE; face <= sim_node::FINAL_FACE; ++face) {
            add_vacant(ts, no_in, face, 1);
         }
         add_neighbor_count(ts, no_in, 0, 1);
      }
   }
}


/*function to set the id of the block _ankit*/
void sim_sched::handle_setid(deterministic_timestamp ts, node::node_id node_id){
	#ifdef DEBUG
   cout << "Create node with " << start_id << endl;
#endif
	/*similar to create_n_nodes but without the loop _ankit*/
      db::node *no(state.all->DATABASE->create_node_id(node_id));
      init_node(no);
      if(!thread_mode || all_instantiated) {
         sim_node *no_in((sim_node *)no);
         no_in->set_instantiated(true);
         for(face_t face = sim_node::INITIAL_FACE; face <= sim_node::FINAL_FACE; ++face) {
            add_vacant(ts, no_in, face, 1);
         }
         add_neighbor_count(ts, no_in, 0, 1);
      }
   	
}

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

void
sim_sched::handle_add_neighbor(const deterministic_timestamp ts, const db::node::node_id in,
      const db::node::node_id out, const face_t face)
{
#ifdef DEBUG
   cout << ts << " neighbor(" << in << ", " << out << ", " << face << ")" << endl;
#endif
   
   sim_node *no_in(dynamic_cast<sim_node*>(state.all->DATABASE->find_node(in)));
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

void
sim_sched::handle_remove_neighbor(const deterministic_timestamp ts,
      const db::node::node_id in, const face_t face)
{
#ifdef DEBUG
   cout << ts << " remove neighbor(" << in << ", " << face << ")" << endl;
#endif
   
   sim_node *no_in(dynamic_cast<sim_node*>(state.all->DATABASE->find_node(in)));
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

void
sim_sched::handle_tap(const deterministic_timestamp ts, const db::node::node_id node)
{
   cout << ts << " tap(" << node << ")" << endl;
   
   sim_node *no(dynamic_cast<sim_node*>(state.all->DATABASE->find_node(node)));

   if(tap_pred) {
      vm::tuple *tpl(new vm::tuple(tap_pred));
      db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));
      
      add_received_tuple(no, ts, stpl);
   }
}

void
sim_sched::handle_accel(const deterministic_timestamp ts, const db::node::node_id node,
      const int_val f)
{
   cout << ts << " accel(" << node << ", " << f << ")" << endl;

   sim_node *no(dynamic_cast<sim_node*>(state.all->DATABASE->find_node(node)));

   if(accel_pred) {
      vm::tuple *tpl(new vm::tuple(accel_pred));
      tpl->set_int(0, f);

      db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));

      add_received_tuple(no, ts, stpl);
   }
}


void
sim_sched::handle_shake(const deterministic_timestamp ts, const db::node::node_id node,
      const int_val x, const int_val y, const int_val z)
{
   cout << ts << " shake(" << node << ", " << x << ", " << y << ", " << z << ")" << endl;

   sim_node *no(dynamic_cast<sim_node*>(state.all->DATABASE->find_node(node)));

   if(shake_pred) {
      vm::tuple *tpl(new vm::tuple(shake_pred));
      tpl->set_int(0, x);
      tpl->set_int(1, y);
      tpl->set_int(2, z);

      db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));

      add_received_tuple(no, ts, stpl);
   }
}

void
sim_sched::check_delayed_queue(void)
{
   const utils::unix_timestamp now(utils::get_timestamp());

   while(!delay_queue.empty()) {
      const utils::unix_timestamp best(delay_queue.top_priority());

      if(best < now) {
         work_info info(delay_queue.pop());
         sim_node *target(dynamic_cast<sim_node*>(info.work.get_node()));
         sim_sched::send_send_message(info, max(dynamic_cast<sim_node*>(info.src)->timestamp, target->timestamp + 1));
      } else {
         return;
      }
   }
}

node*
sim_sched::master_get_work(void)
{
	assert(!thread_mode && !slave);
	message_type *reply;
  // size_t length;	
	if(current_node) {
		// we just did a round of computation
		assert(!thread_mode);

      handle_deterministic_computation();
	}
	
	while(true) {

		if((reply =(message_type*) sim_tcp::poll()) == NULL) {
      		send_pending_messages();
			usleep(100);
         	if(thread_mode && !all_instantiated) {
            	utils::unix_timestamp now(utils::get_timestamp());

            	if(now > start_time + TIME_TO_INSTANTIATE) {
            	    instantiate_all_nodes();
               		all_instantiated = true;
            	}
         	}
         	if(!thread_mode) {
         	   check_delayed_queue();
         	}
	
		} else {

		
	//	assert(length == (size_t)reply[0]);
		
		switch(reply[1]) {
			case SETID: /*Adding the setid command to the interface _ankit*/
				handle_setid((deterministic_timestamp) reply[2], (db::node::node_id) reply[3]);
				break;
			case CREATE_N_NODES:
            	handle_create_n_nodes((deterministic_timestamp)reply[2],
                  (size_t)reply[4],
                  (db::node::node_id)reply[5]);
				break;
         	case RECEIVE_MESSAGE:
            	handle_receive_message((deterministic_timestamp)reply[2],
                   (db::node::node_id)reply[3],
                   (face_t)reply[4],
                   (utils::byte*)reply,
                   5 * sizeof(message_type),
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
		        send_pending_messages();
		        usleep(200);
				return NULL;
         	default: cerr << "Unrecognized message " << reply[1] << endl;
			}
		}
	}
	assert(false);
	return NULL;
}

node*
sim_sched::get_work(void)
{
	if(slave) {
		while(current_node->pending.empty() && !current_node->delayed_available()) {
			usleep(100);
			if(stop_all)
				return NULL;
		}
		return current_node;
	}
   return master_get_work();
}

void
sim_sched::set_color(db::node *n, const int r, const int g, const int b)
{
	message_type *data = new message_type[8];
    size_t i(0);
	
	data[i++] = 7 * sizeof(message_type);
	data[i++] = SET_COLOR;
    data[i++] = 0;
	data[i++] = (message_type)n->get_id();
	data[i++] = (message_type)r; // R
	data[i++] = (message_type)g; // G
	data[i++] = (message_type)b; // B
    data[i++] = 0; // intensity

    schedule_new_message(data);
}

void
sim_sched::schedule_new_message(message_type *data)
{
	sim_tcp::send_message(data);
	delete []data;
}

void
sim_sched::new_agg(work& w)
{
    new_work(w.get_node(), w);
}

void
sim_sched::generate_aggs(void)
{
	iterate_static_nodes();
}

bool
sim_sched::terminate_iteration(void)
{
    generate_aggs();
	return false;
}

void
sim_sched::assert_end(void) const
{
    assert_static_nodes_end(id, state.all);
}

void
sim_sched::assert_end_iteration(void) const
{
	assert_static_nodes_end_iteration(id, state.all);
}

simple_tuple_vector
sim_sched::gather_active_tuples(db::node *node, const vm::predicate_id pred)
{
	return simple_tuple_vector();
}

void
sim_sched::gather_next_tuples(db::node *node, simple_tuple_list& ls)
{
	sim_node *no((sim_node*)node);
	no->pending.pop_list(ls);
   /* if(state.sim_instr_use) {
	    no->get_tuples_until_timestamp(ls, state.sim_instr_limit);
    }*/
    no->add_delayed_tuples(ls);
}

}
#endif
