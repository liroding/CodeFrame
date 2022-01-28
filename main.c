#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "list.h"
#include "defmalloc.h"

MemoryPool MemoryPoolInstance;
COSIM_INSTANCE cosimintance;

void main()
{

    printf("hello world \n");

    MemoryPoolInstance.Init = MemoryPoolInit;
    MemoryPoolInstance.Init(&MemoryPoolInstance);


    /* example
    cosimintance.Test = 0x22;
    printf("addr = 0x%x \n",(UINT32 *)(container_of(&cosimintance.Test,COSIM_INSTANCE,Test)));
   // */
    
     pNODE Listhead = NULL ;
     Listhead = CreatLinkList();
     UINT8 Len = GetLengthLinkList(Listhead);
     printf("list len = 0x%x \n",Len);
     InsertNodeLinkList(Listhead,0,&cosimintance,"cosim");
     InsertNodeLinkList(Listhead,1,&MemoryPoolInstance,"memalloc");

     Len = GetLengthLinkList(Listhead);
     printf("list len = 0x%x \n",Len);
     
     DisplayLinkList(Listhead);

     DelNodeLinkList(Listhead,0);
     DisplayLinkList(Listhead); 
     FreeMemory(&Listhead);


    char *Test1 = (char *)MemoryPoolInstance.MemAlloc(&MemoryPoolInstance,100);
    memcpy(Test1,"12312321312",sizeof("12312321312"));
    printf("!!! %s\n",Test1);
    memset(Test1,0,sizeof(Test1));



}
