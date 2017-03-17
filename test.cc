#include "sockets.h"
#include "rpc.h"

#include <iostream>



bool test_length() {
    size_t len = 233;
    std::string str = encode_length(len);
    if (decode_length(str) != len) return false;
    return true;
}

bool test_argtypes() {
    int argtypes[] = {14324, 143251, 0};
    std::string str = encode_argtypes(argtypes);
    int* ans = decode_argtypes(str).second;
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
    /*int a = 3;
    char b = 'a';
    int argtypes[3];
    argtypes[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    argtypes[1] = (1 << ARG_OUTPUT) | (ARG_CHAR << 16);
    argtypes[2] = 0;
    void** args = (void**) malloc(3 * sizeof(void *));
    args[0] = (void*)&a;
    args[1] = (void*)&b;
    std::string enc = encode_args(argtypes, args);
		a = 100;
		b = 'b';
		args[0] = NULL;
    //std::cout << "encode success" << std::endl;
    //std::cout << "encoded size: " << enc.length() << std::endl;
    void** deco = decode_args(argtypes, enc);
    //std::cout << "decode success" << std::endl;
    std::cout << *(int *)deco[0] << std::endl;
    std::cout << *(char *)deco[1] << std::endl;*/
    int a0 = 5;
    int b0 = 10;
    int count0 = 3;
    int return0=0;
    int argTypes0[count0 + 1];
    void **args0;

    argTypes0[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    argTypes0[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes0[2] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes0[3] = 0;
    args0 = (void **)malloc(count0 * sizeof(void *));
    args0[0] = (void *)&return0;
    args0[1] = (void *)&a0;
    args0[2] = (void *)&b0;

		std::string name = "func_name";
		std::string enc = encode_fname(name) + encode_argtypes(argTypes0) + encode_args(argTypes0, args0);
		std::string dename = decode_fname(enc);
		enc.erase(0, sizeof(size_t) + dename.length());
			std::pair<size_t, int*> a = decode_argtypes(enc);
			std::cout << "sizeof argTypes: " << a.first << std::endl;
			int *argt = a.second;
			enc.erase(0, sizeof(size_t) + a.first * sizeof(int));
			void **dec = decode_args(argt, enc);
            std::cout << "decoding done" << std::endl;

		//void **dec = decode_args(argTypes0, enc);

    std::cout << *(int *)dec[0] << std::endl;
    std::cout << *(int *)dec[1] << std::endl;
    std::cout << *(int *)dec[2] << std::endl;
}

bool test_ints() {
    int a = 3;
    std::string s = encode_int(a);
    int b = decode_int(s);
    if (a == b) return true;
    else return false;
}
// testing
#include <iostream>
int main() {
    init();
    std::cout << test_length() << std::endl;
    std::cout << test_argtypes() << std::endl;
    std::cout << test_fname() << std::endl;
    test_args();
    std::cout << test_ints() << std::endl;
}
