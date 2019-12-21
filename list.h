#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct node node;

struct node {
    int value;
    node * next;
};


node * listCreate();

void listInsert(node * head,int value);
void listDestroy(node * head);
void listPrint(node * head);


#endif


