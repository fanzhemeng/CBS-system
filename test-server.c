#include "rpc.h"
#include <iostream>


int f0(int a, int b) {
	return a + b;
}

long f1(char a, short b, int c, long d) {

  return a + b * c - d;
}


int f0_Skel(int *argTypes, void **args) {

  *(int *)args[0] = f0(*(int *)args[1], *(int *)args[2]);
  return 0;
}


int f1_Skel(int *argTypes, void **args) {

  *((long *)*args) = f1( *((char *)(*(args + 1))), 
		        *((short *)(*(args + 2))),
		        *((int *)(*(args + 3))),
		        *((long *)(*(args + 4))) );

  return 0;
}

int main() {

    rpcInit();



    int count0 = 3;
    int count1 = 5;
    int argTypes0[count0 + 1];
    int argTypes1[count1 + 1];
    argTypes0[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    argTypes0[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes0[2] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes0[3] = 0;
    
    argTypes1[0] = (1 << ARG_OUTPUT) | (ARG_LONG << 16);
    argTypes1[1] = (1 << ARG_INPUT) | (ARG_CHAR << 16);
    argTypes1[2] = (1 << ARG_INPUT) | (ARG_SHORT << 16);
    argTypes1[3] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes1[4] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    argTypes1[5] = 0;



    rpcRegister((char*)"f0", argTypes0, *f0_Skel);
    rpcRegister((char*)"f1", argTypes1, *f1_Skel);
    rpcExecute();
    return 0;

}
