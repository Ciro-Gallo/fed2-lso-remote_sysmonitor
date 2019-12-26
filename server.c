
#include "server.h"

//GLOBAL VARIABLES
node * sdContainer;
int sdAgent;
int sdClient;
pthread_attr_t threadAttributes;
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

void destroyBSTHostInfo(void){
    pthread_mutex_destroy(&bstHostInfo->mutex);
    bstDestroy(bstHostInfo->root);

    free(bstHostInfo);
}

void initBSTHostInfo(void){
    bstHostInfo = (BSTHostInfo *)malloc(sizeof(BSTHostInfo));
    bstHostInfo->root = NULL;

    pthread_mutex_init(&bstHostInfo->mutex,NULL);
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
    
    if(pthread_attr_destroy(&threadAttributes) != 0){
        write(STDERR_FILENO,"Error on attributes destroy",28);
    }

    //Main thread should wait until all threads end with pthread_exit()
    //TODO: Change sleep with syncro mechanism, such as conditional variable or recursive mutex
    sleep(5);


    destroyBSTHostInfo();
    listCloseAndDestroy(sdContainer);

    exit(-2);
} 


void * handleClient(void * arg){
    int socketClient = *(int *)arg;

    char * state;
    if(bstHostInfo->root == NULL)
        state = "There are no agents. Waiting for at least one registered host...";
    else
        state = "List of registered hosts. Pick one:\n";

    if(writen(socketClient,state,strlen(state)) <= 0){
        printf("Errore nella writen2!\n");
    }

    while(bstHostInfo->root == NULL){}
    
    pthread_mutex_lock(&bstHostInfo->mutex);

        char * hosts = bstGetHosts(bstHostInfo->root);
        printf("List to send to client: %s\n", hosts);
    
    pthread_mutex_unlock(&bstHostInfo->mutex);

    if(writen(socketClient,hosts,strlen(hosts)) <= 0){
        printf("Errore nella writen2!\n");
    }

    return NULL;

}

void * handleClientStub(void * arg){

    struct sockaddr_in client_addr;
    

    socklen_t size_client_addr = sizeof(client_addr);

    pthread_t tid;
    int sdClientLocal; 

    while(1){

        sdClientLocal = accept(sdClient,(struct sockaddr *)&client_addr,&size_client_addr);

        if(sdClientLocal != -1){

            printf("Client has connected!\n");

            pthread_create(&tid,&threadAttributes,handleClient,&sdClientLocal);
            
            listInsert(sdContainer,sdClientLocal,tid);
        }
        else{
            error("Accept error!",1);
        }
    }
}

void * handleAgent(void * arg){
    agentInfo * info = (agentInfo *)arg;

    int socketAgent = info->sd;
    bool inserted = false;
    time_t timer;

    char * currentTime;
    char * lastTime;
    
    long localKey;
    BSTNode * node;
    BSTNode * nodeFound;

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
        if(readn(socketAgent,read_buffer,sizeof(read_buffer)) == -2){
            sleep(6);

            nodeFound = bstSearch(bstHostInfo->root,localKey);
            if(strcmp(nodeFound->time,lastTime) == 0){
                //Agent has not reconnected
                printf("Agent set to disconnected!\n");
                bstSetState(bstHostInfo->root,localKey,false);
            }
            //Print bst
            //bstPrint(bstHostInfo->root);
            free(lastTime);

            //pthread_exit(NULL);
            break;
        }
        
        //printf("\nUptime: %lu Freeram: %lu Procs: %lu\n", read_buffer[UPTIME], read_buffer[FREERAM], read_buffer[PROCS]); 

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
                    printf("Host inserted!\n");
                }
                inserted = true;
            }
            else{ //Update of this agent (still connected)
                bstUpdate(bstHostInfo->root,node);
                printf("Host updated!\n");
            }

            //bstPrint(bstHostInfo->root);
            printf("\n");

        pthread_mutex_unlock(&bstHostInfo->mutex);

    }

    free(info->IP);
    free(info->idhost);
    free(info);

    printf("\nSafe-killing thread...\n");
    pthread_exit(NULL);
}


void * handleAgentStub(void * arg){

    struct sockaddr_in agent_addr;
    
    socklen_t size_agent_addr = sizeof(agent_addr);
    
    struct hostent * clientInfo;
    struct in_addr inAgentAddress;

    pthread_t tid;
    int sdAgent2;

    char * idAgent;
    char * instant;
    char * agentIP;

    time_t timer;
    agentInfo * info;

    while(1){

        sdAgent2 = accept(sdAgent,(struct sockaddr *)&agent_addr,&size_agent_addr);

        if(sdAgent2 != -1){

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

            info->sd = sdAgent2;
            info->time = instant;
            info->idhost = idAgent;
            info->IP = agentIP;

            pthread_create(&tid,&threadAttributes,handleAgent,info);
            
            listInsert(sdContainer,sdAgent2,tid);
        }
        else{
            error("Accept error!",1);
        }
    }
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

    //Declaring variables
    struct sockaddr_in server_addr;
    struct sockaddr_in server_addr_client;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_agent);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    server_addr_client.sin_family = AF_INET;
    server_addr_client.sin_port = htons(port_client);
    server_addr_client.sin_addr.s_addr = htonl(INADDR_ANY);

    sdAgent = socket(PF_INET,SOCK_STREAM,0);
    
    if(sdAgent == -1)
        error("Error creating the socket!\n",1);

    if(bind(sdAgent,(struct sockaddr *)&server_addr,sizeof(server_addr)) == -1){
        error("Error binding address!\n",1);
    }

    if(listen(sdAgent,5) == -1){
        error("Error in listening!\n",1);
    }

    sdClient = socket(PF_INET,SOCK_STREAM,0);
    
    if(sdClient == -1)
        error("Error creating the socket!\n",1);

    if(bind(sdClient,(struct sockaddr *)&server_addr_client,sizeof(server_addr_client)) == -1){
        error("Error binding address!\n",1);
    }

    if(listen(sdClient,5) == -1){
        error("Error in listening!\n",1);
    }


    if(pthread_attr_init(&threadAttributes) != 0){
        exit(-1);
    }
    if(pthread_attr_setdetachstate(&threadAttributes,PTHREAD_CREATE_DETACHED) != 0){
        exit(-1);
    }


    //Init struct containing mutex and bst root
    initBSTHostInfo();
    //Create list that will contain socket descriptors and thread ids
    sdContainer = listCreate();

    pthread_t tid_agent, tid_client;

    //Creating stub thread for agent
    if(pthread_create(&tid_agent,&threadAttributes,handleAgentStub,NULL) != 0){
        error("Error creating stub thread for agent\n",-3);
    }

    //Creating stub thread for client
    if(pthread_create(&tid_client,&threadAttributes,handleClientStub,NULL) != 0){
        error("Error creating stub thread for client\n",-3);
    }

    while(1){}

}