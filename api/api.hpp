#include <boost/asio.hpp>
#include <iostream>
#include <string>

#include <list>
#include <queue>

#include "sched/base.hpp"
#include "sched/nodes/sim.hpp"
#include "queue/safe_general_pqueue.hpp"
#include "db/node.hpp"
#include "db/tuple.hpp"


namespace api
{

typedef uint64_t message_type;


static const size_t MAXLENGTH = 512 / sizeof(api::message_type);

void init(sched::base *schedular);
void send_message(db::node* from, const db::node::node_id to, db::simple_tuple* stpl);
bool poll();
void set_color(db::node *n, const int r, const int g, const int b);
void check_pre(sched::base *scheduler);  
bool isReady();


  // only called if there is no local work
  bool ensembleFinished();	// for BBSIM, never finished unless STOP recieved
  				// for MPI, will do token ring passing
				// for serial sched always returns true
}
