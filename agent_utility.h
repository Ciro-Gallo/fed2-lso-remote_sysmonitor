#ifndef AGENT_UTILITY_H
    #define AGENT_UTILITY__H
    
    #include "utility.h"
    #include <sys/sysinfo.h>
    #include <netdb.h>

    #define BUFFSIZE 3

    int sd;

    void handleSigInt(int s);
    void handleSigPipe(int s);

#endif