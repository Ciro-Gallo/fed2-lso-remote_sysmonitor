
#include "list.h"


node * listCreate(){
    node * head = (node *)malloc(sizeof(node));
    head->value = -1;
    head->next = NULL;

    return head;
}

void listInsert(node * head,int value){

    if(head->next != NULL){
        listInsert(head->next,value);
    }
    else{
        node * newnode = (node *)malloc(sizeof(node));
        newnode->value = value;
        newnode->next = NULL;

        head->next = newnode;
    }
}

void nodeDestroy(node * node){
    free(node);
}

void nodePrint(node * node){
    printf("%d ", node->value);
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
    close(head->value);
    nodeDestroy(head);
}

void listPrint(node * head){
    if(head != NULL){
        nodePrint(head);
        listPrint(head->next);
    }
}