#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/sysinfo.h>
#include <arpa/inet.h> //per il client, in particolare per la funzione inet_aton()

#define MIN_PORT 1024
#define MAX_PORT 65535 


int sd2 = 0;

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
    if(close(sd2)<0)
        perror("Errore nella chiusura!\n");
    exit(1);
} 

void handleClient(void * sd_ptr){
    int sd = *((int *)sd_ptr);

    unsigned long read_buffer[3];

    while(1){
        //Pulizia buffer di lettura
        memset(read_buffer,0,sizeof(read_buffer));

        if(read(sd,read_buffer,sizeof(read_buffer))==0){
            printf("Il client si e disconnesso!\n");
            break;
        }

        printf("Uptime: %lu Freeram: %lu Procs: %lu\n", read_buffer[0], read_buffer[1], read_buffer[2]);

        break;
    }  
}


int main(int argc, char * argv[]){

    //signal(SIGPIPE,sigpipeHandler);
    signal(SIGINT,sigintHandler);

    int sd1;
    struct sockaddr_in my_addr, client_addr;

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

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sd1 = socket(PF_INET,SOCK_STREAM,0);
    if(sd1 == -1)
        error("Error creating the socket!\n",1);

    if(bind(sd1,(struct sockaddr *)&my_addr,sizeof(my_addr)) == -1){
        error("Error binding address!\n",1);
    }

    if(listen(sd1,5) == -1){
        error("Error in listening!\n",1);
    }

    socklen_t size_client_addr;

    while(1){
        sd2 = accept(sd1,(struct sockaddr *)&client_addr,&size_client_addr);
        if(sd2 != -1){
            printf("Connection in - IP: %s - PORT: %d\n", inet_ntoa(client_addr.sin_addr), (int)ntohs(client_addr.sin_port));
            handleClient(&sd2);

            printf("Client has disconnected!\n");
            close(sd2);
        }
        else{
            error("Accept error!",1);
        }
    }

}