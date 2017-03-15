#include <string>

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

enum MSGTYPE {
    LOC_REQUEST,
    LOC_SUCCESS,
    LOC_FAILURE,
    REGISTER,
    REGISTER_SUCC,
    REGISTER_FAIL,
	EXECUTE,
	EXECUTE_SUCC,
	EXECUTE_FAIL,
	TERMINATE,
    INVALID,
    FUNC_NOT_FOUND
};


//**communicate functions
void init();

std::string encode(unsigned char* p, size_t len);

void decode(unsigned char* p, std::string str, size_t len);

std::string encode_length(size_t len);

size_t decode_length(std::string str);

std::string encode_int(int num);

int decode_int(std::string str);



size_t length_of_argtypes(int* argtypes);

std::string encode_argtypes(int* argtypes);

std::pair<size_t, int*> decode_argtypes(std::string str);

std::string encode_fname(std::string fname);

std::string decode_fname(std::string str);

std::string encode_args(int* argtypes, void** args);

void** decode_args(int* argtypes, std::string str);
//**end of communication functions
int create_socket();

int getport(int sockfd);

std::string getaddr(int sockfd, const char* str_port);

void selection(int sockfd);

std::pair <int, std::string> respond();

void send_result(std::pair <int, std::string> result);

int connect(std::string &addr, int port);
