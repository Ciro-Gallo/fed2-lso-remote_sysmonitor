#include "utility.h"


void error(char * msg, const int std, int err){
    if(msg != NULL) {
       write(std,msg,strlen(msg)+1);
    }
    exit(err);
}


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
        error(NULL,STDOUT_FILENO,EARGS_NOTVALID);
    }

    int port = argToInt(argv[1]);

    if(port < MIN_PORT || port > MAX_PORT) {
        error("port overcome the range\n",STDOUT_FILENO,EPORT_NOTVALID);
    }
}





