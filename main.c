#include <stdio.h>
#include "main.h"
#include "list.h"
COSIM_INSTANCE cosimintance;

void main()
{

    printf("hello world \n");
   // /* example
    cosimintance.Test = 0x22;
    printf("addr = 0x%x \n",(UINT32 *)(container_of(&cosimintance.Test,COSIM_INSTANCE,Test)));
   // */

    ///*
     pNODE Listhead = NULL ;
     Listhead = CreatLinkList();
     UINT8 Len = GetLengthLinkList(Listhead);
     printf("list len = 0x%x \n",Len);
     InsertNodeLinkList(Listhead,1,&cosimintance,"cosim");
     InsertNodeLinkList(Listhead,2,&cosimintance,"cosim_2");
//     printf("0x%lx\n",((COSIM_INSTANCE *)(Listhead->pNext->pNext->Instance))->Test);
     DisplayLinkList(Listhead);
     // */
}
