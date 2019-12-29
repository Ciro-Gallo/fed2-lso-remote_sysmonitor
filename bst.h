
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define DIM_STATE 13 //Length of string "disconnected"

typedef struct BSTNode BSTNode;

struct BSTNode{
    long key;
    bool connected;

    char * idhost;
    char * time;

    unsigned long uptime;
    unsigned long freeram;
    unsigned long procs;

    BSTNode * dx;
    BSTNode * sx;
};


BSTNode * newNode(long key,char * idhost,char * time,unsigned long uptime,unsigned long freeram,unsigned long procs);
void bstDestroy(BSTNode * root);

BSTNode * bstSearch(BSTNode * root, long key);
BSTNode * bstInsert(BSTNode * root, BSTNode * data);
void bstPrint(BSTNode * root);
bool bstUpdate(BSTNode * root,BSTNode * data);
bool bstSetState(BSTNode * root,long key,bool state);
char * bstGetHosts(BSTNode * root);