#include "client_utility.h"

int getHostsDim(char ** hosts) {
    int j = 0;
    while(hosts[j] != NULL) 
        j++;

    return j;
}

void destroyHosts(char ** hosts) {
    for(int i = 0; i < hostsnumber; ++i) {
        free(hosts[i]);
    }
    free(hosts);
}

void getInt(int * elem, int b) {
    int ret = scanf("%d",elem);
    bool valid = false;

        if(ret != 0) {
        if(*elem < b && *elem >= 0)
            valid = true;
    } 

    while(ret == 0 || !valid) {
        printf("errore di input: inserire un intero\n");
        while(getchar()!='\n') {}
        ret = scanf("%d",elem);
        if(ret != 0) {
            if(*elem < b && *elem >= 0)
                valid = true;
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

char** hostsToArray(char * buff, int * hnumber) {
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
    *hnumber = j;

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


char** printUpdatedList(int sockd, int * hnumber) {
    char buff[BUFFSIZE];
    
    //synchronize client and server
    if( writen(sockd,"ready",6) < 0 ) {
        perror("error writing (synchronization)\n");
        printf("Server disconnected\n");
        exit (-1);
    }
    memset(buff,0,BUFFSIZE); 

    //get the hosts list
    if( read(sockd,buff,BUFFSIZE) <= 0 ) {
        perror("error reading (get hosts list)\n");
        printf("Server disconnected\n");
        exit (-1);
    }

    //index and print the hosts list
    char ** hosts = hostsToArray(buff,hnumber);
    printHosts(hosts);

    return hosts;
}

void releaseResources(char ** hosts, char * hostname) {
    destroyHosts(g_hosts);
    free(g_hostname);
}