#ifndef LIST_H
    #define LIST_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <pthread.h>
    #include <signal.h>

    typedef struct node node;
    struct node {
        pthread_t tid;

        node * next;
    };

    node * listCreate();

    void listInsert(node * head, pthread_t tid);
    void listDestroy(node * head);
    void listPrint(node * head);
#endif


