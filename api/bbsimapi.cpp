#include <iostream>
#include <boost/asio.hpp>
#include <string>

#include "db/database.hpp"
#include "process/remote.hpp"
#include "sched/common.hpp"
#include "process/machine.hpp"
#include "utils/utils.hpp"
#include "api/api.hpp"
#include "sched/serial.hpp"
#include "msg/msg.hpp"
#include "debug/debug_handler.hpp"
#include "vm/determinism.hpp"

using namespace db;
using namespace vm;
using namespace vm::determinism;
using namespace utils;
using namespace process;
using namespace debugger;
using boost::asio::ip::tcp;
using sched::serial_node;
using sched::base;
using namespace sched;
using namespace msg;

#define SETID 1
#define DEBUG 16
#define STOP 4
#define ADD_NEIGHBOR 5
#define REMOVE_NEIGHBOR 6
#define TAP 7
#define SET_COLOR 8
#define SEND_MESSAGE 12
#define RECEIVE_MESSAGE 13
#define ACCEL 14
#define SHAKE 15

#define SET_DETERMINISTIC_MODE		20
#define RESUME_COMPUTATION			21
#define COMPUTATION_PAUSE			22
#define	WORK_END					23
#define TIME_INFO					24

// debug messages for simulation
// #define DEBUG

namespace api
{

  enum face_t {
   INVALID_FACE = -1,
   BOTTOM = 0,
   NORTH = 1,
   EAST = 2,
   WEST = 3,
   SOUTH = 4,
   TOP = 5
};

  /*Storing the block's neighbor information*/

   vm::node_val top;
   vm::node_val bottom;
   vm::node_val east;
   vm::node_val west;
   vm::node_val north;
   vm::node_val south;

   bool instantiated_flag(false);
   size_t neighbor_count;
inline face_t& operator++(face_t &f)
{
   f = static_cast<face_t>(f + 1);
   return f;
}

inline face_t operator++(face_t& f, int) {
   ++f;
   return f;
}




   /*Making compatible with simulator*/
   static const vm::node_val NO_NEIGHBOR = (vm::node_val)-1;

   static const face_t INITIAL_FACE = BOTTOM;
   static const face_t FINAL_FACE = TOP;
// returns a pointer to a certain face, allowing modification

   static vm::node_val 
   *get_node_at_face(const face_t face)
    {
      switch(face) {
         case BOTTOM: return &bottom;
         case NORTH: return &north;
         case EAST: return &east;
         case WEST: return &west;
         case SOUTH: return &south;
         case TOP: return &top;
         default: assert(false);
      }
   }

   /*Get the block at a particular face*/
   static face_t 
   get_face(const vm::node_val node) 
   {
      if(node == bottom) return BOTTOM;
      if(node == north) return NORTH;
      if(node == east) return EAST;
      if(node == west) return WEST;
      if(node == south) return SOUTH;
      if(node == top) return TOP;
      return INVALID_FACE;
   }

   static inline bool 
   has_been_instantiated(void)
   {
      return instantiated_flag;
   }

   static inline void 
   inc_neighbor_count(void)
   {
      ++neighbor_count;
   }

   static inline void 
   dec_neighbor_count(void)
   {
      --neighbor_count;
   }

   static inline size_t 
   get_neighbor_count(void)
   {
      return neighbor_count;
   }

  boost::mpi::communicator *world = NULL;
  
  /*Helper Functions*/
  static const char* msgcmd2str[23];
  static boost::asio::ip::tcp::socket *my_tcp_socket;
  static void processMessage(message_type* reply);
  static void addReceivedTuple(serial_node *no, size_t ts, db::simple_tuple *stpl);
  static void addNeighbor(const size_t ts, serial_node *no, const node_val out, const face_t face, const int count);
  static void addNeighborCount(const size_t ts, serial_node *no, const size_t total, const int count);
  static void addVacant(const size_t ts,  serial_node *no, const face_t face, const int count);
  static void handleSetID(deterministic_timestamp ts, node::node_id node_id);
  static void handleReceiveMessage(const deterministic_timestamp ts, db::node::node_id node,
    const face_t face, db::node::node_id dest_id, utils::byte *data, int offset, const int limit);
  static void handleAddNeighbor(const deterministic_timestamp ts, const db::node::node_id in,
    const db::node::node_id out, const face_t face);
  static void handleRemoveNeighbor(const deterministic_timestamp ts,
    const db::node::node_id in, const face_t face);
  static void handleTap(const deterministic_timestamp ts, const db::node::node_id node);
  static void handleAccel(const deterministic_timestamp ts, const db::node::node_id node,
    const int_val f);
  static void handleShake(const deterministic_timestamp ts, const db::node::node_id node,
    const int_val x, const int_val y, const int_val z);
  static void   check_pre(sched::base *schedular);
  static bool isReady();
  static message_type *tcpPool();
  static void initTCP();
  static void handleDebugMessage(utils::byte* reply, size_t totalSize);
  static void sendMessageTCP(message *m);

	static void handleSetDeterministicMode(const deterministic_timestamp ts,
  const db::node::node_id node, const simulationMode mode);
	static void handleResumeComputation(const deterministic_timestamp ts,
  const db::node::node_id node, deterministic_timestamp duration);

  static bool ready(false);

  /*Stores the scheduler*/
  static sched::base *sched_state(NULL);
  vm::predicate* neighbor_pred(NULL);
  vm::predicate* tap_pred(NULL);
  vm::predicate* neighbor_count_pred(NULL);
  vm::predicate* accel_pred(NULL);
  vm::predicate* shake_pred(NULL);
  vm::predicate* vacant_pred(NULL);
  bool stop_all(false);
  //bool MessageReceived(false);
  static db::node::node_id id(0); 

