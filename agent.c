#include "agent.h"

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
        perror("Error creating socket\n");
        exit (-1);
    }

    if( connect(sd,(struct sockaddr*)&myaddress,sizeof(myaddress)) != 0 ) {
        perror("Error during the connection\n");
        exit (-1);
    }
    

    while(1) {
        if( sysinfo(&info) != 0 ) {
            perror("Error fetching system informations\n");
            exit (-1);
        } 
        buf[UPTIME] = info.uptime;
        buf[FREERAM] = info.freeram;
        buf[PROCS] = info.procs;
        printf("uptime = %lu freeram = %lu procs = %lu\n",buf[UPTIME],buf[FREERAM],buf[PROCS]);
        
        if( writen(sd,buf,sizeof(buf)) == -1 ) {
            perror("Error writing\n");
            exit (-1);
        }
        sleep(3);
    }
   
    return 0;
}
