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

using namespace db;
using namespace vm;
using namespace utils;
using namespace process;
using namespace debugger;
using boost::asio::ip::tcp;
using sched::serial_node;
using sched::base;
using namespace sched;
using namespace msg;

#define SETID 1
#define DEBUG 3
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
  static const char* msgcmd2str[16];
  static boost::asio::ip::tcp::socket *my_tcp_socket;
  static void processMessage(message_type* reply);
  static void add_received_tuple(serial_node *no, size_t ts, db::simple_tuple *stpl);
  static void add_neighbor(const size_t ts, serial_node *no, const node_val out, const face_t face, const int count);
  static void add_neighbor_count(const size_t ts, serial_node *no, const size_t total, const int count);
  static void add_vacant(const size_t ts,  serial_node *no, const face_t face, const int count);
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
  static void sendMessageTCP(message_type *msg);
  static void handleDebugMessage(utils::byte* reply, size_t totalSize);
  static void sendMessageTCP1(message *m);

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

    for (int i=0; i<16; i++) 
    msgcmd2str[i] = NULL;
    msgcmd2str[SETID] = "SETID";
    msgcmd2str[DEBUG] = "DEBUG";
    msgcmd2str[STOP] = "STOP";
    msgcmd2str[ADD_NEIGHBOR] = "ADD_NEIGHBOR";
    msgcmd2str[REMOVE_NEIGHBOR] = "REMOVE_NEIGHBOR";
    msgcmd2str[TAP] = "TAP";
    msgcmd2str[SET_COLOR] = "SET_COLOR";
    msgcmd2str[SEND_MESSAGE] = "SEND_MESSAGE";
    msgcmd2str[RECEIVE_MESSAGE] = "RECEIVE_MESSAGE";
    msgcmd2str[ACCEL] = "ACCEL";
    msgcmd2str[SHAKE] = "SHAKE";

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