  using namespace std;

/*Returns the nodeID*/
  int
  getNodeID(void){
    return id;
  }

/*To initialize the connection to the simulator */
  void 
  init(int argc, char **argv, sched::base* schedular)
  { 
    if (schedular == NULL) return;

    for (int i=0; i<25; i++) 
    msgcmd2str[i] = NULL;
    msgcmd2str[SETID] = "SETID";
    msgcmd2str[STOP] = "STOP";
    msgcmd2str[ADD_NEIGHBOR] = "ADD_NEIGHBOR";
    msgcmd2str[REMOVE_NEIGHBOR] = "REMOVE_NEIGHBOR";
    msgcmd2str[TAP] = "TAP";
    msgcmd2str[SET_COLOR] = "SET_COLOR";
    msgcmd2str[SEND_MESSAGE] = "SEND_MESSAGE";
    msgcmd2str[RECEIVE_MESSAGE] = "RECEIVE_MESSAGE";
    msgcmd2str[ACCEL] = "ACCEL";
    msgcmd2str[SHAKE] = "SHAKE";
    msgcmd2str[DEBUG] = "DEBUG";
    msgcmd2str[SET_DETERMINISTIC_MODE] = "SET_DETERMINISTIC_MODE";
    msgcmd2str[RESUME_COMPUTATION] = "RESUME_COMPUTATION";
    msgcmd2str[COMPUTATION_PAUSE] = "COMPUTATION_PAUSE";
    msgcmd2str[WORK_END] = "WORK_END";
    try{
    /* Calling the connect*/
      initTCP();
      check_pre(schedular);
      while(!isReady())
        pollAndProcess(NULL,NULL);
    } catch(std::exception &e) {
      throw machine_error("can't connect to simulator");
    }
 }

void debugInit(vm::all *all)
{
  /*Initilize the debugger*/
  return;
}


/*Checks the predicates to be used during execution*/
  void 
  check_pre(sched::base *schedular){

   sched_state=schedular;

  // cout<<"Setting the predicates"<<endl;  
   neighbor_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("neighbor");
   if(neighbor_pred) {
     assert(neighbor_pred->num_fields() == 2);
   } else {
  //   cerr << "No neighbor predicate found" << endl;
   }

   tap_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("tap");
   if(tap_pred) {
     assert(tap_pred->num_fields() == 0);
   } else {
  //   cerr << "No tap predicate found" << endl;
   }

   neighbor_count_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("neighborCount");
   if(neighbor_count_pred) {
     assert(neighbor_count_pred->num_fields() == 1);
   } else {
  //   cerr << "No neighbor_count predicate found" << endl;
   }

   accel_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("accel");
   if(accel_pred) {
     assert(accel_pred->num_fields() == 1);
   } else {
  //   cerr << "No accel predicate found" << endl;
   }

   shake_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("shake");
   if(shake_pred) {
     assert(shake_pred->num_fields() == 3);
   } else {
  //   cerr << "No shake predicate found" << endl;
   }

   vacant_pred = (schedular->state).all->PROGRAM->get_predicate_by_name("vacant");
   if(vacant_pred) {
     assert(vacant_pred->num_fields() == 1);
   } else {
  //   cerr << "No vacant predicate found" << endl;
   }
 }

/*Used in MPI, For BBSIM, used in the machine::route method*/
bool 
onLocalVM(const db::node::node_id id){
  return false;
}

/*Polls the socket for any message and processes the message*/
 bool 
 pollAndProcess(sched::base *sched, vm::all *all)
 {
  message_type *reply;

  /*Change the name of the poll function here*/
  if((reply =(message_type*)tcpPool()) == NULL) {
    if(ensembleFinished(sched_state))
      return false;
    else
      return true;
  }
  processMessage(reply);
  return true;
}


/*API function to send the SETCOLOR command to the simulator*/
void 
set_color(db::node *n, const int r, const int g, const int b)
{
  message* colorMessage=(message*)calloc(8, sizeof(message_type));

  cout<<n->get_id() << ":Sending SetColor"<<endl;

  colorMessage->size=7 * sizeof(message_type);
  colorMessage->command=SET_COLOR;
#ifdef SIMD
  colorMessage->timestamp=getCurrentLocalTime();
#else
  colorMessage->timestamp=0;
#endif  
  colorMessage->node=(message_type)n->get_id();
  colorMessage->data.color.r=r;
  colorMessage->data.color.g=g;
  colorMessage->data.color.b=b;
  colorMessage->data.color.i=0;

  sendMessageTCP(colorMessage);
  free(colorMessage);
}


  uint nbReceivedMsg = 0;
  void computationPause() {
    message* pauseComputationMessage = (message*)calloc(4, sizeof(message_type));
    pauseComputationMessage->size = 3 * sizeof(message_type);
    pauseComputationMessage->command = COMPUTATION_PAUSE;
    pauseComputationMessage->timestamp = (message_type) getCurrentLocalTime();
	pauseComputationMessage->node = 0; //(message_type)n->get_id();
    sendMessageTCP(pauseComputationMessage);
    free(pauseComputationMessage);
  }
  
   void workEnd() {
    message* workEndMessage = (message*)calloc(5, sizeof(message_type));
    workEndMessage->size = 4 * sizeof(message_type);
    workEndMessage->command = WORK_END;
    workEndMessage->timestamp = (message_type) getCurrentLocalTime();
	workEndMessage->node = 0; //(message_type)n->get_id();
	workEndMessage->data.workEnd.nbRecMsg = nbReceivedMsg;
    sendMessageTCP(workEndMessage);
    free(workEndMessage);
  }
  
  void timeInfo(db::node *n) {
    message* timeInfoMessage=(message*)calloc(4, sizeof(message_type));
    timeInfoMessage->size=3 * sizeof(message_type);
    timeInfoMessage->command = TIME_INFO;
    timeInfoMessage->timestamp = (message_type) getCurrentLocalTime();
    timeInfoMessage->node = 0; //(message_type)n->get_id();
    sendMessageTCP(timeInfoMessage);
    free(timeInfoMessage);
  }
  
  void handleSetDeterministicMode(const deterministic_timestamp ts,
    const db::node::node_id node, const simulationMode mode) {
		setDeterministicMode(mode);
  }

