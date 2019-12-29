
#ifndef SERVER_UTILITY_H
#define SERVER_UTILITY_H

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
#include <fcntl.h>

#include <pthread.h>
#include <limits.h>

#include "bst.h"
#include "list.h"

#define MAX_CONN_NUMBER 5
#define BUFFSIZE 4096

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


void destroyBSTHostInfo(BSTHostInfo * bstInfo);
void destroyAgentInfo(agentInfo * info);
BSTHostInfo * initBSTHostInfo(void);

long parseInt(char *arg);
long parseIP(char * IP);
int parsePort(char * portToParse);


#endif