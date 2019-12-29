#ifndef CLIENT_UTILITY_H
    #define CLIENT_UTILITY_H
    #include <sys/types.h>
    #include <sys/socket.h>

    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <unistd.h>
    #include <stdbool.h>

    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <signal.h>
    #include <errno.h>

    #define MIN_PORT 1024
    #define MAX_PORT 65535
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
    ssize_t writen(int sd, const void* vptr, size_t n);
    ssize_t readn(int sd, void* vptr, size_t n);
    void handleSigInt(int s);
    void handleSigPipe(int s);
    int argToInt(char* arg);
    void checkArgs(int args, char** argv);
    char** printUpdatedList(int sockd, int * hnumber);
    void releaseResources(char ** hosts, char * hostname);

#endif