//  cout<<"In setcolor"<<endl;
  cout<<n->get_id() << ":Sending SetColor"<<endl;

  colorMessage->size=7 * sizeof(message_type);
  colorMessage->command=SET_COLOR;
  colorMessage->timestamp=0;
  colorMessage->node=(message_type)n->get_id();
  colorMessage->data.color.r=r;
  colorMessage->data.color.g=g;
  colorMessage->data.color.b=b;
  colorMessage->data.color.i=0;

  sendMessageTCP1(colorMessage);
  free(colorMessage);
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
   
   cout<<"in SendMessage"<<endl;
   const size_t stpl_size(stpl->storage_size());
   const size_t msg_size = 5 * sizeof(message_type) + stpl_size;
   message* msga=(message*)calloc((msg_size+ sizeof(message_type)), 1);
  //Something to represent destination node.
   size_t i = 0;
   msga->size = (message_type)msg_size;
   msga->command = SEND_MESSAGE;
   msga->timestamp = 0;//(message_type)ts;
   msga->node = from->get_id();
   msga->data.send_message.face= 0; //(dynamic_cast<serial_node*>(from))->get_face(to);
   msga->data.send_message.dest_nodeID = to;
   cout << from->get_id() << " Send " << *stpl << "to "<< to<< endl;
  int pos = 6 * sizeof(message_type);
  stpl->pack((utils::byte*)msga, msg_size + sizeof(message_type), &pos);

  assert((size_t)pos == msg_size + sizeof(message_type));

  simple_tuple::wipeout(stpl);
  sendMessageTCP1(msga);
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
  try {
    if(my_tcp_socket->available())
    {
      size_t length = my_tcp_socket->read_some(boost::asio::buffer(msg, sizeof(message_type)));
      //cout<<"getting message of length "<< length<<endl;
      length = my_tcp_socket->read_some(boost::asio::buffer(msg + 1,  msg[0]));
      //cout<<"Returning message of length "<< msg[0]<<endl;
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
sendMessageTCP(message_type *msg)
  {
    boost::asio::write(*my_tcp_socket, boost::asio::buffer(msg, msg[0] + sizeof(message_type)));
  }

  static void 
sendMessageTCP1(message *msg)
  {
    cout<<"In send tcp1, size is="<<msg->size<<endl;
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

    switch(reply[1]) {
  /*Initilize the blocks's ID*/
      case SETID: 
      handleSetID((deterministic_timestamp) reply[2], (db::node::node_id) reply[3]);
      id=(db::node::node_id) reply[3];
      ready=true;
      break;

      case RECEIVE_MESSAGE:
      handleReceiveMessage((deterministic_timestamp)reply[2],
        (db::node::node_id)reply[3],
        (face_t)reply[4],
        (db::node::node_id)reply[5],
        (utils::byte*)reply,
        6 * sizeof(message_type),
        (int)(reply[0] + sizeof(message_type)));
      break;

      case ADD_NEIGHBOR:
      if(id==(db::node::node_id) reply[3])
        handleAddNeighbor((deterministic_timestamp)reply[2],
         (db::node::node_id)reply[3],
         (db::node::node_id)reply[4],
         (face_t)reply[5]);
      break;

      case REMOVE_NEIGHBOR:
   // if(id==(db::node::node_id) reply[3])
      handleRemoveNeighbor((deterministic_timestamp)reply[2],
        (db::node::node_id)reply[3],
        (face_t)reply[4]);
      break;

      case TAP:
      handleTap((deterministic_timestamp)reply[2], (db::node::node_id)reply[3]);
      break;

      case ACCEL:
      handleAccel((deterministic_timestamp)reply[2],
       (db::node::node_id)reply[3],
       (int)reply[4]);
      break;

      case SHAKE:
      handleShake((deterministic_timestamp)reply[2], (db::node::node_id)reply[3],
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
 add_received_tuple(serial_node *no, size_t ts, db::simple_tuple *stpl)
 {
  if(ts>0){}

 work new_work(no, stpl);
 sched_state->new_work(no, new_work);

}

/*Add the neighbor to the block*/
static void 
add_neighbor(const size_t ts, serial_node *no, const node_val out, const face_t face, const int count)
{
 if(!neighbor_pred)
  return;

vm::tuple *tpl(new vm::tuple(neighbor_pred));
tpl->set_node(0, out);
tpl->set_int(1, static_cast<int_val>(face));

db::simple_tuple *stpl(new db::simple_tuple(tpl, count));

add_received_tuple(no, ts, stpl);
}


static void 
add_neighbor_count(const size_t ts, serial_node *no, const size_t total, const int count)
{
  if(!neighbor_count_pred)
    return;

  vm::tuple *tpl(new vm::tuple(neighbor_count_pred));
  tpl->set_int(0, (int_val)total);
  cout <<id<< ":Adding tuple:" << tpl << endl;

  db::simple_tuple *stpl(new db::simple_tuple(tpl, count));
  cout <<id<< ":Adding simple tuple:" << stpl << endl;

  add_received_tuple(no, ts, stpl);
}

static void 
remove_neighbor_count(const size_t ts, serial_node *no, const size_t total, const int count)
{
   vm::tuple *tpl(new vm::tuple(neighbor_count_pred));
  tpl->set_int(0, (int_val)total);
  cout <<id<< ":Adding tuple:" << tpl << endl;

  db::simple_tuple *stpl(new db::simple_tuple(tpl, count));
  cout <<id<< ":Adding simple tuple:" << stpl << endl;

  add_received_tuple(no, ts, stpl);
}

static void 
add_vacant(const size_t ts,  serial_node *no, const face_t face, const int count)
{
 if(!vacant_pred)
  return;

vm::tuple *tpl(new vm::tuple(vacant_pred));
tpl->set_int(0, static_cast<int_val>(face));

db::simple_tuple *stpl(new db::simple_tuple(tpl, count));

add_received_tuple(no, ts, stpl);
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
 // cout<<"Node id is "<<no->get_id()<<endl;
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
    add_vacant(ts, no_in, face, 1);
  }

    add_neighbor_count(ts, no_in, 0, 1);
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
  size_t msgSize=totalSize/sizeof(message_type)-3;

  message_type* msg= new message_type[msgSize];
  int position=4*sizeof(message_type);

  utils::unpack<message_type>(reply, totalSize+sizeof(message_type), &position, msg, msgSize);
  //messageQueue->push(msg);
  
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
     add_vacant(ts, no_in, face, -1);
     add_neighbor_count(ts, no_in, get_neighbor_count(), -1);
   }
  inc_neighbor_count();
#ifdef DEBUG
   //cout << id << ":neighbor count=" << get_neighbor_count() << endl;
#endif
   if(has_been_instantiated())
     add_neighbor_count(ts, no_in, get_neighbor_count(), 1);
   *neighbor = out;
   if(has_been_instantiated())
     add_neighbor(ts, no_in, out, face, 1);
 } else {
  if(*neighbor != out) {
         // remove old node
   if(has_been_instantiated())
    add_neighbor(ts, no_in, *neighbor, face, -1);
  *neighbor = out;
  if(has_been_instantiated())
    add_neighbor(ts, no_in, out, face, 1);
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
     add_neighbor_count(ts, no_in, get_neighbor_count(), -1);
   dec_neighbor_count();
   add_vacant(ts, no_in, face, 1);
   if(has_been_instantiated())
     add_neighbor_count(ts, no_in, get_neighbor_count(), 1);
 }

 add_neighbor(ts, no_in, *neighbor, face, -1);

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

  add_received_tuple(no, ts, stpl);
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

  add_received_tuple(no, ts, stpl);
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

  add_received_tuple(no, ts, stpl);
}
}
/*Helper functions end*/

/*Debugger Messages*/
void 
debugGetMsgs(void)
{
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
debugSendMsg(int destination,
                             message_type* msg, size_t messageSize)
{
  size_t datasize=messageSize+4;
  message_type *data=new message_type[datasize];
  size_t i(0);
  
  data[i++] = datasize - sizeof(message_type);
  data[i++] = DEBUG;
  data[i++] = 0;
  data[i++] = (message_type)destination;
  int pos=i * sizeof(message_type);
  utils::pack<message_type>((void*)msg, messageSize, (utils::byte*)data, datasize, &pos);
  sendMessageTCP(data);
  delete []data;
  delete[] msg;

}
}

// Local Variables:
// tab-width: 4
// indent-tabs-mode: nil
// End:

