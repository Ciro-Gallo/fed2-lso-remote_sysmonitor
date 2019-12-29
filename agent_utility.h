#ifndef AGENT_UTILITY_H
    #define AGENT_UTILITY__H
    
    #include "utility.h"
    #include <sys/sysinfo.h>
    #include <netdb.h>


    #define MIN_PORT 1024
    #define MAX_PORT 65535
    #define BUFFSIZE 3
    #define UPTIME 0
    #define FREERAM 1
    #define PROCS 2

    int sd;

    void handleSigInt(int s);
    void handleSigPipe(int s);

#endif