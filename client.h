


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

    ssize_t writen(int sd, const void* vptr, size_t n) {
        size_t nleft;
        ssize_t nwritten;
        const char* ptr;

        ptr = vptr;
        nleft = n;

        while(nleft > 0) {
            if( (nwritten = write(sd,ptr,nleft)) <= 0 ) {
                if( nwritten < 0 && errno == EINTR ) 
                    nwritten = 0;
                else 
                    return (-1);
            }

            nleft -= nwritten;
            ptr += nwritten;
        }
        return (n);
    } 

    ssize_t readn(int sd, void* vptr, size_t n) {
        size_t nleft;
        ssize_t nread;
        char * ptr;

        ptr = vptr;
        nleft = n;

        while(nleft > 0) {
            if( (nread = read(sd,ptr,nleft)) < 0 ) {
                if(errno == EINTR)
                    nread = 0;
                else 
                    return (-1);
            } else if(nread == 0)
                return (-2);
            
            nleft -= nread;
            ptr += nread;
        }
        return (n-nleft);
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

    int argToInt(char* arg) {
        char* p = NULL;
        int result = (int)strtol(arg,&p,10);
        if(p == NULL || *p != '\0') {
            return -1;
        }
        return result;
    }

    void checkArgs(int args, char** argv) {
        if(args != 3) {
            printf("usage: %s <port> <ipaddress>\n",argv[0]);
            exit (-1);
        }

        int port = argToInt(argv[1]);

        if(port < MIN_PORT || port > MAX_PORT) {
            perror("port overcome the range \n");
            exit (-1);
        }
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

#endif