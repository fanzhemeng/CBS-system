#include <string>


std::string encode(unsigned char* p, size_t len) {
    std::string str;
    for (size_t i = 0; i < len; i ++)
        str += p[i];
    return str;
}

void decode(unsigned char* p, std::string str, size_t len) {
    for (size_t i = 0; i < len; i ++)
        p[i] = str[i];
}

std::string encode_length(size_t len) {
    return encode((unsigned char*)&len, sizeof(len));
}

size_t decode_length(std::string str) {
    size_t length;
    decode((unsigned char*)&length, str, sizeof(length));
    return length;
} 

size_t length_of_argtypes(int* argtypes) {
    size_t i;
    for (i = 0; argtypes[i] != 0; i ++);
    return i;
}

std::string encode_argtypes(int* argtypes) {
    size_t length = length_of_argtypes(argtypes);
    return encode_length(length) + encode((unsigned char*)argtypes, sizeof(int) * length);
}

int* decode_argtypes(std::string str) {
    size_t length = decode_length(str);
    str.erase(0, sizeof(length));
    int* argtypes = new int[length];
    decode((unsigned char*)argtypes, str, length * sizeof(int));
    return argtypes;
}

bool test_length() {
    size_t len = 233;
    std::string str = encode_length(len);
    if (decode_length(str) != len) return false;
    return true;
}

bool test_argtypes() {
    int argtypes[] = {14324, 143251, 0};
    std::string str = encode_argtypes(argtypes);
    int* ans = decode_argtypes(str);
    for (int i = 0; i < 3; i ++)
        if (argtypes[i] != ans[i]) {
            delete [] ans;
            return false;
        }
    delete [] ans;
    return true;
}

// testing
#include <iostream>
int main() {
    std::cout << test_length() << std::endl;
    std::cout << test_argtypes() << std::endl;
}
