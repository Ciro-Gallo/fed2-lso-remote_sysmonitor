#include "agent_utility.h"

int main(int args, char** argv) {

    checkArgs(args,argv);
    signal(SIGINT,handleSigInt);
    signal(SIGPIPE,handleSigPipe);

    unsigned long buf[BUFFSIZE];
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
        printf("uptime = %lu procs = %lu\n",buf[UPTIME],buf[PROCS]);
        printf ("free RAM   : %5.1f MB  Perc: %2.1f\n", info.freeram / megabytes, freeRamPercentage);
        
        if( writen(sd,buf,sizeof(buf)) < 0 ) {
            error("Error writing host informations\n",STDOUT_FILENO,EWRITE);
        }
        sleep(3);
    }
   
    return 0;
}
