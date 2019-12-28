
#include "server.h"


//GLOBAL VARIABLES
node * sdContainer;
int sdAgent;
int sdClient;
pthread_attr_t threadAttributes;
bool serverKilled = false;
BSTHostInfo * bstHostInfo;

int threadsCounter = 0;
pthread_mutex_t mutexThreadsCounter;

ssize_t writen(int sd, const void* vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char* ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0) {
        if( (nwritten = write(sd,ptr,nleft)) <= 0) {
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


ssize_t readn(int fd, void * vptr, size_t n){
    size_t nleft;
    ssize_t nread;
    char * ptr;

    ptr = vptr;
    nleft = n;

    while(nleft > 0){
        if((nread = read(fd,ptr,nleft)) < 0){
            if(errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if(nread == 0)
            return -2; //Socket broken

        nleft -= nread;
        ptr += nread;
    }

    return (n - nleft); 
}

void destroyBSTHostInfo(BSTHostInfo * bstInfo){
    pthread_mutex_destroy(&bstInfo->mutex);
    bstDestroy(bstInfo->root);

    free(bstInfo);
}

BSTHostInfo * initBSTHostInfo(void){
    BSTHostInfo * bstInfo = (BSTHostInfo *)malloc(sizeof(BSTHostInfo));
    bstInfo->root = NULL;

    pthread_mutex_init(&bstInfo->mutex,NULL);

    return bstInfo;
}

//Takes a string and converts it to a number.
long parseInt(char *arg) {
  char *p = NULL;
  long result = (long) strtol(arg, &p, 10);
  if (p == NULL || *p != '\0') 
    return -1; 
  return result;
} 

//Takes a string representing an IP and converts it to a number.
long parseIP(char * IP){
    int len = strlen(IP);
    char * newIP = (char *)malloc(sizeof(char)*(len-2)); //Length of IP minus 3 dots

    int i=0;
    int j=0;

    //Copy IP without dots into newIP
    while(i<len){
        if(IP[i] != '.'){
            newIP[j] = IP[i];
            j++;
        }

        i++;
    }
    newIP[j]='\0';

    long newIPInt = parseInt(newIP);

    free(newIP);

    return newIPInt;
}

//Atomic increment of counter shared by threads
void incrementThreadCounter(int * counter){
    pthread_mutex_lock(&mutexThreadsCounter);
        (*counter)++;
        printf("Increment: %d\n", *counter);
    pthread_mutex_unlock(&mutexThreadsCounter);
}

//Atomic increment of counter shared by threads
void decrementThreadCounter(int * counter){
    pthread_mutex_lock(&mutexThreadsCounter);
        (*counter)--;
        printf("Decrement: %d\n", *counter);
    pthread_mutex_unlock(&mutexThreadsCounter);
}

//Terminates the process with the exit status specified by "err". 
//Uses perror function to print a message describing the meaning of the value of errno.
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

} 


void * handleClient(void * arg){
    int socketClient = *(int *)arg;
    char * state;

    incrementThreadCounter(&threadsCounter);

    if(bstHostInfo->root == NULL)
        state = "There are no agents. Waiting for at least one registered host...\n\0";
    else
        state = "List of registered hosts. Pick one:\n\0";

    int err;
    if((err=write(socketClient,state,strlen(state)+1)) <= 0){
        printf("Errore nella writen2!\n");
        printf("STATE: %s - ERROR: %d - SOCKETCLIENT: %d\n", state, err, socketClient);
        perror("Errore nella write: ");
    }

    //Wait till one agent connects or server is killed
    while(bstHostInfo->root == NULL && !serverKilled){}

    char read_buff[BUFFSIZE];
    struct hostent * host;
    int nread;
    char * hosts;
    long hostIP;
    BSTNode * hostNode;
    unsigned long buffInfo[3];

    while(!serverKilled){
        
        if(readn(socketClient,read_buff,6) < 0){
            perror("Error reading\n");
            break;
        }

        pthread_mutex_lock(&bstHostInfo->mutex);

            hosts = bstGetHosts(bstHostInfo->root);
            printf("List to send to client:\n %s\n", hosts);
        
        pthread_mutex_unlock(&bstHostInfo->mutex);

        printf("I'm writing %s\n", hosts);
        //Send hosts list to client
        if(write(socketClient,hosts,strlen(hosts)) <= 0){
            printf("Errore nella writen2!\n");
        }
        free(hosts);

        memset(read_buff,0,BUFFSIZE);

        //Read the choice from client
        printf("Reading choice from client...\n");
        while((nread=read(socketClient,read_buff,BUFFSIZE)) < 0){
            if(serverKilled){
                perror("error reading\n");
                break;
            }
        }
        if(serverKilled || nread==0)
            break;

        read_buff[strlen(read_buff)] = '\0';

        host = gethostbyname(read_buff);
        printf("IP host: %s\n", inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));
        
        hostIP = parseIP(inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));

        hostNode = bstSearch(bstHostInfo->root,hostIP);

        //Clean buffer
        memset(read_buff,0,BUFFSIZE);

        if(hostNode != NULL){
            if(hostNode->connected){
                strcpy(read_buff,"connected\0");
                write(socketClient,read_buff,strlen(read_buff)+1);                

                buffInfo[UPTIME] = hostNode->uptime;
                buffInfo[FREERAM] = hostNode->freeram;
                buffInfo[PROCS] = hostNode->procs;

                printf("SENT-> Uptime: %lu Freeram: %lu Procs: %lu\n", buffInfo[UPTIME], buffInfo[FREERAM], buffInfo[PROCS]);

                if( writen(socketClient,buffInfo,sizeof(buffInfo)) < 0 ) {
                    perror("Error writing\n");
                    break;
                }
            }
            else{
                strcpy(read_buff,"disconnected\0");
                write(socketClient,read_buff,strlen(read_buff)+1);  

                printf("SENT-> %s\n", hostNode->time);

                if( writen(socketClient,hostNode->time,strlen(hostNode->time)+1) < 0 ) {
                    perror("Error writing\n");
                    break;
                }
            }
        }
    }

    printf("Client kill...\n");
    free((int *)arg);
    if(close(socketClient)<0){
        perror("Error closing client socket\n");
    }
    decrementThreadCounter(&threadsCounter);
    //pthread_exit(NULL);

    return NULL;
}

void * handleClientStub(void * arg){

    struct sockaddr_in client_addr;
    
    socklen_t size_client_addr = sizeof(client_addr);

    pthread_t tid;
    int * sdClientLocal; 
    int acceptResult;
    struct timeval timer;
    timer.tv_sec=1;
    timer.tv_usec=0;

    //Set read calls non-blocking. If fails, kill server.
    if(setsockopt(sdClient,SOL_SOCKET,SO_RCVTIMEO,(const char *)&timer,sizeof(timer))<0){
        printf("ERROR SETSOCKOPT\n");
        perror("error\n");
        serverKilled = true;
    }

    while(!serverKilled){

        acceptResult = accept(sdClient,(struct sockaddr *)&client_addr,&size_client_addr);

        if(acceptResult != -1){
            sdClientLocal = (int *)malloc(sizeof(int));
            *sdClientLocal = acceptResult;

            printf("Client has connected!\n");
            pthread_create(&tid,&threadAttributes,handleClient,sdClientLocal);
            
            listInsert(sdContainer,*sdClientLocal,tid);
        }
    }

    printf("ClientStub kill...\n");
    return NULL;
}

void * handleAgent(void * arg){
    agentInfo * info = (agentInfo *)arg;

    int socketAgent = *(info->sd);
    bool inserted = false;
    time_t timer;

    char * currentTime;
    char * lastTime;
    
    long localKey;
    BSTNode * node;
    BSTNode * nodeFound;

    unsigned long read_buffer[3];
    
    incrementThreadCounter(&threadsCounter);

    //Set localKey to agent's IP.
    localKey = parseIP(info->IP);
    int ret;

    while(!serverKilled){
        //Get time
        time(&timer);
        currentTime = ctime(&timer);

        //Clean buffer
        memset(read_buffer,0,sizeof(read_buffer));

        //If agent closes socket, then wait 6 seconds and check if it has reconnected.
        if((ret=readn(socketAgent,read_buffer,sizeof(read_buffer))) == -2){
            sleep(6);

            nodeFound = bstSearch(bstHostInfo->root,localKey);
            if(strcmp(nodeFound->time,lastTime) == 0){
                //Agent has not reconnected
                printf("Agent set to disconnected!\n");
                bstSetState(bstHostInfo->root,localKey,false);
            }

            break;
        }
        else if(ret == -1){
            //Error on reading
            break;
        }

        pthread_mutex_lock(&bstHostInfo->mutex);
            
            node = newNode(localKey,info->idhost,currentTime,read_buffer[UPTIME],read_buffer[FREERAM],read_buffer[PROCS]);
            
            if(inserted)
                free(lastTime);

            lastTime = (char *)malloc(sizeof(char)*(strlen(currentTime)+1));
            strcpy(lastTime,currentTime);

            if(!inserted){ //First insertion of this agent
                if(bstSetState(bstHostInfo->root,localKey,true)){
                    //Agent has reconnected: it is already in the structure, just update infos.
                    bstUpdate(bstHostInfo->root,node);
                }
                else{
                    //Agent isn't in the structure, insert new infos.
                    bstHostInfo->root = bstInsert(bstHostInfo->root,node);
                }
                inserted = true;
            }
            else{ //Update of this agent (still connected)
                bstUpdate(bstHostInfo->root,node);
            }

        pthread_mutex_unlock(&bstHostInfo->mutex);

    }

    free(lastTime);
    close(*(info->sd));
    free(info->sd);
    free(info->IP);
    free(info->idhost);
    free(info);

    printf("Agent kill...\n");
    decrementThreadCounter(&threadsCounter);
    //pthread_exit(NULL);
    return NULL;
}


void * handleAgentStub(void * arg){

    struct sockaddr_in agent_addr;
    
    socklen_t size_agent_addr = sizeof(agent_addr);
    
    struct hostent * clientInfo;
    struct in_addr inAgentAddress;

    pthread_t tid;
    int * socketAgent;
    int acceptResult;

    char * idAgent;
    char * instant;
    char * agentIP;

    time_t timer;
    agentInfo * info;

    while(!serverKilled){
        
        acceptResult = accept(sdAgent,(struct sockaddr *)&agent_addr,&size_agent_addr);
        
        if(acceptResult != -1){
            socketAgent = (int *)malloc(sizeof(int));
            *socketAgent = acceptResult;

            //Get time
            time(&timer);
            instant = ctime(&timer);
            
            //Get hostname or IP (if host not available)
            inAgentAddress = agent_addr.sin_addr;

            printf("\nAgent IP before resolution: %s\n", inet_ntoa(agent_addr.sin_addr));

            clientInfo = gethostbyaddr(&inAgentAddress,sizeof(inAgentAddress),AF_INET);

            //Get agent's IP as string
            agentIP = (char *)malloc(sizeof(char)*(strlen(inet_ntoa(agent_addr.sin_addr))+1));
            strcpy(agentIP,inet_ntoa(agent_addr.sin_addr));

            if(clientInfo != NULL){
                idAgent = (char *)malloc(sizeof(char)*(strlen(clientInfo->h_name)+1));
                strcpy(idAgent,clientInfo->h_name);
            }
            else{
                //error("gethostfun",h_errno);
                idAgent = (char *)malloc(sizeof(char)*(strlen(agentIP)+1));
                strcpy(idAgent,agentIP);
                printf("Resolution failed. IP: %s\n", idAgent);
            }

            //Prepare struct to pass to thread
            info = (agentInfo *)malloc(sizeof(agentInfo));

            info->sd = socketAgent;
            info->time = instant;
            info->idhost = idAgent;
            info->IP = agentIP;

            pthread_create(&tid,&threadAttributes,handleAgent,info);
            
            listInsert(sdContainer,*socketAgent,tid);
        }
    }
    
    printf("AgentStub kill...\n");
    //pthread_exit(NULL);
    return NULL;
}

//Takes a string representig a port in the range MIN_PORT MAX_PORT and returns it as integer.
int parsePort(char * portToParse){
    int port = parseInt(portToParse);

    if(port<MIN_PORT || port>MAX_PORT){ //Fallita la conversione o porta non compresa nel range
        printf("usage: ./program <port_agent> <port_client>. Insert valid port!\n");
        exit(1);
    }

    return port;
}


bool isZeroThreadsCounter(int * counter,pthread_mutex_t * mutex){
    pthread_mutex_lock(mutex);
        if(*counter ==0){
            pthread_mutex_unlock(mutex);
            return true;
        }
        else{
            pthread_mutex_unlock(mutex);
            return false;
        }
    
}


int main(int argc, char * argv[]){

    //signal(SIGPIPE,sigpipeHandler);
    signal(SIGINT,sigintHandler);

    //Redirect stderr to stdout
    dup2(STDOUT_FILENO,STDERR_FILENO);

    if(argv[1]==NULL || argv[2]==NULL){
        printf("usage: %s <port_agent> <port_client>\n", argv[0]);
        exit(1);
    }
    
    int port_agent = parsePort(argv[1]);

    int port_client = parsePort(argv[2]);

    printf("\nServer listening on ports %d (agents), %d (clients)...\n\n", port_agent, port_client);

    pthread_mutex_init(&mutexThreadsCounter,NULL);

    //Declaring variables
    struct sockaddr_in server_addr;
    struct sockaddr_in server_addr_client;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_agent);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    server_addr_client.sin_family = AF_INET;
    server_addr_client.sin_port = htons(port_client);
    server_addr_client.sin_addr.s_addr = htonl(INADDR_ANY);

    //Creates non blocking socket to avoid it blocking in accept()
    sdAgent = socket(PF_INET,SOCK_STREAM | SOCK_NONBLOCK,0);
    
    if(sdAgent == -1)
        error("Error creating the socket!\n",1);

    if(bind(sdAgent,(struct sockaddr *)&server_addr,sizeof(server_addr)) == -1){
        error("Error binding address!\n",1);
    }

    if(listen(sdAgent,5) == -1){
        error("Error in listening!\n",1);
    }

    sdClient = socket(PF_INET,SOCK_STREAM | SOCK_NONBLOCK,0);
    
    if(sdClient == -1)
        error("Error creating the socket!\n",1);

    if(bind(sdClient,(struct sockaddr *)&server_addr_client,sizeof(server_addr_client)) == -1){
        error("Error binding address!\n",1);
    }

    if(listen(sdClient,MAX_CONN_NUMBER) == -1){
        error("Error in listening!\n",1);
    }

    if(pthread_attr_init(&threadAttributes) != 0){
        exit(-1);
    }
    if(pthread_attr_setdetachstate(&threadAttributes,PTHREAD_CREATE_DETACHED) != 0){
        exit(-1);
    }

    //Init struct containing mutex and bst root
    bstHostInfo = initBSTHostInfo();
    //Create list that will contain socket descriptors and thread ids
    sdContainer = listCreate();

    pthread_t tid_agent, tid_client;

    //Creating stub thread for agent
    if(pthread_create(&tid_agent,NULL,handleAgentStub,NULL) != 0){
        error("Error creating stub thread for agent\n",-3);
    }

    //Creating stub thread for client
    if(pthread_create(&tid_client,NULL,handleClientStub,NULL) != 0){
        error("Error creating stub thread for client\n",-3);
    }

    while(!serverKilled || !isZeroThreadsCounter(&threadsCounter,&mutexThreadsCounter)){}

    pthread_join(tid_agent,NULL);
    pthread_join(tid_client,NULL);

    if(pthread_attr_destroy(&threadAttributes) != 0){
        write(STDERR_FILENO,"Error on attributes destroy",28);
    }
    if(close(sdAgent)<0){
        perror("Error closing agent socket!\n");
    }
    if(close(sdClient)<0){
        perror("Error closing agent socket!\n");
    }

    destroyBSTHostInfo(bstHostInfo);
    printf("Destroying list...\n");
    listCloseAndDestroy(sdContainer);
    pthread_mutex_destroy(&mutexThreadsCounter);

    return 0;

}