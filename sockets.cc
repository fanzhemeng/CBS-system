#include <set>
#include <string>

#include <stdlib.h>
#include <netdb.h>
#include <memory.h>
#include <map>
#include <algorithm>
#include <iostream>

#include "sockets.h"
#include "rpc.h"
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CONNECTION 5
const size_t MAX_BUFFER = 2048;
std::set <int> sfdset;
std::map <int, std::string> read_buf, write_buf;
std::map <int, std::string> requests;
std::map <int, size_t> lengths;

std::map <unsigned int, size_t> size_lookup;

void init() {
    size_lookup[1] = sizeof(char);
    size_lookup[2] = sizeof(short);
    size_lookup[3] = sizeof(int);
    size_lookup[4] = sizeof(long);
    size_lookup[5] = sizeof(double);
    size_lookup[6] = sizeof(float);
}


std::string encode(unsigned char* p, size_t len) {
    std::string str;
    for (size_t i = 0; i < len; i ++)
        str += p[i];
    return str;
}

void decode(unsigned char* p, std::string str, size_t len) {
    for (size_t i = 0; i < len; i ++) {
        p[i] = str[i];
    }
}

std::string encode_length(size_t len) {
    return encode((unsigned char*)&len, sizeof(len));
}

size_t decode_length(std::string str) {
    size_t length;
    decode((unsigned char*)&length, str, sizeof(length));
    return length;
} 

std::string encode_int(int num) {
    return encode((unsigned char*)&num, sizeof(num));
}

int decode_int(std::string str) {
    int n;
    decode((unsigned char*)&n, str, sizeof(n));
    return n;
}

size_t length_of_argtypes(int* argtypes) {
    size_t i;
    for (i = 0; argtypes[i] != 0; i ++);
    return i+1;
}

std::string encode_argtypes(int* argtypes) {
    size_t length = length_of_argtypes(argtypes);
    return encode_length(length) + encode((unsigned char*)argtypes, sizeof(int) * length);
}

std::pair <size_t, int*> decode_argtypes(std::string str) {
    std::pair <size_t, int*> result;
    size_t length = decode_length(str);
    str.erase(0, sizeof(length));
    int* argtypes = new int[length];
    decode((unsigned char*)argtypes, str, length * sizeof(int));
    result.second = argtypes;
    result.first = length;
    return result;
}

std::string encode_fname(std::string fname) {
    size_t length = fname.length();
    return encode_length(length) + fname;
}

std::string decode_fname(std::string str) {
    size_t length = decode_length(str);
    str.erase(0, sizeof(length));
    std::string fname = str.substr(0,length);
    return fname;
}

std::pair <size_t, std::string> decode_socket(std::string str) {
    std::pair <size_t, std::string> result;
    size_t length = decode_length(str);
    str.erase(0, sizeof(length));
    result.first = length;
    result.second = str;
    return result;
}

std::string encode_args(int* argtypes, void** args) {
    size_t len = length_of_argtypes(argtypes) -1;

    std::string str;
    for (int i = 0; i < len; i ++) {
        unsigned int array_length = std::max((argtypes[i] & 0xffff), 1);
        unsigned int type = (argtypes[i] >> 16) & 0xff;
        //std::cout << "array lenth: " << array_length << std::endl;
        //std::cout << "type: " << size_lookup[type] << std::endl;
        str += encode((unsigned char*)args[i], size_lookup[type] * array_length);
        //std::cout << "size : " << str.length() << std::endl;
    }
    return str;
}

void ** decode_args(int* argtypes, std::string str) {
    size_t len = length_of_argtypes(argtypes)-1;
    void **args = new void*[len];
    for (int i = 0; i < len; i ++) {
        unsigned int array_len = std::max((argtypes[i] & 0xffff), 1);
        unsigned int type = (argtypes[i] >> 16) & 0xff;
        //std::cout << "array lenth: " << array_len << std::endl;
        //std::cout << "type: " << size_lookup[type] << std::endl;
        args[i] = new unsigned char*[array_len * size_lookup[type]];
        decode((unsigned char*)args[i], str, array_len * size_lookup[type]);
        //std::cout << "decoding done" << std::endl;
        str.erase(0, array_len * size_lookup[type]);
    }
    return args;
}


