
#include "bst.h"


BSTNode * newNode(long key,char * idhost,char * time,unsigned long uptime,float freeRamPercentage,unsigned long procs){
    BSTNode * newNode = (BSTNode *)malloc(sizeof(BSTNode));

    newNode->key = key;
    newNode->height = 1;
    newNode->connected = true;

    newNode->idhost = (char *)malloc(sizeof(char)*(strlen(idhost)+1));
    strcpy(newNode->idhost,idhost);
    
    newNode->time = (char *)malloc(sizeof(char)*(strlen(time)+1));
    strcpy(newNode->time,time);

    newNode->uptime = uptime;
    newNode->freeRamPercentage = freeRamPercentage;
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

int height(BSTNode * node) { 
	if (node == NULL) 
		return 0; 
	return node->height; 
} 

int max(int a, int b) { 
	return (a > b)? a : b; 
} 

BSTNode * rightRotate(BSTNode * root) { 
	BSTNode * x = root->sx; 
	BSTNode * T2 = x->dx; 

	//Rotation 
	x->dx = root; 
	root->sx = T2; 

	//Update heights 
	root->height = max(height(root->sx), height(root->dx))+1; 
	x->height = max(height(x->sx), height(x->dx))+1; 
 
	return x; 
} 


BSTNode * leftRotate(BSTNode * root) { 
	BSTNode * y = root->dx; 
	BSTNode * T2 = y->sx; 

	//Rotation 
	y->sx = root; 
	root->dx = T2; 

	//Update heights 
	root->height = max(height(root->sx), height(root->dx))+1; 
	y->height = max(height(y->sx), height(y->dx))+1; 

	return y; 
} 

int getBalance(BSTNode * node) { 
	if (node == NULL) 
		return 0; 
	return height(node->sx) - height(node->dx); 
} 


BSTNode * bstInsert(BSTNode * root, BSTNode * data) { 
    if (root == NULL) 
        return data; 
    
    if (data->key < root->key){
        root->sx = bstInsert(root->sx, data); 
    }
    else if (data->key > root->key){
        root->dx = bstInsert(root->dx, data);   
    } 
  
	root->height = 1 + max(height(root->sx),height(root->dx)); 

	int balance = getBalance(root);

	// Left Left Case 
	if (balance > 1 && data->key < root->sx->key) 
		return rightRotate(root); 

	// Right Right Case 
	if (balance < -1 && data->key > root->dx->key) 
		return leftRotate(root); 

	// Left Right Case 
	if (balance > 1 && data->key > root->sx->key) { 
		root->sx = leftRotate(root->sx); 
		return rightRotate(root); 
	} 

	// Right Left Case 
	if (balance < -1 && data->key < root->dx->key) { 
		root->dx = rightRotate(root->dx); 
		return leftRotate(root); 
	} 

	return root; 
} 


void bstPrint(BSTNode * root){
    if(root != NULL){
        bstPrint(root->sx);
        printf("Key: %ld\nHost: %s\nState: %d\nTime: %s\n", root->key, root->idhost, root->connected,root->time);
        printf("freeRamPercentage: %.2f\nUptime: %lu\nProcs: %lu\n\n", root->freeRamPercentage, root->uptime, root->procs);
        bstPrint(root->dx);
    }
}


char * bstGetHosts(BSTNode * root){
    char * oldStrSx = NULL;
    char * oldStrDx = NULL;
    char * newStr = NULL;

    if(root != NULL){

        if(root->sx != NULL)
            oldStrSx = bstGetHosts(root->sx);

        if(root->dx != NULL)
            oldStrDx = bstGetHosts(root->dx);

        int dim = sizeof(char)*(strlen_custom(oldStrSx)+strlen_custom(oldStrDx)+strlen_custom(root->idhost)+(sizeof(char)*DIM_STATE)+(sizeof(char)*10));
        newStr = (char *)malloc(dim);
        memset(newStr,0,dim);

        if(root->sx != NULL){
            newStr[0] = '\0';
            strcat(newStr,oldStrSx);
            free(oldStrSx);
        }

        if(root->dx != NULL){
            if(root->sx == NULL)
                newStr[0] = '\0';
            else
                newStr[strlen(newStr)] = '\0';

            strcat(newStr,oldStrDx);
            free(oldStrDx);
        }

        //Concatenates local informations
        if(root->dx == NULL && root->sx == NULL)
            newStr[0] = '\0';
        
        strcat(newStr,root->idhost);

        if(root->connected)
            strcat(newStr," connected\n");
        else
            strcat(newStr," disconnected\n");

        return newStr;
    }

    //Never (recursively) called on NULL child
    return NULL;
}


bool bstUpdate(BSTNode * root,BSTNode * data){
    if(root != NULL){
        if (root->key == data->key){
            //Update information
            strcpy(root->idhost,data->idhost);
            strcpy(root->time,data->time);

            root->uptime = data->uptime;
            root->freeRamPercentage = data->freeRamPercentage;
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


