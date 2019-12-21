#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#include <sys/sysinfo.h>
#include <arpa/inet.h> 
#include <netdb.h>

#include <pthread.h>

#include "list.h"

#define MIN_PORT 1024
#define MAX_PORT 65535 

node * sdContainer;

int sdAgent;
pthread_attr_t threadAttributes;
bool serverKilled;


int parsePort(char *arg) {
  char *p = NULL;
  int result = (int) strtol(arg, &p, 10);
  if (p == NULL || *p != '\0') 
    return -1; 
  return result;
} 

void error(char * msg,int err){
    perror(msg);
    exit(err);
}

void sigpipeHandler(int code){
    write(STDOUT_FILENO,"Ho catturato SIGPIPE!\n",23);
}

void sigintHandler(int code){
    write(STDOUT_FILENO,"\nHo catturato SIGINT!\n",22);
    serverKilled = true;
    listCloseAndDestroy(sdContainer);
    if(pthread_attr_destroy(&threadAttributes) != 0){
        write(STDERR_FILENO,"Error on attributes destroy",28);
    }

    //Main thread should wait until all threads end with pthread_exit()
    sleep(1);
    exit(-2);
} 

void * handleAgent(void * arg){
    int socketAgent = *(int *)arg;

    unsigned long read_buffer[3];

    while(!serverKilled){
        //Pulizia buffer di lettura
        memset(read_buffer,0,sizeof(read_buffer));

        if(read(socketAgent,read_buffer,sizeof(read_buffer))==0){
            printf("Agent has disconnected!\n");
            pthread_exit(NULL);
            break;
        }

        printf("\nUptime: %lu Freeram: %lu Procs: %lu\n", read_buffer[0], read_buffer[1], read_buffer[2]); 
    }

    pthread_exit(NULL);
}


int main(int argc, char * argv[]){

    //signal(SIGPIPE,sigpipeHandler);
    signal(SIGINT,sigintHandler);

    //Redirect stderr to stdout
    dup2(STDOUT_FILENO,STDERR_FILENO);

    struct sockaddr_in server_addr, client_addr;

    if(argv[1]==NULL){
        printf("Error: port needed!\n");
        exit(1);
    }
    
    int port = parsePort(argv[1]);

    if(port<MIN_PORT || port>MAX_PORT){ //Fallita la conversione o porta non compresa nel range
        printf("Port error!\n");
        exit(1);
    }

    printf("\nServer listening on port %d...\n\n", port);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sdAgent = socket(PF_INET,SOCK_STREAM,0);
    if(sdAgent == -1)
        error("Error creating the socket!\n",1);

    if(bind(sdAgent,(struct sockaddr *)&server_addr,sizeof(server_addr)) == -1){
        error("Error binding address!\n",1);
    }

    if(listen(sdAgent,5) == -1){
        error("Error in listening!\n",1);
    }

    socklen_t size_client_addr = 0;
    int sdAgent2;
    pthread_t tid;

    if(pthread_attr_init(&threadAttributes) != 0){
        exit(-1);
    }
    if(pthread_attr_setdetachstate(&threadAttributes,PTHREAD_CREATE_DETACHED) != 0){
        exit(-1);
    }

    sdContainer = listCreate();
    struct hostent * clientInfo;
    struct in_addr inAgentAddress;
    char * idClient;
    time_t timer;
    char * instant;

    while(1){

        sdAgent2 = accept(sdAgent,(struct sockaddr *)&client_addr,&size_client_addr);

        if(sdAgent != -1){
            
            //Get time
            time(&timer);
            instant = ctime(&timer);
            printf("Instant: %s\n", instant);

            inAgentAddress = client_addr.sin_addr;
            clientInfo = gethostbyaddr(&inAgentAddress,sizeof(inAgentAddress),AF_INET);

            if(clientInfo != NULL){
                idClient = (char *)malloc(sizeof(char)*sizeof(clientInfo->h_name));
                strcpy(idClient,clientInfo->h_name);
                printf("Connection in - IP: %s - PORT: %d - HOST: %s\n", inet_ntoa(client_addr.sin_addr), (int)ntohs(client_addr.sin_port),idClient);
            }
            else{
                //error("gethostfun",h_errno);
                printf("\nErrore sulla gethostbyaddr\n");
                idClient = (char *)malloc(sizeof(char)*sizeof(inet_ntoa(client_addr.sin_addr)));
                strcpy(idClient,inet_ntoa(client_addr.sin_addr));
                printf("Resolution failed. IP: %s\n", idClient);
            }

            pthread_create(&tid,&threadAttributes,handleAgent,&sdAgent2);
            pthread_detach(tid);
            listInsert(sdContainer,sdAgent2,tid);
            printf("List of sd: ");
            listPrint(sdContainer);
        }
        else{
            error("Accept error!",1);
        }
    }

}