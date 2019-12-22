
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct BSTNode BSTNode;

struct BSTNode{
    int key;
    bool connected;

    char * idhost;
    char * time;

    unsigned long uptime;
    unsigned long freeram;
    unsigned long procs;

    BSTNode * dx;
    BSTNode * sx;
};


BSTNode * newNode(int key,char * idhost,char * time,unsigned long uptime,unsigned long freeram,unsigned long procs);
void bstDestroy(BSTNode * root);

BSTNode * bstSearch(BSTNode * root, int key);
BSTNode * bstInsert(BSTNode * root, BSTNode * data);
void bstPrint(BSTNode * root);
bool bstUpdate(BSTNode * root,BSTNode * data);
bool bstSetState(BSTNode * root,int key,bool state);
