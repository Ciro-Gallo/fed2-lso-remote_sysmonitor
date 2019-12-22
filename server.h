
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#include <sys/sysinfo.h>
#include <arpa/inet.h> 
#include <netdb.h>

#include <pthread.h>

#include "bst.h"
#include "list.h"


#define MIN_PORT 1024
#define MAX_PORT 65535 

typedef struct agentInfo{
    int sd;
    char * instant;
    char * id;
} agentInfo;

typedef struct BSTHostInfo{
    BSTNode * root;
    pthread_mutex_t mutex;
} BSTHostInfo;