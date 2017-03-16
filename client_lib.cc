#include "rpc.h"
#include "sockets.h"


#include <iostream>
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


void* handle2(void* data) {
    while(active2) {
        pthread_mutex_lock(&lock2);
        selection(-1);
        std::pair<int, std::string> result = respond();
        pthread_mutex_unlock(&lock2);
        if (result.first != -1) {
            std::string message = result.second;
        }
    }
    return NULL;
}

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

    pthread_create(&response_thread2, NULL, &handle2, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_mutex_lock(&lock2);
    send_result(std::make_pair(server_sockfd, message));
    pthread_mutex_unlock(&lock2);
     


    return 0;
}

int rpcTermination( void ) {
	return 0;
}

