#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MIN_PORT 1024
#define MAX_PORT 65535
#define BUFFSIZE 3

int sd;

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

void handleSignals(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    exit (-1);
}


int main(int args, char** argv) {

    checkArgs(args,argv);
    signal(SIGINT,handleSignals);

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
        buf[0] = info.uptime;
        buf[1] = info.freeram;
        buf[2] = info.procs;
        printf("uptime = %lu freeram = %lu procs = %lu\n",buf[0],buf[1],buf[2]);
        
        if( write(sd,buf,sizeof(buf)) != sizeof(buf) ) {
            perror("Error writing\n");
            exit (-1);
        }
        sleep(3);
    }
   
    return 0;
}