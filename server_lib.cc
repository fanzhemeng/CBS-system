#include <string>
#include <map>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include "rpc.h"
#include "sockets.h"
#include "server_lib.h"

std::string addr;
int port;
volatile int binder_sockfd;
volatile int client_sockfd;
pthread_mutex_t lock;
pthread_t response_thread;
bool active;
typedef std::map< std::pair< std::string, std::vector<int> >, skeleton> tab;
tab ftable;

typedef std::vector <int> argtypes;
bool operator < (const argtypes &argTypes1, const argtypes &argTypes2) {
    int l1 = argTypes1.size();
    int l2 = argTypes2.size();
    if (l1 < l2) return true;
    else if (l1 > l2) return false;
    else {
        for(int i = 0; i < l1; i++) {
            if (argTypes1[i] < argTypes2[i]) return true;
            else if (argTypes1[i] > argTypes2[i]) return false;
        }
        return false;
    }
}


void* handle(void* data) {
    while(active) {
        pthread_mutex_lock(&lock);
        selection(-1);
        std::pair <int, std::string> result = respond();
        pthread_mutex_unlock(&lock);
        if (result.first != -1) {
            std::cout << "Binder: " << result.second << std::endl;
        }
    } 
    return NULL;
}

int rpcInit(void) {
	// create connection socket used for communacate with clients
	client_sockfd = create_socket();
	port = getport(client_sockfd);
	std::stringstream ss;
	ss << port;
	std::string str_port = ss.str();
	addr = getaddr(client_sockfd, str_port.c_str());

	// open a connection with binder
    std::string binder_addr = getenv("BINDER_ADDRESS");
	int binder_port = atoi(getenv("BINDER_PORT"));
    binder_sockfd = connect(binder_addr, binder_port);

    if (binder_sockfd == -1) {
        return -1;
    }
    return 0;
}

int rpcRegister(char* name, int* argTypes, skeleton f) {
	// encode REGISTER message
    std::string type = encode_int(REGISTER);

    std::string id = encode_length(addr.length()) + addr;

    std::string encoded_port = encode_int(port);
    std::string fname = encode_fname(name);
    std::string argt = encode_argtypes(argTypes);
    size_t l = length_of_argtypes(argTypes); 
    std::string enc = type + id + encoded_port + fname + argt;

	// save skeleton f in local table
	std::vector<int> argt_v;
	for (int i=0; argTypes[i]!=0; i++)
			argt_v.push_back(argTypes[i]);
	argt_v.push_back(0);
	ftable[std::make_pair(name, argt_v)] = f;
    active = true;
    pthread_create(&response_thread, NULL, &handle, NULL);
    pthread_mutex_init(&lock, NULL);

	// send REGISTER message
    pthread_mutex_lock(&lock);
    send_result(std::make_pair(binder_sockfd, enc));
    pthread_mutex_unlock(&lock);
	sleep(1);
    active = false;
    pthread_join(response_thread, NULL);
	return 0;
}

int rpcExecute(void) {
		std::cout << "hello out while" << std::endl;
	while (1) {
		// update buffers to receive messages
		selection(client_sockfd);
		// fetch the message in the read buffer
		std::pair <int, std::string> request = respond();
		if (request.first == -1) { continue; }

		std::cout << "hello" << std::endl;
		// get the actual message and its type
		std::string msg = request.second;
        std::cout << "mes: " << msg << std::endl;
		int msgtype = decode_int(msg);
		msg.erase(0, sizeof(int));
		if (msgtype == EXECUTE) {
            std::cout << "receiving an execute request" << std::endl;
			// decode fname, argtypes, and args
			std::string fname = decode_fname(msg);
			msg.erase(0, sizeof(size_t) + fname.length());
			std::pair<size_t, int*> a = decode_argtypes(msg);
			std::cout << "sizeof argTypes: " << a.first << std::endl;
			int *argt = a.second;
			msg = msg.substr(sizeof(size_t) + a.first * sizeof(int));
			void **args = decode_args(argt, msg);
            std::cout << "decoding done" << std::endl;
	std::vector<int> argt_v2;
	for (int i=0; argt[i]!=0; i++) {
			argt_v2.push_back(argt[i]);
			std::cout << argt[i+1] << "  ";
	}
	argt_v2.push_back(0);
	//std::cout << "0" << std::endl;

	std::cout << "hello ftable " << std::endl;
	tab::iterator tit;
	for (tit=ftable.begin(); tit!=ftable.end(); tit++) {
			std::cout << tit->first.first << "  ";
			std::vector<int> a = tit->first.second;
			for (std::vector<int>::iterator iit=a.begin(); iit!=a.end(); iit++)
					std::cout << *iit << " ";
			std::cout << std::endl;
	}
			// find and call skeleton function
			skeleton f = ftable.at(std::make_pair(fname, argt_v2));
	std::cout << "done ftable " << std::endl;
			int result = f(argt, args);

			// now send result back to client
			std::string enc_type, ret_msg;
			if (result == 0) {
				enc_type = std::string("") + (char)EXECUTE_SUCC;
				std::string enc_fname = encode_fname(fname);
				std::string enc_argt = encode_argtypes(argt);
				std::string enc_args = encode_args(argt, args);
				ret_msg = enc_type + enc_fname + enc_argt + enc_args;
				send_result(std::make_pair(client_sockfd, ret_msg));
			}
			else {
				enc_type = std::string("") + (char)EXECUTE_FAIL;
				std::string enc_reasonCode = encode_int(result);
				ret_msg = enc_type + enc_reasonCode;
				send_result(std::make_pair(client_sockfd, ret_msg));
			}
		}
        /*
		else if (msgtype == TERMINATE) {
			// also need to verify IP and port 
			active = false;
			pthread_join(response_thread, NULL);
			pthread_mutex_destroy(&lock);
			return 0;
		}*/
	}
	return 0;
}
