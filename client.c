#include "client.h"

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
        g_hosts = printUpdatedList(sd,&hostsnumber);

        //choose the host and send his name to the server
        printf("\nChoose an host to take additional info: ");
        getInt(&choice,hostsnumber);

        g_hostname = getHostName(g_hosts[choice]);
        strcpy(write_buff,g_hostname);

        if(writen(sd,write_buff,strlen(write_buff)+1) < 0) {
            perror("error writing (send host name to the server)\n");
        }

        //get the host state (connected or disconnected)
        if( read(sd,read_buff,BUFFSIZE) < 0 ) {
            perror("error reading (get host state)\n");
        }

        if(strcmp(read_buff,"connected") == 0) {
            //read the host informations
            if( readn(sd,buffinfo,sizeof(buffinfo)) == -2 ) {
                perror("error reading (host informations)\n");
            } else {
                printf("uptime: %lu  freeram: %lu  procs: %lu\n",buffinfo[UPTIME],buffinfo[FREERAM],buffinfo[PROCS]);
            }
        } else {
            //read last registered date
            memset(read_buff,0,BUFFSIZE);
            if( read(sd,read_buff,BUFFSIZE) <= 0 ) {
                perror("error reading (last registered date)\n");
            } else {
                printf("last date: %s",read_buff);
            }
            memset(read_buff,0,BUFFSIZE);
        }

        destroyHosts(g_hosts);
        free(g_hostname);

        g_hosts = NULL;
        g_hostname = NULL;
    }

    printf("Server disconnected\n");
    return 0;
}