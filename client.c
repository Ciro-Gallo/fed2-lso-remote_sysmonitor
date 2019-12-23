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


int sd;

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


int main(int args, char** argv) {
    
    checkArgs(args,argv);
    signal(SIGINT,handleSigInt);
    signal(SIGPIPE,handleSigPipe);

    struct sockaddr_in myaddress;
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
    printf("Connected\n");



    char buf[BUFFSIZE];
    if( read(sd,buf,BUFFSIZE) < 0 ) {
        perror("error reading\n");
    }
    printf("%s\n",buf);

    memset(buf,0,BUFFSIZE);
    if( read(sd,buf,BUFFSIZE) < 0 ) {
        perror("error reading\n");
    }
    printf("%s\n\n",buf);



  /*  bool stillReading = true;
    int lenBuf[1];
   
    //For getting the host list from the server
    while(stillReading) {
        read(sd,lenBuf,sizeof(lenBuf));
        if(lenBuf[0] == 0) {
            stillReading = false;
        } else {
            char * buf[lenBuf[0]];
            read(sd,buf,sizeof(buf));
            write(STDOUT_FILENO,buf,sizeof(buf));
        }
    } */

    return 0;
}