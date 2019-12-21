
#include "list.h"

int main(){
    node * head = listCreate();
    listInsert(head,5);
    listInsert(head,6);
    listInsert(head,7);
    listInsert(head,8);

    listPrint(head);
    listDestroy(head);

    printf("\n");
}