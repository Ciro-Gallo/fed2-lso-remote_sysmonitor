
#include <stdio.h>
#include <stdbool.h>


typedef struct BSTNode{
    int key;
    bool connected;

    char * idhost;
    char * time;

    unsigned long uptime;
    unsigned long freeram;
    unsigned long procs;

    BSTNode * dx;
    BSTNode * sx;
} BSTNode;


BSTNode * newNode(char * idhost,char * time,unsigned long uptime,unsigned long freeram,unsigned long procs);
void bstDestroy(BSTNode * root);

BSTNode * bstSearch(BSTNode * root, int key);
BSTNode * bstInsert(BSTNode * root, BSTNode * data);
void bstPrint(BSTNode * root);
bool bstUpdate(BSTNode * root,BSTNode * data);
void bstSetState(BSTNode * root,int key,bool state);
