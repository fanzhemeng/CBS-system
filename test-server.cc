#include "rpc.h"
#include <iostream>


int f0(int a, int b) {
	return a + b;
}

int f0_Skel(int *argTypes, void **args) {

  *(int *)args[0] = f0(*(int *)args[1], *(int *)args[2]);
  return 0;
}

int main() {
	std::cout << "hello" << std::endl;
	rpcInit();
	std::cout << "hello" << std::endl;


  int count0 = 3;
  int argTypes0[count0 + 1];
  argTypes0[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  argTypes0[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
  argTypes0[2] = (1 << ARG_INPUT) | (ARG_INT << 16);
  argTypes0[3] = 0;

  rpcRegister((char*)"f0", argTypes0, *f0_Skel);

    return 0;

}
