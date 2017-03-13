#include <string>
#include <utility>
#include <iostream>
#include <pthread.h>
#include "rpc.h"
#include "sockets.h"
#include "server_lib.h"

std::string addr;
int port;
volatile int send_sockfd;
volatile int rec_sockfd;
pthread_mutex_t lock;
pthread_t response_thread;
bool active;

void* handle(void* data) {
    while(active) {
        pthread_mutex_lock(&lock);
        selection(-1);
        std::pair <int, std::string> result = respond();
        pthread_mutex_unlock(&lock);
    } 
    return NULL;
}

int rpcInit(void) {
    std::string addr = getenv("BINDER_ADDRESS");
    port = atoi(getenv("BINDER_PORT"));
    send_sockfd = connect(addr, port);

    if (send_sockfd == -1) {
        return -1;
    }
    active = true;
    std::string msg = "hello init success";

    pthread_create(&response_thread, NULL, &handle, NULL);
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_lock(&lock);
    send_result(std::make_pair(send_sockfd, msg));
    pthread_mutex_unlock(&lock);
    return 0;
}

int rpcRegister(char* name, int* argTypes, skeleton f) {
    std::string type = std::string("") + (char)REGISTER;
    std::cout << type << std::endl;
    std::string id = encode_length(addr.length()) + addr;
    std::string encoded_port = encode_int(port);
    std::string fname = encode_fname(name);
    std::string argt = encode_argtypes(argTypes);
    std::string enc = type + id + encoded_port + fname + argt;

    pthread_mutex_lock(&lock);
    send_result(std::make_pair(send_sockfd, enc));
    pthread_mutex_unlock(&lock);
    active = false;
    pthread_join(response_thread, NULL);
    pthread_mutex_destroy(&lock);

    return 0;
}

int rpcExecute(void) {
	while (1) {
		// update buffers to receive messages
		selection(rec_sockfd);
		// fetch the message in the read buffer
		std::pair <int, std::string> request = respond();
		if (request.first == -1) {
			// err ?
			return -1;
		}

		// get the actual message
		std::string msg = request.second;
		if (msg.length() < 7) {continue;} // invalid message

		if (msg.substr(0, 7) == "EXECUTE") {
			msg = msg.substr(7);
			std::string fname = decode_fname(msg);
			msg = msg.substr(sizeof(size_t) + fname.length());
			std::pair<size_t, int*> a = decode_argtypes(msg);
			int *argt = a.second;
			msg = msg.substr(sizeof(size_t) + a.first * sizeof(int*));
			void **args = decode_args(argt, msg);

			// find and call skeleton function
			skeleton f = funcs.at(std::make_pair(fname, argt));
			int result = f(argt, args);

			// now send result back to client
			std::string enc_type, ret_msg;
			if (result == 0) {
				enc_type = std::string("") + (char)EXECUTE_SUCCESS;
				std::string enc_fname = encode_fname(fname);
				std::string enc_argt = encode_argtypes(argt);
				std::string enc_args = encode_args(argt, args);
				ret_msg = enc_type + enc_fname + enc_argt + enc_args;
				send_result(std::make_pair(send_sockfd, ret_msg));
			}
			else {
				enc_type = std::string("") + (char)EXECUTE_FAILURE;
				std::string enc_reasonCode = encode_int(result);
				ret_msg = enc_type + enc_reasonCode;
				send_result(std::make_pair(send_sockfd, ret_msg));
			}
		}

		else if (msg == "TERMINATE") {
			// also need to verify IP and port 
			return 0;
		}
	}
	return 0;
}
