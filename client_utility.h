#ifndef CLIENT_UTILITY_H
    #define CLIENT_UTILITY_H
    
    #include "utility.h"
    #include <stdbool.h>

    #define BUFFSIZE 4096
    #define TIMELENGTH 26v


    int getHostsDim(char ** hosts);
    void destroyHosts(char ** hosts, int hnumber);
    void getInt(int * elem, int b);
    void printHosts(char ** hosts, int hnumber);
    char* getHostName(char * hostinfo);
    char** hostsToArray(char * buff, int * hnumber);
    void handleSigInt(int s);
    void handleSigPipe(int s);
    char** printAndGetUpdatedList(int sockd, int * hnumber);
    void releaseResources(char ** hosts, char * hostname, int hnumber);

#endif