int create_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(0);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        close(sockfd);
        return -1;
    }
    if (listen(sockfd, MAX_CONNECTION) < 0) {
        close(sockfd);
        return -1;
    }
    sfdset.insert(sockfd);
    return sockfd;
}

int getport(int sockfd) {
    int port = -1;
    struct sockaddr_in saddr;
    socklen_t len = sizeof(saddr); 
    if(getsockname(sockfd, (struct sockaddr*)&saddr, &len) != 0) {
        return port;
    }
    port = ntohs(saddr.sin_port);
    return port;
}

std::string getaddr(int sockfd, const char* str_port) {
    std::string addr = "";
    struct addrinfo hints;
    struct addrinfo *servinfo;
    memset(&hints, 0, sizeof(hints));
    char name[256];
    gethostname(name, sizeof(name));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    if (getaddrinfo(name, str_port, &hints, &servinfo) != 0) {
        return addr;
    }
    addr = servinfo->ai_canonname;
    return addr;
}

void selection(int sockfd) {

    fd_set read_fds, write_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    for (std::set <int>::iterator it = sfdset.begin(); it != sfdset.end(); it ++) {
        if (*it != sockfd) {

            FD_SET(*it, &read_fds);
            FD_SET(*it, &write_fds);
        }
    }

    if (sockfd > 0) FD_SET(sockfd, &read_fds);

    char buf[MAX_BUFFER];
    int numfds = *(sfdset.rbegin());

    if (select(numfds + 1, &read_fds, &write_fds, NULL, NULL) == -1) {

        return;
    }

    for (std::set <int>::iterator it = sfdset.begin(); it != sfdset.end(); it ++) {
        if (FD_ISSET(*it, &read_fds)) {
			//std::cout << "one fd in read" << std::endl;
            if (*it == sockfd) {
				//std::cout << "server" << std::endl;
                int new_fd = accept(sockfd, NULL, NULL);
                sfdset.insert(new_fd);
            }
            else {
				//std::cout << "client" << std::endl;
				//std::cout << "fd is : " << *it << std::endl;
                size_t len = read(*it, &buf, sizeof(buf));
				//std::cout << "string len is: " << std::endl;
                if (len > 0) {
                    for (int i = 0;  i < len; i ++) {
                        read_buf[*it] += buf[i];
                    }
                }
				//std::cout << "message read is: " << read_buf[*it] << std::endl;
                if (len == 0) {
                    std::cout << "close fd: " << *it << std::endl;
                    sfdset.erase(it);
                    close(*it);
                }
            }
        }

        if (FD_ISSET(*it, &write_fds)) {
            std::string message = write_buf[*it];
            if (message.size() > 0) {
                size_t len = std::min(message.size(), MAX_BUFFER);
                if (len > 0) {
                    for (size_t i = 0; i < len; i ++) {
                        buf[i] = message[i];
                    }
                    write(*it, buf, len);
                }
                write_buf[*it].erase(0, len);
            }
        }

    }
}

std::pair <int, std::string> respond() {
    for (std::map<int, std::string>::iterator it = read_buf.begin(); it != read_buf.end(); it ++) {

        if ((it->second).size() > 0) {
            //std::cout << "find a request" << std::endl;
            if (requests.find(it->first) == requests.end()) {
                //std::cout << "request not in set" << std::endl;
                std::pair <size_t, std::string> de_str = decode_socket(it->second);
                requests[it->first] = de_str.second;
                lengths[it->first] = de_str.first;
            }
            else requests[it->first] += it->second;
            it->second.clear();
            //std::cout << "recieved len: " << requests[it->first].length() << std::endl;
            //std::cout << "expected len: " << lengths[it->first] << std::endl;
            if ((requests[it->first]).length() == lengths[it->first]) {
                std::pair <int, std::string> result = std::make_pair(it->first, requests[it->first]);
                requests.erase(it->first);
                return result;
            }
            break;
        }
    }
    return std::make_pair(-1, "");
} 

void send_result(std::pair <int, std::string> result) {
    write_buf[result.first] = encode_fname(result.second);
}

int connect(std::string &addr, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent* he;
    he = gethostbyname(addr.c_str());
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    memcpy(&saddr.sin_addr, he->h_addr, he->h_length);
    connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
    sfdset.insert(sockfd);
    return sockfd;

}
