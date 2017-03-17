#include "rpc.h"
#include "sockets.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

volatile int binder_sockfd;
pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_t response_thread;
pthread_t response_thread2;

bool active;
bool active2;
std::string server_addr;
int server_port;

/*
void* handle2(void* data) {
    //std::cout << "server addr is : " << server_addr << std::endl;
    while(active2) {
        pthread_mutex_lock(&lock2);
        selection(-1);
        std::pair<int, std::string> result = respond();
        pthread_mutex_unlock(&lock2);
        if (result.first != -1) {
						std::cout << "handle2" << std::endl;
            std::string message = result.second;
								std::cout << "handle2 message: " << message << std::endl;
						int type = decode_int(message);
						message.erase(0,sizeof(int));
						if (type == EXECUTE_SUCC) {
								std::cout << "handle2 remain message: " << message << std::endl;
			// decode fname, argtypes, and args
			std::string fname2 = decode_fname(message);
			message.erase(0, sizeof(size_t) + fname2.length());
			std::pair<size_t, int*> a2 = decode_argtypes(message);
			std::cout << "sizeof argTypes: " << a2.first << std::endl;
			int *argt2 = a2.second;
			message.erase(sizeof(size_t) + a2.first * sizeof(int));
			void **args2 = decode_args(argt2, message);
            std::cout << "decoding done" << std::endl;
						std::cout << "result: " << ((int**)args2)[0] << std::endl;
        }
				}
    }
    return NULL;
}*/

void* handle(void* data) {
	while(active) {
		pthread_mutex_lock(&lock);
		selection(-1);
		std::pair <int, std::string> result = respond();
		pthread_mutex_unlock(&lock);
        if (result.first != -1) {
            std::string mes = result.second;
            std::cout << "encode msg: " << mes << std::endl;
            int type = decode_int(mes);
            mes.erase(0,sizeof(int));
            if (type == LOC_SUCCESS) {
                std::cout << "remaining msg without type: " << mes << std::endl;
                std::string addr = decode_fname(mes);
                server_addr = addr;
                mes.erase(0, sizeof(size_t) + addr.length());
                int port = decode_int(mes);
                server_port = port;
                std::cout << "LOC_SUCCESS: " << std::endl;
                std::cout << "addr: " << addr << std::endl;
                std::cout << "port: " << port << std::endl;
            }
            active = false;
        }
	} 
	return NULL;
}

int rpcCall(char *name, int *argTypes, void** args) {
    std::cout << "in rpccall" << std::endl;
	// open a connection with binder
	std::string binder_addr = getenv("BINDER_ADDRESS");
	int binder_port = atoi(getenv("BINDER_PORT"));
	binder_sockfd = connect(binder_addr, binder_port);
    
	if (binder_sockfd == -1) {

		return -1;
	}

	active = true;

	// encode loc_request message
	std::string type = encode_int(LOC_REQUEST);
	std::string enc_fname = encode_fname(name);
	std::string enc_argt = encode_argtypes(argTypes);
	std::string msg = type + enc_fname + enc_argt;


	// send loc_request to binder
	pthread_create(&response_thread, NULL, &handle, NULL);
	pthread_mutex_init(&lock, NULL);
	pthread_mutex_lock(&lock);
	send_result(std::make_pair(binder_sockfd, msg));
	pthread_mutex_unlock(&lock);
    pthread_join(response_thread, NULL);
    pthread_mutex_destroy(&lock);
    std::cout << "connecting to address: " << server_addr << std::endl;
    std::cout << "port: " << server_port << std::endl; 
    int server_sockfd = connect(server_addr, server_port);
    std::cout << "server sockfd is : " << server_sockfd << std::endl;
    if (server_sockfd == -1) return -1;
    active2 = true;
    type = encode_int(EXECUTE);
    std::string fname = encode_fname(name);
    std::string argtyps = encode_argtypes(argTypes);
    std::string ars = encode_args(argTypes, args);
    std::string message = type + fname + argtyps + ars;

		if (message != "") {
				msg = message;
            std::cout << "decoding begin" << std::endl;
						msg.erase(0, sizeof(int));
			std::string fname = decode_fname(msg);
			std::cout << fname << std::endl;
			msg.erase(0, sizeof(size_t) + fname.length());
			std::pair<size_t, int*> a = decode_argtypes(msg);
			std::cout << "sizeof argTypes: " << a.first << std::endl;
			int *argt = a.second;
			msg.erase(0, sizeof(size_t) + a.first * sizeof(int));
			void **argss = decode_args(argTypes, msg);
            std::cout << "decoding done" << std::endl;
	std::cout << "argTypes array is ";
	for (int i=0; argt[i]!=0; i++) {
			std::cout << argt[i] << "  ";
	}
	std::cout << "0" << std::endl;
						for (int i=0; i<a.first-1; i++) {
								std::cout << "args[" << i << "]: " << *(((int**)args)[i]) << std::endl;
								std::cout << "argss[" << i << "]: " << *(((int**)argss)[i]) << std::endl;
						}
		}


	sleep(2);
    //pthread_create(&response_thread2, NULL, &handle2, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_mutex_lock(&lock2);
    send_result(std::make_pair(server_sockfd, message));
    pthread_mutex_unlock(&lock2);
		std::cout << "execution request sent" << std::endl;	
		while (active2) {
        pthread_mutex_lock(&lock2);
        selection(-1);
        std::pair<int, std::string> result = respond();
						//std::cout << "getting response from server" << std::endl;
        pthread_mutex_unlock(&lock2);
        if (result.first != -1) {
            std::string message = result.second;
								std::cout << "server response message: " << message << std::endl;
						int type = decode_int(message);
						message.erase(0,sizeof(int));
						if (type == EXECUTE_SUCC) {
								std::cout << "EXECUTE_SUCC remain message: " << message << std::endl;
			// decode fname, argtypes, and args
			std::string fname2 = decode_fname(message);
			message.erase(0, sizeof(size_t) + fname2.length());
			std::pair<size_t, int*> a2 = decode_argtypes(message);
			std::cout << "sizeof argTypes: " << a2.first << std::endl;
			int *argt2 = a2.second;
			message.erase(sizeof(size_t) + a2.first * sizeof(int));
			void **args2 = decode_args(argt2, message);
            std::cout << "decoding done" << std::endl;
						std::cout << "result: " << *(((int**)args2)[0]) << std::endl;

						}
						active2 = false;
				}
		}
    return 0;
}

int rpcTermination( void ) {
	return 0;
}

