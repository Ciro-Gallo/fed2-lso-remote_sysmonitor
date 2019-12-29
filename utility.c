#include "utility.h"

ssize_t writen(int sd, const void* vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char* ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0) {
        if( (nwritten = write(sd,ptr,nleft)) <= 0 ) {
            if( nwritten < 0 && errno == EINTR ) 
                nwritten = 0;
            else 
                return (-1);
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
} 



ssize_t readn(int sd, void* vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char * ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0) {
        if( (nread = read(sd,ptr,nleft)) < 0 ) {
            if(errno == EINTR)
                nread = 0;
            else 
                return (-1);
        } else if(nread == 0)
            return (-2);
        
        nleft -= nread;
        ptr += nread;
    }
    return (n-nleft);
}


ssize_t readn(int sd, void* vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char * ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0) {
        if( (nread = read(sd,ptr,nleft)) < 0 ) {
            if(errno == EINTR)
                nread = 0;
            else 
                return (-1);
        } else if(nread == 0)
            return (-2);
        
        nleft -= nread;
        ptr += nread;
    }
    return (n-nleft);
}

int argToInt(char* arg) {
    char* p = NULL;
    int result = (int)strtol(arg,&p,10);
    if(p == NULL || *p != '\0') {
        return -1;
    }
    return result;
}

void checkArgs(int args, char** argv) {
    if(args != 3) {
        printf("usage: %s <port> <ipaddress>\n",argv[0]);
        exit (-1);
    }

    int port = argToInt(argv[1]);

    if(port < MIN_PORT || port > MAX_PORT) {
        printf("port overcome the range \n");
        exit (-1);
    }
}




