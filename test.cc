#include "sockets.h"



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

bool test_fname() {
    std::string fname = "function_name";
    std::string enc = encode_fname(fname);
    if (decode_fname(enc) != fname) return false;
    return true;
}

void test_args() {
    int a = 3;
    char b = 'a';
    int argtypes[3];
    argtypes[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    argtypes[1] = (1 << ARG_OUTPUT) | (ARG_CHAR << 16);
    argtypes[2] = 0;
    void** args = (void**) malloc(3 * sizeof(void *));
    args[0] = (void*)&a;
    args[1] = (void*)&b;
    std::string enc = encode_args(argtypes, args);
    //std::cout << "encode success" << std::endl;
    //std::cout << "encoded size: " << enc.length() << std::endl;
    void** deco = decode_args(argtypes, enc);
    //std::cout << "decode success" << std::endl;
    std::cout << *(int *)deco[0] << std::endl;
    std::cout << *(char *)deco[1] << std::endl;
}
// testing
#include <iostream>
int main() {
    init();
    std::cout << test_length() << std::endl;
    std::cout << test_argtypes() << std::endl;
    std::cout << test_fname() << std::endl;
    test_args();
}
