#include <boost/asio.hpp>
#include <iostream>
#include <string>

#include <list>
#include <queue>

#include "sched/base.hpp"
#include "sched/nodes/sim.hpp"
#include "queue/safe_general_pqueue.hpp"

namespace bbsimapi
{



typedef uint64_t message_type;

boost::asio::ip::tcp::socket *tcp_socket;
static const size_t MAXLENGTH = 512 / sizeof(bbsimapi::message_type);

void init(sched::base *schedular);
static void init_tcp();
void send_message(message_type* msg);
message_type* poll();
static message_type *tcp_poll();

}
