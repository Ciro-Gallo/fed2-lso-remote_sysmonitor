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
    
    const double megabyte = 1024 * 1024;

    while(1) {
        if( sysinfo(&info) != 0 ) {
            error("Error fetching system informations\n",STDOUT_FILENO,ESYS_INFO);
        } 
        buf[UPTIME] = info.uptime;
        buf[FREERAM] = info.freeram;
        buf[PROCS] = info.procs;
        printf("uptime = %lu freeram = %lu procs = %lu\n",buf[UPTIME],buf[FREERAM],buf[PROCS]);

        printf ("total RAM   : %5.1f MB\n", info.totalram / megabyte);
        printf ("free RAM   : %5.1f MB  Perc: %2.1f\n", info.freeram / megabyte, ((info.freeram / megabyte)*100)/(info.totalram / megabyte));
        
        if( writen(sd,buf,sizeof(buf)) < 0 ) {
            error("Error writing host informations\n",STDOUT_FILENO,EWRITE);
        }
        sleep(3);
    }
   
    return 0;
}
