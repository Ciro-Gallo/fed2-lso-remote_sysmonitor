
#include "list.h"


node * listCreate(){
    node * head = (node *)malloc(sizeof(node));

    head->tid = -1;
    head->next = NULL;

    return head;
}

void listInsert(node * head, pthread_t tid){

    if(head->next != NULL){
        listInsert(head->next,tid);
    }
    else{
        node * newnode = (node *)malloc(sizeof(node));

        newnode->tid = tid;
        newnode->next = NULL;

        head->next = newnode;
    }
}

void listDestroy(node * head){
    if(head->next != NULL){
        listDestroy(head->next);
    }

    free(head);
}

void listPrint(node * head){
    if(head != NULL){
        printf("tid: %ld ", head->tid);
        listPrint(head->next);
    }
}