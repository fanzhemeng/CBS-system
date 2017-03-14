#include "sockets.h"


#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <set>

typedef std::vector <int> argtypes;
typedef std::pair <std::string, argtypes> function_def;

std::vector <int> server_queue;

std::map <int, std::pair<std::string, int> > id_table;

std::map <int, std::set<function_def> > func_table;

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


static void list_funcs(void) {
    //list server and their addresses

    for(std::map<int, std::pair<std::string, int> >::iterator it = id_table.begin(); it != id_table.end(); it ++) {
        std::cout << it->first << "   " << (it->second).first << "    " << (it->second).second << std::endl;
    }
    //list functions
    for(std::map<int, std::set<function_def> >::iterator it = func_table.begin(); it != func_table.end(); it ++) {
        std::cout << it->first << ":" << std::endl;
        std::set<function_def> functions = it->second;
        for(std::set<function_def>::iterator it2 = functions.begin(); it2 != functions.end(); it2++) {
            std::cout << it2->first << std::endl;
        }
        
    }
}

int main() {
    // central main sockfd
    int sockfd = create_socket();
    int port = getport(sockfd);
    std::stringstream ss;
    ss << port;
    std::string str_port = ss.str();
    std::string addr = getaddr(sockfd, str_port.c_str());
    std::cout << "BINDER_ADDRESS: " << addr << std::endl;
    std::cout << "BINDER_PORT: " << port << std::endl;
    while(1) {
        //update buffers to receive messages
        selection(sockfd);
        //fetch the message in the read buffers
        std::pair <int, std::string> request = respond();
        // get the sockfd given to the channel
        int current_sockfd = request.first;
        if (request.first != -1) {
            std::string mes = request.second;
            std::cout << "mes : " << mes << std::endl;
            //determine message type
            int type = decode_int(mes);
            mes.erase(0, sizeof(int));

            //split functions according to types
            if (type == REGISTER) {
                try {
                    //decode address and port
                    std::string addr = decode_fname(mes);
                    mes.erase(0, sizeof(size_t) + addr.length());
                    int port = decode_int(mes);

                    std::map<int, std::pair<std::string, int> >::iterator it;
                    it = id_table.find(current_sockfd);
                    //adding a new server to the map
                    if (it == id_table.end()) {
                        id_table[current_sockfd] = std::make_pair(addr, port);
                        server_queue.insert(server_queue.begin(), current_sockfd);
                    }
                    //decode function signature
                    mes.erase(0, sizeof(port)); 
                    std::string fname = decode_fname(mes);
                    mes.erase(0, sizeof(size_t) + fname.length());                
                    std::pair<size_t, int*> argts = decode_argtypes(mes);
                    std::map<int, std::set<function_def> >::iterator it2;
                    it2 = func_table.find(current_sockfd);
                    std::vector <int> argtypes;
                    for (int i = 0; i < argts.first; i++) {
                        argtypes.push_back(argts.second[i]);
                    }
                    function_def func_def = std::make_pair(fname, argtypes);

                    if (it2 == func_table.end()) {
                        std::set<function_def>functions;
                        functions.insert(func_def);
                        func_table[current_sockfd] = functions;
                    }
                    else {
                        (func_table[current_sockfd]).insert(func_def);
                    }
                    request.second = "REGISTER_SUCCESS";
                    send_result(request);
                    list_funcs();
                } catch (...) {
                    request.second = "REGISTER_FAILURE";
                    send_result(request); 
                } 
            }
        }
    }
}
