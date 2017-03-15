#include "rpc.h"
#include "sockets.h"


#include <iostream>
#include <string>
#include <sstream>

volatile int binder_sockfd;
pthread_mutex_t lock;
pthread_t response_thread;
bool active;

void* handle(void* data) {
	while(active) {
		pthread_mutex_lock(&lock);
		selection(-1);
		std::pair <int, std::string> result = respond();
		pthread_mutex_unlock(&lock);
        if (result.first != -1) {
            std::string msg = result.second;
        }
	} 
	return NULL;
}

int rpcCall(char *name, int *argTypes, void** args) {

	// open a connection with binder
	std::string binder_addr = getenv("BINDER_ADDRESS");
	int binder_port = atoi(getenv("BINDER_PORT"));
	binder_sockfd = connect(binder_addr, binder_port);

	if (binder_sockfd == -1) {
		return -1;
	}
	active = true;

	// encode loc_request message
	std::string type = std::string("") + (char)LOC_REQUEST;
	std::string enc_fname = encode_fname(name);
	std::string enc_argt = encode_argtypes(argTypes);
	std::string msg = type + enc_fname + enc_argt;
	std::cout << msg << std::endl;

	// send loc_request to binder
	pthread_create(&response_thread, NULL, &handle, NULL);
	pthread_mutex_init(&lock, NULL);
	pthread_mutex_lock(&lock);
	send_result(std::make_pair(binder_sockfd, msg));
	pthread_mutex_unlock(&lock);

			active = false;
			//pthread_join(response_thread, NULL);
			//pthread_mutex_destroy(&lock);

	std::cout << "loc_request sent to binder" << std::endl;

	// receive response from binder
	selection(binder_sockfd);
	std::pair <int, std::string> req = respond();
	if (req.first == -1) {
		return -1;
	}
    
    //handle this in handler
	// decode response message
	std::string req_result = req.second;
	std::cout << "result: " << req_result << std::endl;
	int type = decode_int(req_result);
	req_result.erase(0, sizeof(int));
	if (type == LOC_FAIL) {
		return -1;
	}
	else if (type == LOC_SUCC) {

	// now send procedure call to server
	std::string server_addr = binder_addr;
	int server_port = binder_port;
	int server_sockfd = connect(server_addr, server_port);

	if (server_sockfd == -1) {
		return -1;
	}
	active = true;

	type = std::string("") + (char)EXECUTE;
	std::string enc_args = encode_args(argTypes, args);
	msg = type + enc_fname + enc_argt + enc_args;
	std::cout << msg << std::endl;

	pthread_mutex_lock(&lock);
	send_result(std::make_pair(server_sockfd, msg));
	pthread_mutex_unlock(&lock);

			active = false;
			pthread_join(response_thread, NULL);
			pthread_mutex_destroy(&lock);

	std::cout << "procedure call sent to server" << std::endl;
	}

	return 0;
}

int rpcTermination( void ) {
	return 0;
}

int main() {
	int result, v;
	int *vec = &v;
	int argTypes[3];
	void **args = (void **)malloc(2  * sizeof(void *));
	argTypes[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);           // result
	argTypes[1] = (1 << ARG_INPUT)  | (ARG_INT << 16) | 23;  // vector
	argTypes[2] = 0;                                             // Terminator
	args[0] = (void *)&result;
	args[1] = (void *)vec;
	rpcCall("sum", argTypes, args);
	rpcTermination();
	return 0;
}
