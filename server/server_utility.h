
#ifndef SERVER_UTILITY_H
    #define SERVER_UTILITY_H

    #include <stdbool.h>
    #include <netdb.h>
    #include <sys/time.h>
    #include <pthread.h>

    #include "../utility/utility.h"
    #include "../bst/bst.h"
    #include "../list/list.h"

    #define MAX_CONN_NUMBER 5
    #define BUFFSIZE 4096
    #define NUM_ARGS 3

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

    void destroyBSTHostInfo(BSTHostInfo * bstInfo);
    void destroyAgentInfo(agentInfo * info);
    BSTHostInfo * initBSTHostInfo(void);

    long parseInt(char *arg);
    long parseIP(char * IP);
    int parsePort(char * portToParse);
#endif