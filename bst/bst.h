#ifndef BST_H
    #define BST_H

    #include "../utility/utility.h"
    
    #include <stdbool.h>

    #define DIM_STATE 13 //Length of string "disconnected"

    typedef struct BSTNode BSTNode;

    struct BSTNode{
        long key;
        bool connected;

        char * idhost;
        char * time;

        unsigned long uptime;
        float freeRamPercentage;
        unsigned long procs;


        int height;
        BSTNode * dx;
        BSTNode * sx;
    };


    BSTNode * newNode(long key,char * idhost,char * time,unsigned long uptime,float freeRamPercentage,unsigned long procs);
    void bstDestroy(BSTNode * root);

    BSTNode * bstSearch(BSTNode * root, long key);
    BSTNode * bstInsert(BSTNode * root, BSTNode * data);
    void bstPrint(BSTNode * root);
    bool bstUpdate(BSTNode * root,BSTNode * data);
    bool bstSetState(BSTNode * root,long key,bool state);
    char * bstGetHosts(BSTNode * root);

#endif