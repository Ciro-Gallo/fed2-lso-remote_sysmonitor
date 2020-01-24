
#include "../utility/utility.h"
#include <sys/sysinfo.h>
#include <netdb.h>

#define BUFFSIZE 3

int sd;

void handleSigInt(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
    exit (-1);
}

void handleSigPipe(int s) {
    if( close(sd) < 0 ) {
        perror("Error closing socket\n");
    }
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

    float buf[BUFFSIZE];
    struct sockaddr_in myaddress;
    struct sysinfo info;
    int port = argToInt(argv[1]);

    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(port);
    inet_aton(argv[2],&myaddress.sin_addr);

    if( (sd = socket(PF_INET,SOCK_STREAM,0)) < 0 ) {
        error("Error creating socket\n",STDOUT_FILENO,ESOCK_CREATE);
    }

    printf("Waiting the server...\n");
    if( connect(sd,(struct sockaddr*)&myaddress,sizeof(myaddress)) != 0 ) {
        error("Error during connection\n",STDOUT_FILENO,ESOCK_CONN);
    }
    
    float freeRamPercentage;
    const double megabytes = 1024 * 1024;

    while(1) {
        if( sysinfo(&info) != 0 ) {
            error("Error fetching system informations\n",STDOUT_FILENO,ESYS_INFO);
        } 
        freeRamPercentage = ramToPercentage(info.freeram,info.totalram);
        buf[UPTIME] = info.uptime;
        buf[FREERAM] = freeRamPercentage;
        buf[PROCS] = info.procs;
        printf("uptime = %.1f procs = %d\n",buf[UPTIME],(int)buf[PROCS]);
        printf ("free RAM: %5.1f MB  Perc: %2.1f\n", info.freeram / megabytes, freeRamPercentage);
        printf("-----------------------\n");
        
        if( writen(sd,buf,sizeof(buf)) < 0 ) {
            error("Error writing host informations\n",STDOUT_FILENO,EWRITE);
        }
        sleep(3);
    }
   
    return 0;
}
