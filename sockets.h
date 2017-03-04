#include <string>

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>



int create_socket();

int getport(int sockfd);

std::string getaddr(int sockfd, const char* str_port);

void selection(int sockfd);



std::pair <int, std::string> respond();

void send_result(std::pair <int, std::string> result);

int connect(std::string &addr, int port);
