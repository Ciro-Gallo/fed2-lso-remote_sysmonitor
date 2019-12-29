#ifndef CLIENT_UTILITY_H
    #define CLIENT_UTILITY_H
    
    #include "utility.h"
    #include <stdbool.h>

    #define BUFFSIZE 4096
    #define UPTIME 0
    #define FREERAM 1
    #define PROCS 2


    int sd,hostsnumber;
    char ** g_hosts;
    char* g_hostname;

    int getHostsDim(char ** hosts);
    void destroyHosts(char ** hosts);
    void getInt(int * elem, int b);
    void printHosts(char ** hosts);
    char* getHostName(char * hostinfo);
    char** hostsToArray(char * buff, int * hnumber);
    void handleSigInt(int s);
    void handleSigPipe(int s);
    char** printUpdatedList(int sockd, int * hnumber);
    void releaseResources(char ** hosts, char * hostname);

#endif