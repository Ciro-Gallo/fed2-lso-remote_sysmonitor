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

ssize_t writen(int sd, const void* vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char* ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0) {
        if( (nwritten = write(sd,ptr,nleft) <= 0) ) {
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
        perror("port overcome the range \n");
        exit (-1);
    }
}

void handleSigInt(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    exit (-1);
}

void handleSigPipe(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    printf("Server disconnected\n");
    exit (-1);
}


int main(int args, char** argv) {

    checkArgs(args,argv);
    signal(SIGINT,handleSigInt);
    signal(SIGPIPE,handleSigPipe);

    unsigned long buf[BUFFSIZE];
    struct sockaddr_in myaddress;
    struct sysinfo info;
    int port = argToInt(argv[1]);

    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(port);
    inet_aton(argv[2],&myaddress.sin_addr);

    if( (sd = socket(PF_INET,SOCK_STREAM,0)) < 0 ) {
        perror("Error creating socket\n");
        exit (-1);
    }

    if( connect(sd,(struct sockaddr*)&myaddress,sizeof(myaddress)) != 0 ) {
        perror("Error during the connection\n");
        exit (-1);
    }
    

    while(1) {
        if( sysinfo(&info) != 0 ) {
            perror("Error fetching system informations\n");
            exit (-1);
        } 
        buf[UPTIME] = info.uptime;
        buf[FREERAM] = info.freeram;
        buf[PROCS] = info.procs;
        printf("uptime = %lu freeram = %lu procs = %lu\n",buf[UPTIME],buf[FREERAM],buf[PROCS]);
        
        if( writen(sd,buf,sizeof(buf)) == -1 ) {
            perror("Error writing\n");
            exit (-1);
        }
        sleep(3);
    }
   
    return 0;
}
