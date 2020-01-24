#include "client_utility.h"

int sd,g_hnumber;
char ** g_hosts;
char* g_hostname;


void handleSigInt(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    if(g_hosts != NULL)
        destroyHosts(g_hosts,g_hnumber);

    exit (-1);
}

void handleSigPipe(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    if(g_hosts != NULL)
        destroyHosts(g_hosts,g_hnumber);

    if(g_hostname != NULL)
        free(g_hostname);

    error("Server disconnected\n",STDOUT_FILENO,SIGPIPE);
}

int main(int args, char** argv) {
    
    checkArgs(args,argv);
    if(signal(SIGINT,handleSigInt) == SIG_ERR) {
        error("Error setting handler\n",STDOUT_FILENO,ESIGNAL);
    }
    if(signal(SIGPIPE,handleSigPipe) == SIG_ERR) {
        error("Error setting handler\n",STDOUT_FILENO,ESIGNAL);
    }

    struct sockaddr_in myaddress;
    int port = argToInt(argv[1]);

    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(port);
    if(inet_aton(argv[2],&myaddress.sin_addr) == 0) {
        error("Not valid IP address\n",STDOUT_FILENO,EIP_NOTVALID);
    }

    if( (sd = socket(PF_INET,SOCK_STREAM,0)) < 0 ) {
        error("Error creating socket\n",STDOUT_FILENO,ESOCK_CREATE);
    }

    printf("Waiting the server...\n");
    if( connect(sd,(struct sockaddr*)&myaddress,sizeof(myaddress)) != 0 ) {
        error("Error during connection\n",STDOUT_FILENO,ESOCK_CONN);
    }

    printf("Connected\n\n");

    char read_buff[BUFFSIZE] = "";
    char write_buff[BUFFSIZE] = "";
    float buffinfo[3];

    if( read(sd,read_buff,BUFFSIZE) <= 0 ) {
        error("Error reading intro message\n",STDOUT_FILENO,EREAD);
    }
    printf("%s\n",read_buff);

    int choice;
    while(1) {
        //print the host list sent by the server and make an array of hosts
        g_hosts = printAndGetUpdatedList(sd,&g_hnumber);

        //choose the host and send his name to the server
        printf("\nChoose an host to take additional info: ");
        getInt(&choice,g_hnumber);

        g_hostname = getHostName(g_hosts[choice]);
        strcpy(write_buff,g_hostname);

        if(writen(sd,write_buff,strlen(write_buff)+1) < 0) {
            printf("error writing (send host name to the server)\n");
            break;
        }

        //get the host state (connected or disconnected)
        if( read(sd,read_buff,BUFFSIZE) <= 0 ) {
            printf("error reading (get host state)\n");

            releaseResources(g_hosts,g_hostname,g_hnumber);
            break;
        }

        printf("\n----------------------------\n");
        printf("Requested infos about host %s\n\n",g_hostname);

        if(strcmp(read_buff,"connected") == 0) {
            //read the host informations
            if( readn(sd,buffinfo,sizeof(buffinfo)) < 0 ) {
                printf("error reading (host informations)\n");

                releaseResources(g_hosts,g_hostname,g_hnumber);
                break;
            } 
            
            printf("- uptime: %.1f\n- freeRamPercentage: %.1f%% \n- procs: %d\n",buffinfo[UPTIME],buffinfo[FREERAM],(int)buffinfo[PROCS]);
        } else {
            //read last registered date
            memset(read_buff,0,BUFFSIZE);
            //read TIMELENGTH bytes in order to take only the date and not 
            //also the list
            if( read(sd,read_buff,TIMELENGTH) <= 0 ) {
                printf("error reading (last registered date)\n");
                
                releaseResources(g_hosts,g_hostname,g_hnumber);
                break;
            } 
            printf("last date: %s",read_buff);
            memset(read_buff,0,TIMELENGTH);
        }
        printf("----------------------------\n");

        releaseResources(g_hosts,g_hostname,g_hnumber);

        g_hosts = NULL;
        g_hostname = NULL;
    }

    printf("\nDisconnected from the server\n");
    return 0;
}