  void handleResumeComputation(const deterministic_timestamp ts,
    const db::node::node_id node, deterministic_timestamp duration) {
	 resumeComputation(ts, duration);
  }

/*Returns the node id for bbsimAPI*/
  int 
  getVMId(const db::node::node_id id)
  {
    return id;
  }

/*Used in MPI*/
 void 
 serializeBeginExec(void)
 {
    return;
 }
 
/*Used in MPI*/ 
 void serializeEndExec(void)
 {

 }

/*Sends the "SEND_MESSAGE" command*/
void
  sendMessage(const db::node* from, db::node::node_id to, db::simple_tuple* stpl)
  {
   //Find the tuple size
   const size_t stpl_size(stpl->storage_size());
 //  cout<<getNodeID<<":Stpl:"<<*stpl<<":Stpl size:"<<stpl_size<<endl;

//Compute the size field  
const size_t msg_size = 5 * sizeof(message_type) + stpl_size;

//Allocating the buffer for the message   msg_size + the space for msga->size field.
message* msga=(message*)calloc((msg_size+ sizeof(message_type)), 1);
 
 
   msga->size = (message_type)msg_size;
   msga->command = SEND_MESSAGE;
#ifdef SIMD
  msga->timestamp = getCurrentLocalTime();
#else
  msga->timestamp = 0;
#endif 
   msga->node = from->get_id();
   msga->data.send_message.face= 0; //(dynamic_cast<serial_node*>(from))->get_face(to);
   msga->data.send_message.dest_nodeID = to;
   cout << from->get_id() << " Send " << *stpl << "to "<< to<< endl;

/*Setting the position at the end of header to copy the tuple*/ 
int pos = 6 * sizeof(message_type);
  stpl->pack((utils::byte*)msga, msg_size + sizeof(message_type), &pos);
// cout<<"Message Size:"<< msg_size<<" Pos:"<<pos<<" Assertion:"<<msg_size+sizeof(message_type)<<endl;
  assert((size_t)pos == msg_size + sizeof(message_type));

  simple_tuple::wipeout(stpl);
  sendMessageTCP(msga);
  free(msga);
}


/*Flags if VM can run now*/
bool 
isReady()
{
 return ready;
}

void 
end(void)
{
  return;
}


/*tcp helper functions begin*/
static void 
initTCP()
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

static message_type *
tcpPool()
{
  static message_type msg[1024];
#ifdef SIMD
	if(mustQueueMessages()) return NULL;
#endif
  try {
    if(my_tcp_socket->available())
    {
      size_t length = my_tcp_socket->read_some(boost::asio::buffer(msg, sizeof(message_type)));
      //cout<<"getting message of length "<< length<<endl;
      length = my_tcp_socket->read_some(boost::asio::buffer(msg + 1,  msg[0]));
      //cout<<"Returning message of length "<< msg[0]<<endl;
      nbReceivedMsg++;
      return msg;   
    }
  } catch(std::exception &e) {
    cout<<"Could not recieve!"<<endl;
    return NULL;
  }
  return NULL;
}

/*Sends the message over the socket*/
static void 
sendMessageTCP(message *msg)
  {
    boost::asio::write(*my_tcp_socket, boost::asio::buffer(msg, msg->size + sizeof(message_type)));
  }


/*TCP helper functions end*/


/*Helper function Definitions*/

/*Handles the incoming commangs from the simulator*/
  static void 
  processMessage(message_type* reply)
  {
    printf("%d:Processing %s %lud bytes for %lud\n",id, msgcmd2str[reply[1]], reply[0], reply[3]);
    assert(reply!=NULL);
#ifdef SIMD
	if (reply[1] != DEBUG)
		setCurrentLocalTime((deterministic_timestamp)reply[2]);
#endif
    message* msg=(message*)reply;

    switch(msg->command) {
  /*Initilize the blocks's ID*/
      case SETID: 
      handleSetID((deterministic_timestamp) msg->timestamp, (db::node::node_id) msg->node);
      id=(db::node::node_id) reply[3];
      ready=true;
      cout << "ID received" << endl;
      break;

      case RECEIVE_MESSAGE:
      handleReceiveMessage((deterministic_timestamp)msg->timestamp,
        (db::node::node_id)msg->node,
        (face_t)msg->data.receiveMessage.face,
        (db::node::node_id)msg->data.receiveMessage.from,
        (utils::byte*)reply,
        6 * sizeof(message_type),
        (int)(msg->size + sizeof(message_type)));
      break;

      case ADD_NEIGHBOR:
      if(id==(db::node::node_id) msg->node)
        handleAddNeighbor((deterministic_timestamp)msg->timestamp,
         (db::node::node_id)msg->node,
         (db::node::node_id)msg->data.addNeighbor.nid,
         (face_t)msg->data.addNeighbor.face);
      break;

      case REMOVE_NEIGHBOR:
   // if(id==(db::node::node_id) reply[3])
      handleRemoveNeighbor((deterministic_timestamp)msg->timestamp,
        (db::node::node_id)msg->node,
        (face_t)msg->data.delNeighbor.face);
      break;

      case TAP:
      handleTap((deterministic_timestamp)msg->timestamp, (db::node::node_id)msg->node);
      break;

      case ACCEL:
      handleAccel((deterministic_timestamp)msg->timestamp,
       (db::node::node_id)msg->node,
       (int)reply[4]);
      break;

      case SHAKE:
      handleShake((deterministic_timestamp)msg->timestamp, (db::node::node_id)msg->node,
       (int)reply[4], (int)reply[5], (int)reply[6]);
      break;

      case DEBUG:
       handleDebugMessage((utils::byte*)reply, (size_t)reply[0]);
      break;

      case STOP:
      stop_all = true;
      sleep(1);
      usleep(200);
      break;

#ifdef SIMD      
      case SET_DETERMINISTIC_MODE:
		handleSetDeterministicMode((deterministic_timestamp)reply[2],
		 (db::node::node_id)reply[3], (simulationMode)reply[4]);
      break;
      
      case RESUME_COMPUTATION:
		 handleResumeComputation((deterministic_timestamp)reply[2],
		  (db::node::node_id)reply[3], (deterministic_timestamp)reply[4]);
      break;
#endif
      default: cerr << "Unrecognized message " << reply[1] << endl;
    }
  }

