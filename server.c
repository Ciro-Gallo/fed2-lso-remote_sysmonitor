
#include "server_utility.h"

//GLOBAL VARIABLES
node * sdContainer;
int sdAgent;
int sdClient;
bool serverKilled = false;
BSTHostInfo * bstHostInfo;

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
        
        //If agent closes socket or read remains blocked for more than 6 seconds check if it has reconnected.
        if((ret=read(socketAgent,read_buffer,sizeof(read_buffer))) <= 0){
            bstSetState(bstHostInfo->root,localKey,false);
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

    struct timeval timer_sock;

    timer_sock.tv_sec=6;
    timer_sock.tv_usec=0;

    //Set timeout of 6 seconds on read operations (from agents).
    if(setsockopt(sdAgent,SOL_SOCKET,SO_RCVTIMEO,(const char *)&timer_sock,sizeof(timer_sock))<0){
        serverKilled = true;
    }
    
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

    if(argc != NUM_ARGS){
        error("usage: ./cmd <port_agent> <port_client>\n",STDERR_FILENO,EARGS_NOTVALID);
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
        error("Error creating agent socket\n",STDERR_FILENO,ESOCK_CREATE);

    if(bind(sdAgent,(struct sockaddr *)&server_addr,sizeof(server_addr)) == -1){
        error("Error binding agent socket\n",STDERR_FILENO,ESOCK_BIND);
    }

    if(listen(sdAgent,5) == -1){
        error("Error preparing agent socket to accept connections\n",STDERR_FILENO,ESOCK_LISTEN);
    }

    sdClient = socket(PF_INET,SOCK_STREAM | SOCK_NONBLOCK,0);
    
    if(sdClient == -1)
        error("Error creating client socket\n",STDERR_FILENO,ESOCK_CREATE);

    if(bind(sdClient,(struct sockaddr *)&server_addr_client,sizeof(server_addr_client)) == -1){
        error("Error binding client socket\n",STDERR_FILENO,ESOCK_BIND);
    }

    if(listen(sdClient,MAX_CONN_NUMBER) == -1){
        error("Error preparing client socket to accept connections\n",STDERR_FILENO,ESOCK_LISTEN);
    }

    //Init struct containing mutex and bst root
    bstHostInfo = initBSTHostInfo();
    //Create list that will contain thread ids
    sdContainer = listCreate();

    pthread_t tid_agent, tid_client;

    //Creating stub thread for agent
    if(pthread_create(&tid_agent,NULL,handleAgentStub,NULL) != 0){
        error("Error creating stub thread for agent\n",STDERR_FILENO,ETHREAD_CREATE);
    }

    //Creating stub thread for client
    if(pthread_create(&tid_client,NULL,handleClientStub,NULL) != 0){
        error("Error creating stub thread for client\n",STDERR_FILENO,ETHREAD_CREATE);
    }

    while(!serverKilled){}

    pthread_join(tid_agent,NULL);
    pthread_join(tid_client,NULL);

    node * root = sdContainer->next;
    while(root != NULL){
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