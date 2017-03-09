#include "sockets.h"


#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>


std::map <std::string, std::vector <std::string> > database;



int main() {
    int sockfd = create_socket();
    int port = getport(sockfd);
    std::stringstream ss;
    ss << port;
    std::string str_port = ss.str();
    std::string addr = getaddr(sockfd, str_port.c_str());
    std::cout << "BINDER_ADDRESS: " << addr << std::endl;
    std::cout << "BINDER_PORT: " << port << std::endl;
    while(1) {
        selection(sockfd);
        std::pair <int, std::string> request = respond();
        if (request.first != -1) {
            std::string mes = request.second;
            std::cout << mes << std::endl;
            
        }

    } 
}
