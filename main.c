#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "list.h"
COSIM_INSTANCE cosimintance;
void main()
{

    printf("hello world \n");
    /* example
    cosimintance.Test = 0x22;
    printf("addr = 0x%x \n",(UINT32 *)(container_of(&cosimintance.Test,COSIM_INSTANCE,Test)));
   // */
    
     pNODE Listhead = NULL ;
     Listhead = CreatLinkList();
     UINT8 Len = GetLengthLinkList(Listhead);
     printf("list len = 0x%x \n",Len);
     InsertNodeLinkList(Listhead,0,&cosimintance,"cosim");
     DisplayLinkList(Listhead);
     DelNodeLinkList(Listhead,0);
     DisplayLinkList(Listhead);
     FreeMemory(&Listhead);


}
