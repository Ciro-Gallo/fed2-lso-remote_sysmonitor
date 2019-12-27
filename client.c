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
#define UPTIME 0
#define FREERAM 1
#define PROCS 2


int sd,hostsnumber;
char ** g_hosts;
char* g_hostname;

int getHostsDim(char ** hosts) {
    int j = 0;
    while(hosts[j] != NULL) {
        j++;
    }
    
    return j;
}

void destroyHosts(char ** hosts) {
    for(int i = 0; i < hostsnumber; ++i) {
        free(hosts[i]);
    }
    free(hosts);
}

void getInt(int * elem) {
    int ret = scanf("%d",elem);
    while(ret == 0) {
        printf("errore di input: inserire un intero\n");
        while(getchar()!='\n') {
            ret = scanf("%d",elem);
        }
    }
}

void printHosts(char ** hosts) {
    for(int i = 0; i <  hostsnumber; ++i) {
        printf("\n%d. %s\n",i,hosts[i]);
    }
}

char* getHostName(char * hostinfo) {
    int i = 0;
    int dim = strlen(hostinfo);
    int pos = dim - 9;
    char * hostname;

    if(hostinfo[pos] == 'c') {
        hostname = (char*)malloc(sizeof(char)*(dim - 9));
    } else {
        hostname = (char*)malloc(sizeof(char)*(dim - 12));
    }

    while(hostinfo[i] != ' ') {
        hostname[i] = hostinfo[i];
        i++;
    }
    hostname[i] = '\0';

    return hostname;
}

char** hostsToArray(char * buff) {
    char ** arrayHost = (char**)malloc(sizeof(char*));
    int i = 0,j = 0,k = 0;
    int bytes = 0;

    while(buff[i] != '\0') {
        bytes++;

        if(buff[i] == '\n') {
            arrayHost = (char**)realloc(arrayHost,sizeof(arrayHost) + (sizeof(char*)));
            arrayHost[j] = (char*)malloc(sizeof(char)*bytes);
            bytes = 0;
            j++;
        }
        i++;
    }
    hostsnumber = j;

    i = 0;
    j = 0;
    while(buff[i] != '\0') {
        if(buff[i] != '\n') {
            arrayHost[j][k] = buff[i];
            k++;
        } else {
            arrayHost[j][k] = '\0';
            k = 0;
            j++;
        }
        i++;
    }

    return arrayHost;
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

void handleSigInt(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    if(g_hosts != NULL)
        destroyHosts(g_hosts);

    exit (-1);
}

void handleSigPipe(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    if(g_hosts != NULL)
        destroyHosts(g_hosts);

    if(g_hostname != NULL)
        free(g_hostname);

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

char** printUpdatedList() {
    char buff[BUFFSIZE];
     
    //syncronize client and server
    if( writen(sd,"ready",6) < 0 ) {
        perror("error writing\n");
        exit (-1);
    }
    memset(buff,0,BUFFSIZE); 

    //get the hosts list
    if( read(sd,buff,BUFFSIZE) <= 0 ) {
        perror("error reading\n");
        exit (-1);
    }
;
    //index and print the hosts list
    char ** hosts = hostsToArray(buff);
    printHosts(hosts);

    return hosts;
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

    char read_buff[BUFFSIZE];
    char write_buff[BUFFSIZE];
    unsigned long buffinfo[3];

    if( read(sd,read_buff,BUFFSIZE) < 0 ) {
        perror("error reading\n");
        exit (-1);
    }
    printf("%s\n",read_buff);
    memset(read_buff,0,BUFFSIZE);

    int choice;
    while(1) {
        g_hosts = printUpdatedList();

        //choose the host and send his name to the server
        printf("\nChoose an host to take additional info: ");
        getInt(&choice);

        g_hostname = getHostName(g_hosts[choice]);
        strcpy(write_buff,g_hostname);

        if(writen(sd,write_buff,strlen(write_buff)+1) < 0) {
            perror("error writing\n");
            exit (-1);
        }

        //get the host state (connected or disconnected)
        if( read(sd,read_buff,BUFFSIZE) < 0 ) {
            perror("error writing\n");
            exit (-1);
        }

        if(strcmp(read_buff,"connected") == 0) {
            //read the host informations
            if( readn(sd,buffinfo,sizeof(buffinfo)) == -2 ) {
                perror("error reading\n");
                exit (-1);
            }
            printf("uptime: %lu  freeram: %lu  procs: %lu\n",buffinfo[UPTIME],buffinfo[FREERAM],buffinfo[PROCS]);
        } else {
            //read last registered date
            memset(read_buff,0,BUFFSIZE);
            if( read(sd,read_buff,BUFFSIZE) < 0 ) {
                perror("error writing\n");
                exit (-1);
            }
            printf("last date: %s",read_buff);
            memset(read_buff,0,BUFFSIZE);
        }

        destroyHosts(g_hosts);
        free(g_hostname);

        g_hosts = NULL;
        g_hostname = NULL;
    }

    return 0;
}