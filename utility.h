#ifndef UTILITY_H
    #define UTILITY_H
    #include <sys/types.h>
    #include <sys/socket.h>

    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <unistd.h>

    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <signal.h>
    #include <errno.h>

    #define MIN_PORT 1024
    #define MAX_PORT 65535
    #define UPTIME 0
    #define FREERAM 1
    #define PROCS 2

    #define ESOCK_CREATE 5
    #define ESOCK_BIND 6
    #define ESOCK_LISTEN 7
    #define ESOCK_CONN 8

    #define ESYS_INFO 9
    #define EWRITE 10
    #define EREAD 11
    #define EIP_NOTVALID 12
    #define EARGS_NOTVALID 13
    #define EPORT_NOTVALID 14

    #define ETHREAD_CREATE 15
    
    
    ssize_t writen(int sd, const void* vptr, size_t n);
    ssize_t readn(int sd, void* vptr, size_t n);
    
    int argToInt(char* arg);
    void checkArgs(int args, char** argv);
    void error(char * msg,const int std,int err);
    float ramToPercentage(unsigned long freeram, unsigned long totalram);


#endif