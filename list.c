
#include "list.h"


node * listCreate(){
    node * head = (node *)malloc(sizeof(node));
    head->sd = -1;
    head->tid = -1;
    head->next = NULL;

    return head;
}

void listInsert(node * head,int sd,int tid){

    if(head->next != NULL){
        listInsert(head->next,sd,tid);
    }
    else{
        node * newnode = (node *)malloc(sizeof(node));
        newnode->sd = sd;
        newnode->tid = tid;

        newnode->next = NULL;

        head->next = newnode;
    }
}

void nodeDestroy(node * node){
    free(node);
}

void nodePrint(node * node){
    printf("sd: %d - tid: %d ", node->sd, node->tid);
}

void listDestroy(node * head){
    if(head->next != NULL){
        listDestroy(head->next);
    }
    nodeDestroy(head);
}

void listCloseAndDestroy(node * head){
    if(head->next != NULL){
        listDestroy(head->next);
    }
    if(head->sd >=0 && head->tid >= 0){
        close(head->sd);
        //pthread_kill(head->tid,SIGINT);
    }

    nodeDestroy(head);
}

void listPrint(node * head){
    if(head != NULL){
        nodePrint(head);
        listPrint(head->next);
    }
}