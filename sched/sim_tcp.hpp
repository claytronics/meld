#include <boost/asio.hpp>
#include <iostream>
#include <string>

namespace sim_tcp
{

typedef uint64_t message_type;
boost::asio::ip::tcp::socket *my_socket;


void init_tcp();
void send_message(message_type* msg);
message_type* poll();

}
