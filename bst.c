
#include "bst.h"


BSTNode * newNode(long key,char * idhost,char * time,unsigned long uptime,unsigned long freeram,unsigned long procs){
    BSTNode * newNode = (BSTNode *)malloc(sizeof(BSTNode));

    newNode->key = key;
    newNode->connected = true;

    newNode->idhost = (char *)malloc(sizeof(char)*(strlen(idhost)+1));
    strcpy(newNode->idhost,idhost);
    
    newNode->time = (char *)malloc(sizeof(char)*(strlen(time)+1));
    strcpy(newNode->time,time);

    newNode->uptime = uptime;
    newNode->freeram = freeram;
    newNode->procs = procs;

    newNode->sx = NULL;
    newNode->dx = NULL;

    return newNode;
}

void freeNode(BSTNode * node){
    free(node->idhost);
    free(node->time);
    
    free(node);
}

void bstDestroy(BSTNode * root) {
    if(root != NULL) {
        bstDestroy(root->sx);
        bstDestroy(root->dx);
        freeNode(root);
    }
}

BSTNode * bstSearch(BSTNode * root, long key) { 
    if (root == NULL || root->key == key) 
       return root; 
     
    if (root->key < key) 
       return bstSearch(root->dx, key); 
  
    return bstSearch(root->sx, key); 
} 


BSTNode * bstInsert(BSTNode * root, BSTNode * data) { 
    if (root == NULL) 
        return data; 
  
    if (data->key < root->key) 
        root->sx  = bstInsert(root->sx, data); 
    else if (data->key > root->key) 
        root->dx = bstInsert(root->dx, data);    
  
    return root; 
} 


void bstPrint(BSTNode * root){
    if(root != NULL){
        bstPrint(root->sx);
        printf("Key: %ld\nHost: %s\nState: %d\nTime: %s\n", root->key, root->idhost, root->connected,root->time);
        printf("Freeram: %lu\nUptime: %lu\nProcs: %lu\n\n", root->freeram, root->uptime, root->procs);
        bstPrint(root->dx);
    }
}


bool bstUpdate(BSTNode * root,BSTNode * data){
    if(root != NULL){
        if (root->key == data->key){
            //Update information
            strcpy(root->idhost,data->idhost);
            strcpy(root->time,data->time);

            root->uptime = data->uptime;
            root->freeram = data->freeram;
            root->procs = data->procs;

            freeNode(data);

            return true; 
        }
        
        if (root->key < data->key) 
            return bstUpdate(root->dx, data); 
    
        return bstUpdate(root->sx, data); 
    }
    else{
        return false;
    }
}


bool bstSetState(BSTNode * root,long key,bool state){
    if(root != NULL){
        if (root->key == key){
            //Set state
            root->connected = state;

            return true; 
        }
        
        if (root->key < key) 
            return bstSetState(root->dx, key, state); 
    
        return bstSetState(root->sx, key, state); 
    }
    else{
        return false;
    }
}

