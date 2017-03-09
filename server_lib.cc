#include <string>
#include "rpc.h"
#include "sockets.h"
#include "server_lib.h"

#include <pair>


std::string addr;
int port;
volatile int send_sockfd;
volatile int rec_sockfd;



int rpcInit(void) {
    std::string addr = getenv("BINDER_ADDRESS");
    port = atoi(getenv("BINDER_PORT"));
    send_sockfd = connect(addr, port); 
    if (send_sockfd == -1) {
        return -1;
    }
    rec_sockfd = create_socket();
    return 0;
}

int rpcRegister(char* name, int* argTypes, skeleton f) {
    std::string type = std::string("") + (char)REGISTER;
    std::string id = encode_length(addr.length()) + addr;
    std::string port = encode_int(port);
    std::string fname = encode_fname(name);
    std::string argt = encode_argtypes(argTypes);
    std::string enc = type + id + port + fname + argt;
    send_result(std::makepair(send_sockfd, enc));
    return 0;
}

