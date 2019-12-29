
#include "server_utility.h"

//GLOBAL VARIABLES
node * sdContainer;
int sdAgent;
int sdClient;
bool serverKilled = false;
BSTHostInfo * bstHostInfo;

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
            if(errno == EINTR) //errno is thread safe
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


//Terminates the process with the exit status specified by "err". 
//Uses perror function to print a message describing the meaning of the value of errno.
void error(char * msg,int err){
    write(STDERR_FILENO,msg,strlen(msg)+1);
    exit(err);
}

void sigintHandler(int code){
    serverKilled = true;
} 

void * handleClient(void * arg){
    int socketClient = *(int *)arg;

    char * state;
    char * hosts;
    char read_buff[BUFFSIZE];
    int nread;
    long hostIP;
    unsigned long buffInfo[3];
    BSTNode * hostNode;
    struct hostent * host;
    bool threadKilled = false; //True if fatal error occurred in the thread and it must be killed.

    if(bstHostInfo->root == NULL)
        state = "There are no agents. Waiting for at least one registered host...\n\0";
    else
        state = "List of registered hosts. Pick one:\n\0";

    if(write(socketClient,state,strlen(state)+1) <= 0){
        threadKilled = true;
    }

    //Waits till one agent connects or server is killed
    while(bstHostInfo->root == NULL && !serverKilled && !threadKilled){}

    while(!serverKilled && !threadKilled){
        
        if(readn(socketClient,read_buff,6) < 0){
            break;
        }

        pthread_mutex_lock(&bstHostInfo->mutex);

            hosts = bstGetHosts(bstHostInfo->root);
        
        pthread_mutex_unlock(&bstHostInfo->mutex);

        //Send hosts list to client
        if(write(socketClient,hosts,strlen(hosts)) <= 0){
            free(hosts);
            break;
        }
        free(hosts);

        memset(read_buff,0,BUFFSIZE);

        //Read the choice from client
        while((nread=read(socketClient,read_buff,BUFFSIZE)) < 0){
            if(serverKilled)
                break;
        }
        if(serverKilled || nread==0)
            break;

        //read_buff[strlen(read_buff)] = '\0';

        host = gethostbyname(read_buff);
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

                if( writen(socketClient,buffInfo,sizeof(buffInfo)) < 0 ) {
                    break;
                }
            }
            else{
                strcpy(read_buff,"disconnected\0");
                write(socketClient,read_buff,strlen(read_buff)+1);  

                if( writen(socketClient,hostNode->time,strlen(hostNode->time)+1) < 0 ) {
                    break;
                }
            }
        }
    }

    printf("Client kill...\n");
    free((int *)arg);
    close(socketClient);

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
        serverKilled = true;
    }

    while(!serverKilled){
        //Accept is not blocking
        acceptResult = accept(sdClient,(struct sockaddr *)&client_addr,&size_client_addr);

        if(acceptResult != -1){
            sdClientLocal = (int *)malloc(sizeof(int));
            *sdClientLocal = acceptResult;

            pthread_create(&tid,NULL,handleClient,sdClientLocal);
            
            listInsert(sdContainer,tid);
        }
    }

    printf("ClientStub kill...\n");
    return NULL;
}

void * handleAgent(void * arg){

    agentInfo * info = (agentInfo *)arg;

    BSTNode * node;
    BSTNode * nodeFound;

    int ret; //To capture return value of readn() function
    int socketAgent = *(info->sd);

    bool inserted = false;
    time_t timer;

    char * currentTime;
    char * lastTime;
    
    long localKey;

    unsigned long read_buffer[3];

    //Set localKey to agent's IP.
    localKey = parseIP(info->IP);


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
    destroyAgentInfo(info);

    printf("Agent kill...\n");
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

            clientInfo = gethostbyaddr(&inAgentAddress,sizeof(inAgentAddress),AF_INET);

            //Get agent's IP as string
            agentIP = (char *)malloc(sizeof(char)*(strlen(inet_ntoa(agent_addr.sin_addr))+1));
            strcpy(agentIP,inet_ntoa(agent_addr.sin_addr));

            if(clientInfo != NULL){
                idAgent = (char *)malloc(sizeof(char)*(strlen(clientInfo->h_name)+1));
                strcpy(idAgent,clientInfo->h_name);
            }
            else{
                idAgent = (char *)malloc(sizeof(char)*(strlen(agentIP)+1));
                strcpy(idAgent,agentIP);
            }

            //Prepare struct to pass to thread
            info = (agentInfo *)malloc(sizeof(agentInfo));

            info->sd = socketAgent;
            info->time = instant;
            info->idhost = idAgent;
            info->IP = agentIP;

            pthread_create(&tid,NULL,handleAgent,info);
            listInsert(sdContainer,tid);
        }
    }
    
    printf("AgentStub kill...\n");
    
    return NULL;
}


int main(int argc, char * argv[]){

    signal(SIGPIPE,SIG_IGN);
    signal(SIGINT,sigintHandler);

    //Redirect stderr to stdout
    dup2(STDOUT_FILENO,STDERR_FILENO);

    if(argv[1]==NULL || argv[2]==NULL){
        printf("usage: %s <port_agent> <port_client>\n", argv[0]);
    }
    
    int port_agent = parsePort(argv[1]);

    int port_client = parsePort(argv[2]);

    printf("\nServer listening on ports %d (agents), %d (clients)...\n\n", port_agent, port_client);

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

    //Init struct containing mutex and bst root
    bstHostInfo = initBSTHostInfo();
    //Create list that will contain thread ids
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

    while(!serverKilled){}

    pthread_join(tid_agent,NULL);
    pthread_join(tid_client,NULL);

    node * root = sdContainer->next;
    while(root != NULL){
        printf("TID: %d\n", root->tid);
        pthread_join(root->tid,NULL);
        root = root->next;
    }

    if(close(sdAgent)<0){
        write(STDERR_FILENO,"Error closing agent socket.\n",29);
    }
    if(close(sdClient)<0){
        write(STDERR_FILENO,"Error closing client socket.\n",30);
    }

    destroyBSTHostInfo(bstHostInfo);
    listDestroy(sdContainer);

    return 0;

}