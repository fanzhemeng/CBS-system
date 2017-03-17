#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rpc.h"

#define CHAR_ARRAY_LENGTH 100

int main() {
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
    printf("before call\n");
    int s0 = rpcCall((char*)"f0", argTypes0, args0);
		for (int i=0; i < 10000; i++) 
				printf("return from call\nvalue: %d\n", return0);
    return 0;

}