  bool
  ensembleFinished(sched::base *sched)
  {
   return stop_all;
 }

/*Adds the tuple to the node's work queue*/
 static void 
 addReceivedTuple(serial_node *no, size_t ts, db::simple_tuple *stpl)
 {
  if(ts>0){}

 work new_work(no, stpl);
 sched_state->new_work(no, new_work);

}

/*Add the neighbor to the block*/
static void 
addNeighbor(const size_t ts, serial_node *no, const node_val out, const face_t face, const int count)
{
 if(!neighbor_pred)
  return;

vm::tuple *tpl(new vm::tuple(neighbor_pred));
tpl->set_node(0, out);
tpl->set_int(1, static_cast<int_val>(face));

db::simple_tuple *stpl(new db::simple_tuple(tpl, count));

addReceivedTuple(no, ts, stpl);
}


static void 
addNeighborCount(const size_t ts, serial_node *no, const size_t total, const int count)
{
  if(!neighbor_count_pred)
    return;

  vm::tuple *tpl(new vm::tuple(neighbor_count_pred));
  tpl->set_int(0, (int_val)total);
  cout <<id<< ":Adding tuple:" << tpl << endl;

  db::simple_tuple *stpl(new db::simple_tuple(tpl, count));
  cout <<id<< ":Adding simple tuple:" << stpl << endl;

  addReceivedTuple(no, ts, stpl);
}

static void 
remove_neighbor_count(const size_t ts, serial_node *no, const size_t total, const int count)
{
   vm::tuple *tpl(new vm::tuple(neighbor_count_pred));
  tpl->set_int(0, (int_val)total);
  cout <<id<< ":Adding tuple:" << tpl << endl;

  db::simple_tuple *stpl(new db::simple_tuple(tpl, count));
  cout <<id<< ":Adding simple tuple:" << stpl << endl;

  addReceivedTuple(no, ts, stpl);
}

static void 
addVacant(const size_t ts,  serial_node *no, const face_t face, const int count)
{
 if(!vacant_pred)
  return;

vm::tuple *tpl(new vm::tuple(vacant_pred));
tpl->set_int(0, static_cast<int_val>(face));

db::simple_tuple *stpl(new db::simple_tuple(tpl, count));

addReceivedTuple(no, ts, stpl);
}


/*function to set the id of the block */
static void 
handleSetID(deterministic_timestamp ts, db::node::node_id node_id)
{
#ifdef DEBUG
 // cout << "Create node with " << node_id << endl;
#endif
  /*similar to create_n_nodes*/
  db::node *no((sched_state)->state.all->DATABASE->create_node_id(node_id));
  sched_state->init_node(no);

  serial_node *no_in((serial_node *)no);
    top=NO_NEIGHBOR;
    bottom=NO_NEIGHBOR;
    east=NO_NEIGHBOR;
    west=NO_NEIGHBOR;
    north=NO_NEIGHBOR;
    south=NO_NEIGHBOR;
    neighbor_count=0;

  instantiated_flag=true;
  for(face_t face = INITIAL_FACE; face <= FINAL_FACE; ++face) {
    addVacant(ts, no_in, face, 1);
  }

    addNeighborCount(ts, no_in, 0, 1);
}

static void handleReceiveMessage(const deterministic_timestamp ts,
  db::node::node_id dest_id,
  const face_t face, db::node::node_id node, utils::byte *data, int offset, const int limit)
{

  if(ts>0&&face==0&&node==0){}
 //  serial_node *origin(dynamic_cast<serial_node*>((sched_state->state).all->DATABASE->find_node(node)));
    serial_node *target(NULL);
  target=dynamic_cast<serial_node*>((sched_state->state).all->DATABASE->find_node(dest_id));

/*
   if(face == INVALID_FACE)
      target = origin;
   else
      target = dynamic_cast<serial_node*>((sched_state->state).all->DATABASE->find_node((db::node::node_id)*(origin->get_node_at_face(face))));
*/

    simple_tuple *stpl(simple_tuple::unpack(data, limit,
      &offset, (sched_state->state).all->PROGRAM));

#ifdef DEBUG
  //  cout << id<<":Received message from" << node << " to " << target->get_id() << " with Tuple" << *stpl << " with priority " << ts << endl;
#endif

    work new_work(target, stpl);
    sched_state->new_work(target, new_work);
 }

static void
handleDebugMessage(utils::byte* reply, size_t totalSize)
{
 size_t msgSize=totalSize/sizeof(message_type);
 message_type* m = (message_type*) reply;
 message* msg= (message*)calloc(msgSize+1, sizeof(message_type));
 memcpy(msg,reply, totalSize+sizeof(message_type));
 debugger::messageQueue->push((message_type*)msg);
}




static void 
  handleAddNeighbor(const deterministic_timestamp ts, const db::node::node_id in,
    const db::node::node_id out, const face_t face)
  {
#ifdef DEBUG
  // cout << id << ":Added neighbor("<<out << " on face " << face << ")" << endl;
#endif
   
   serial_node *no_in(dynamic_cast<serial_node*>((sched_state->state).all->DATABASE->find_node(in)));
   node_val *neighbor(get_node_at_face(face));

   if(*neighbor == NO_NEIGHBOR) {
      // remove vacant first, add 1 to neighbor count
    if(has_been_instantiated()) {
     addVacant(ts, no_in, face, -1);
     addNeighborCount(ts, no_in, get_neighbor_count(), -1);
   }
  inc_neighbor_count();
#ifdef DEBUG
   //cout << id << ":neighbor count=" << get_neighbor_count() << endl;
#endif
   if(has_been_instantiated())
     addNeighborCount(ts, no_in, get_neighbor_count(), 1);
   *neighbor = out;
   if(has_been_instantiated())
     addNeighbor(ts, no_in, out, face, 1);
 } else {
  if(*neighbor != out) {
         // remove old node
   if(has_been_instantiated())
    addNeighbor(ts, no_in, *neighbor, face, -1);
  *neighbor = out;
  if(has_been_instantiated())
    addNeighbor(ts, no_in, out, face, 1);
}
}
}

static void
handleRemoveNeighbor(const deterministic_timestamp ts,
  const db::node::node_id in, const face_t face)
{
  


serial_node *no_in(dynamic_cast<serial_node*>((sched_state->state).all->DATABASE->find_node(in)));
node_val *neighbor(get_node_at_face(face));

#ifdef DEBUG
//  cout << id << ":Remove neighbor(" << *neighbor << ", " << face << ")" << endl;
#endif
  if(*neighbor == NO_NEIGHBOR) {
      // remove vacant first, add 1 to neighbor count
    cerr << "Current face is vacant, cannot remove node!" << endl;
    assert(false);
  } else {
      // remove old node
    if(has_been_instantiated())
     addNeighborCount(ts, no_in, get_neighbor_count(), -1);
   dec_neighbor_count();
   addVacant(ts, no_in, face, 1);
   if(has_been_instantiated())
     addNeighborCount(ts, no_in, get_neighbor_count(), 1);
 }

 addNeighbor(ts, no_in, *neighbor, face, -1);

 *neighbor = NO_NEIGHBOR;
}

static void 
handleTap(const deterministic_timestamp ts, const db::node::node_id node)
{
 //cout << id << ":tap(" << node << ")" << endl;

 serial_node *no(dynamic_cast<serial_node*>((sched_state->state).all->DATABASE->find_node(node)));

 if(tap_pred) {
  vm::tuple *tpl(new vm::tuple(tap_pred));
  db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));

