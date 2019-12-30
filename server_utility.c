
#include "server_utility.h"


void destroyAgentInfo(agentInfo * info){
    close(*(info->sd));
    free(info->sd);
    free(info->IP);
    free(info->idhost);
    free(info);
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

//Takes a string representig a port in the range MIN_PORT MAX_PORT and returns it as integer.
int parsePort(char * portToParse){
    int port = parseInt(portToParse);

    if(port<MIN_PORT || port>MAX_PORT){ //Conversion failed or port not in range
        printf("usage: ./program <port_agent> <port_client>. Insert valid port!\n");
        error(NULL,STDERR_FILENO,EPORT_NOTVALID);
    }

    return port;
}