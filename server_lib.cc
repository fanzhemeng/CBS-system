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

