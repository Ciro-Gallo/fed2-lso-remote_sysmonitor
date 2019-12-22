
#include "server.h"


node * sdContainer;
int sdAgent;
pthread_attr_t threadAttributes;
bool serverKilled;

BSTHostInfo * bstHostInfo;

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

void initBSTHostInfo(void){
    bstHostInfo = (BSTHostInfo *)malloc(sizeof(BSTHostInfo));
    bstHostInfo->root = NULL;

    pthread_mutex_t mutex;
    pthread_mutex_init(&bstHostInfo->mutex,NULL);
}

void destroyBSTHostInfo(void){
    pthread_mutex_destroy(&bstHostInfo->mutex);
    bstDestroy(bstHostInfo->root);
    free(bstHostInfo);
}

void * handleAgent(void * arg){
    agentInfo * info = (agentInfo *)arg;

    int socketAgent = info->sd;

    unsigned long read_buffer[3];

    printf("\nTHREAD - Instant: %s HOST: %s\n", info->instant, info->id);

    while(!serverKilled){
        //Pulizia buffer di lettura
        memset(read_buffer,0,sizeof(read_buffer));

        if(read(socketAgent,read_buffer,sizeof(read_buffer))==0){
            printf("Agent has disconnected!\n");

            free(info->id);
            //free(info->instant);
            free(info);

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

    initBSTHostInfo();

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
    char * idAgent;
    time_t timer;
    char * instant;

    while(1){

        sdAgent2 = accept(sdAgent,(struct sockaddr *)&client_addr,&size_client_addr);

        if(sdAgent != -1){
            
            //Get time
            time(&timer);
            instant = ctime(&timer);
            
            //Get hostname or IP (if host not available)
            inAgentAddress = client_addr.sin_addr;
            clientInfo = gethostbyaddr(&inAgentAddress,sizeof(inAgentAddress),AF_INET);

            if(clientInfo != NULL){
                idAgent = (char *)malloc(sizeof(char)*(strlen(clientInfo->h_name)+1));
                strcpy(idAgent,clientInfo->h_name);
            }
            else{
                //error("gethostfun",h_errno);
                printf("\nErrore sulla gethostbyaddr\n");
                idAgent = (char *)malloc(sizeof(char)*strlen(inet_ntoa(client_addr.sin_addr)+1));
                strcpy(idAgent,inet_ntoa(client_addr.sin_addr));
                printf("Resolution failed. IP: %s\n", idAgent);
            }

            //Prepare struct to pass to thread
            agentInfo * info = (agentInfo *)malloc(sizeof(agentInfo));
            info->sd = sdAgent2;
            info->instant = instant;
            info->id = idAgent;

            pthread_create(&tid,&threadAttributes,handleAgent,info);
            
            listInsert(sdContainer,sdAgent2,tid);
        }
        else{
            error("Accept error!",1);
        }
    }

}