#include <curses.h>
#include <panel.h>
#include <iostream>
#include <clocale>
#include <filesystem>
#include <vector>
#include <zmq.hpp>
#include <string>


#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


int main ()
{
	// create the zmq context
	zmq::context_t context (1);
	// create the socket that we will be binding
	zmq::socket_t tyr_socket (context, ZMQ_REP);
	// bind it to the ipc port/path that plugins will be communicating on
	tyr_socket.bind("ipc:///tmp/tyrplugins.ipc");
	std::cout << "socket bound!" << std::endl;
	// check for the client ready message
	zmq::message_t ready_message;
    tyr_socket.recv(&ready_message);
    std::string ready_message_contents = std::string(static_cast<char*>(ready_message.data()), ready_message.size());
    // check to make sure that the message was a READY message
    if (ready_message_contents == "READY"){
		std::cout << "received READY message"<< std::endl;
    	// send the READY message back
    	zmq::message_t  ready_reply (5);
    	memcpy(ready_reply.data(), "READY", 5);
    	tyr_socket.send(ready_reply);
    	// start sending and receiving messages from the client
		while (true){
			
		}	
    }
	
	return 0;
}