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

message_type *poll()
{
	message_type msg[1024];
	bool flag = true;
	try {
		/* keep polling till message received */		
		//while(flag)
	//	{
			if(my_socket->available())
			{
				size_t length = my_socket->read_some(boost::asio::buffer(msg, sizeof(message_type)));
				length = my_socket->read_some(boost::asio::buffer(msg + 1,  msg[0]));
				//flag = false;
				return msg;		
			}
	//	}
	} catch(std::exception &e) {
		cout<<"Could not recieve!"<<endl;
		return NULL;
	}
		return NULL;
	//return msg;
}
/*
int main()
{
	message_type message[] = {56,8,0,0,255,0,0,0};
	make_connection();
//	cout<<"connection established!"<<endl;

	message_type *recd_msg = poll();
//	cout<<"message received:";

	int i = recd_msg[0]/8;
	int j=0;
	while(j !=(i+1) )
	{
		cout<<recd_msg[j]<< "i="<<i<<endl;
		j++;
	}

	message[3]=recd_msg[3];
	send_message(message);
//	cout<<"message sent!"<<endl;
//	cout<<endl;
	return 0;
}
*/
