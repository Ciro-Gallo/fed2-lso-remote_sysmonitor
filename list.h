#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

typedef struct node node;

struct node {
    int sd;
    int tid;

    node * next;
};


node * listCreate();

void listInsert(node * head,int sd,int tid);
void listDestroy(node * head);
void listCloseAndDestroy(node * head);
void listPrint(node * head);


#endif