  addReceivedTuple(no, ts, stpl);
}
}

static void 
handleAccel(const deterministic_timestamp ts, const db::node::node_id node,
  const int_val f)
{
 //cout << id << ":accel(" << node << ", " << f << ")" << endl;

 serial_node *no(dynamic_cast<serial_node*>((sched_state->state).all->DATABASE->find_node(node)));

 if(accel_pred) {
  vm::tuple *tpl(new vm::tuple(accel_pred));
  tpl->set_int(0, f);

  db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));

  addReceivedTuple(no, ts, stpl);
}
}


static void 
handleShake(const deterministic_timestamp ts, const db::node::node_id node,
  const int_val x, const int_val y, const int_val z)
{
// cout << id << ":shake(" << node << ", " << x << ", " << y << ", " << z << ")" << endl;

 serial_node *no(dynamic_cast<serial_node*>((sched_state->state).all->DATABASE->find_node(node)));

 if(shake_pred) {
  vm::tuple *tpl(new vm::tuple(shake_pred));
  tpl->set_int(0, x);
  tpl->set_int(1, y);
  tpl->set_int(2, z);

  db::simple_tuple *stpl(new db::simple_tuple(tpl, 1));

  addReceivedTuple(no, ts, stpl);
}
}

/*Helper functions end*/

/*Debugger Messages*/
void 
debugGetMsgs(void)
{
  pollAndProcess(sched_state, debugger::all);
  return;
}

void 
debugBroadcastMsg(message_type *msg, size_t messageSize)
{}

void 
debugWaitMsg(void)
{}

/* Output the database in a synchronized manner */
 void 
 dumpDB(std::ostream &out, const db::database::map_nodes &nodes)
 {

 }
 
/*Print the database*/  
 void 
 printDB(std::ostream &out, const db::database::map_nodes &nodes)
 {

 }

void 
debugSendMsg(int destination,message_type* msg, size_t messageSize)
{
  sendMessageTCP((message*)msg);
  delete[] msg;
}
}

// Local Variables:
// tab-width: 4
// indent-tabs-mode: nil
// End:

