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

char** hostsToArray(char * buf) {
    char ** arrayHost = (char**)malloc(sizeof(char*));
    int i = 0,j = 0,k = 0;
    int bytes = 0;

    while(buf[i] != '\0') {
        bytes++;

        if(buf[i] == '\n') {
            arrayHost =(char**)realloc(arrayHost,sizeof(arrayHost) + (sizeof(char*)));
            arrayHost[j] = (char*)malloc(sizeof(char)*bytes);
            bytes = 0;
            j++;
        }
        i++;
    }

    i = 0;
    j = 0;
    while(buf[i] != '\0') {
        if(buf[i] != '\n') {
            arrayHost[j][k] = buf[i];
            k++;
        } else {
            k = 0;
            j++;
        }
        i++;
    }

    return arrayHost;
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


    while(1) {
        memset(buf,0,BUFFSIZE);
        if( read(sd,buf,BUFFSIZE) < 0 ) {
            perror("error reading\n");
        }
        char ** array = hostsToArray(buf);
        printf("array[0] = %s  \narray[1] = %s\n",array[0],array[1]);
    }

    return 0;
}