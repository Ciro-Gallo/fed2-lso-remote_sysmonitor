
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
#include <sys/time.h>
#include <errno.h>

#include <pthread.h>
#include <limits.h>

#include "bst.h"
#include "list.h"


#define MIN_PORT 1024
#define MAX_PORT 65535 

#define MAX_CONN_NUMBER 5
#define BUFFSIZE 4096

#define UPTIME 0
#define FREERAM 1
#define PROCS 2

typedef struct agentInfo{
    int * sd;
    char * time;
    char * idhost;
    char * IP;
} agentInfo;

typedef struct BSTHostInfo{
    BSTNode * root;
    pthread_mutex_t mutex;
} BSTHostInfo;