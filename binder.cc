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

std::map <function_def, std::set<int> > func_to_server;



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
                        //also add it to the queue
                        server_queue.insert(server_queue.begin(), current_sockfd);
                    }
                    //decode function signature
                    mes.erase(0, sizeof(port)); 
                    std::string fname = decode_fname(mes);
                    mes.erase(0, sizeof(size_t) + fname.length());                
                    std::pair<size_t, int*> argts = decode_argtypes(mes);
                    //insert into func_table
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


                    std::map < function_def, std::set<int> >::iterator it3;
                    it3 = func_to_server.find(func_def);
                    if (it3 == func_to_server.end()) {
                        std::set <int> new_set;
                        new_set.insert(current_sockfd);
                        func_to_server[func_def] = new_set;
                    } else {
                        (func_to_server[func_def]).insert(current_sockfd);
                    }
                    request.second = "REGISTER_SUCCESS";
                    send_result(request);
                    list_funcs();
                } catch (...) {
                    request.second = "REGISTER_FAILURE";
                    send_result(request); 
                } 
            } else if (type == LOC_REQUEST) {
                //decode the name
                std::string fname = decode_fname(mes);
                mes.erase(0, sizeof(size_t) + fname.length());
                std::pair<size_t, int*> argts = decode_argtypes(mes);
                std::vector <int> argtype;
                for (size_t i = 0; i < argts.first; i ++) {
                    argtype.push_back((argts.second)[i]);
                }
                function_def fde = std::make_pair(fname, argtype);
                std::map <function_def, std::set<int> >::iterator it;
                it = func_to_server.find(fde);
                //the function is on one of the server
                if (it != func_to_server.end()) {
                    std::set<int> servers = it->second;
                    std::set<int>::iterator sit;
                    std::vector<int>::iterator qit;
                    int server;
                    //find out which server should serve the request
                    for(qit = server_queue.begin(); qit != server_queue.end(); qit ++) {
                        sit = servers.find(*qit);
                        if (sit != servers.end()) {
                            server = *sit;
                            break;
                        }
                    }
                    //remove the current server from the queue and put it to the back
                    server_queue.erase(qit);
                    server_queue.push_back(server);
                    //find the server info
                    std::pair <std::string, int> server_info = id_table[server];
                    std::string type = encode_int(LOC_SUCCESS);
                    std::string id = encode_length(server_info.first.length()) + server_info.first;
                    std::string port = encode_int(server_info.second);
                    std::string msg = type + id + port;
                    request.second = msg;
                    send_result(request);
                } else {
                    std::string type = encode_int(LOC_FAILURE);
                    std::string reason = encode_int(FUNC_NOT_FOUND);
                    std::string msg = type + reason;
                    request.second = msg;
                    send_result(request);
                }
            }
        }
    }
    
}
