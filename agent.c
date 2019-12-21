#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 3

int filed;

void sigIntHandler(int fd) {
  if( close(filed)<0 ) {
    printf("\nChiusura non riuscita\n");
  }
  printf("\nHo catturato SIGINT, chiusura...\n");
  exit (-1);
}

int main() {
  int fd;
  //ciao
  struct sysinfo str;
  if( sysinfo(&str) != 0 ) {
    printf("sysinfo error\n");
    exit (-1);
  }

  unsigned long wbuf[3];
  wbuf[0] = str.uptime;
  wbuf[1] = str.freeram;
  wbuf[2] = str.procs;
  printf("VALORI ORIGINALI: uptime = %ld   freeram = %lu   procs = %lu\n",wbuf[0],wbuf[1],wbuf[2]);
  printf("VALORI CASTATI: uptime = %lu   freeram = %lu   procs = %lu\n",wbuf[0],wbuf[1],wbuf[2]);

  char rbuf[BUFFSIZE];
  size_t bytes = BUFFSIZE;
  struct sockaddr_in myaddress;

  //inizializzazione connessione
  myaddress.sin_family = AF_INET;
  myaddress.sin_port = htons(5000);
  inet_aton("109.115.248.125",&myaddress.sin_addr);

  //creazione socket e connessione
  if( (fd = socket(PF_INET,SOCK_STREAM,0))<0 ) {
    perror("socket error\n");
    exit (-1);
  }
  if( connect(fd,(struct sockaddr*)&myaddress,sizeof(myaddress)) < 0 ) {
    perror("connect error\n");
    exit (-1);
  }

  filed = fd;
  signal(SIGINT,sigIntHandler);

  //avvio chat
  while(1) {
    write(STDOUT_FILENO,"\nScrivi: ",10);
    //fgets(wbuf,254,stdin);
    if( write(fd,wbuf,sizeof(wbuf)) != sizeof(wbuf) ) {
      perror("write error\n");
    }
    exit (-1);
    write(STDOUT_FILENO,"Il server sta scrivendo...\n",28);
    read(fd,rbuf,bytes);
    write(STDOUT_FILENO,rbuf,strlen(rbuf));

    memset(wbuf,0,sizeof(wbuf));
    memset(rbuf,0,sizeof(rbuf));
  }

}