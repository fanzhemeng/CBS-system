#include "sockets.h"


#include <iostream>
#include <string>
#include <sstream>




int main() {
    int sockfd = create_socket();
    int port = getport(sockfd);
    std::stringstream ss;
    ss << port;
    std::string str_port = ss.str();
    std::string addr = getaddr(sockfd, str_port.c_str());
    std::cout << "BINDER_ADDRESS: " << addr << std::endl;
    std::cout << "BINDER_PORT: " << port << std::endl;
    
}
