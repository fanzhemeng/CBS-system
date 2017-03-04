#include "sockets.h"


typedef int (*skeleton) (int *, void **);

int rpcInit(void);

int rpcRegister(char *name, int *argTypes, skeleton f);

int rpcExecute(void);
