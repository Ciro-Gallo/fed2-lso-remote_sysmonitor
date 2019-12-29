#ifndef AGENT_UTILITY_H
    #define AGENT_UTILITY__H
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/sysinfo.h>

    #include <netdb.h>

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
    #define BUFFSIZE 3
    #define UPTIME 0
    #define FREERAM 1
    #define PROCS 2

    int sd;

    ssize_t writen(int sd, const void* vptr, size_t n);
    int argToInt(char* arg);
    void checkArgs(int args, char** argv);
    void handleSigInt(int s);
    void handleSigPipe(int s);

#endif