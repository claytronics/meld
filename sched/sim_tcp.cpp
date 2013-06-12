#include <iostream>
#include <boost/asio.hpp>
#include <string>

using boost::asio::ip::tcp;

using namespace std;

typedef uint64_t message_type;
tcp::socket *my_socket;

/* make the connection with the simulator */
void init_tcp()
{
	try {
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(tcp::v4(), "127.0.0.1", "5000");
		tcp::resolver::iterator iterator = resolver.resolve(query);

		my_socket = new tcp::socket(io_service);
		my_socket->connect(*iterator);
	} catch(std::exception &e) {
		cout<<"Could not connect!"<<endl;
	}
}

void send_message(message_type *msg)
{
	boost::asio::write(*my_socket, boost::asio::buffer(msg, msg[0] + sizeof(message_type)));
}

/* poll to the simulator to receive message */
message_type *poll()
{
	message_type msg[1024];
	try {
		if(my_socket->available())
		{
			size_t length = my_socket->read_some(boost::asio::buffer(msg, sizeof(message_type)));
			length = my_socket->read_some(boost::asio::buffer(msg + 1,  msg[0]));
			return msg;		
		}
	} catch(std::exception &e) {
		cout<<"Could not recieve!"<<endl;
		return NULL;
	}
	return NULL;
}
