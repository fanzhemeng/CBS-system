#include <string>


std::string encode(unsigned char* p, size_t len);

void decode(unsigned char* p, std::string str, size_t len);

std::string encode_length(size_t len);

size_t decode_length(std::string str);

size_t length_of_argtypes(int* argtypes);

std::string encode_argtypes(int* argtypes);

int* decode_argtypes(std::string str);


