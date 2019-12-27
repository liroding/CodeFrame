#include "list.h"


pNODE CreatLinkList(void)
{
        pNODE pTail = NULL , p_new = NULL;
        pNODE pHead = (pNODE)malloc(sizeof(NODE));
        if(pHead == NULL)
        {
                printf("malloc failed \n");
                return NULL;
        }
        pHead -> pPre     = NULL;
        pHead -> pNext    = NULL;
        pHead -> Instance = NULL;
        pHead -> IdName   = "Head";
        pTail = pHead;


        p_new = (pNODE)malloc(sizeof(NODE));
        if(p_new == NULL)
        {
                printf("malloc failed \n");
                return NULL;
        }
        p_new -> pPre     = pTail;
        p_new -> pNext    = NULL;
        p_new -> Instance = NULL;
        p_new -> IdName   = "Tail";
        pTail -> pNext    = p_new;
        pTail = p_new ;

        return pHead;
}
LSTATUS IsEmptyLinkList(IN pNODE pHead){

    pNODE pt = pHead -> pNext;
    if (pt = NULL)
    {
            printf("empty");
            return -1;
    }else{
            return 0;
    }
}
UINT8 GetLengthLinkList(pNODE pHead)
{
        UINT8 Length = 0;
        pNODE pt = pHead ->pNext ;
        while(pt != NULL)
        {
            Length ++;
            pt = pt ->pNext;
        }
        return Length;
}
LSTATUS InsertNodeLinkList(pNODE pHead,UINT16 pos,void *Instance,UINT8 *idname)
{
    pNODE pt = NULL, p_new = NULL;
    if( (pos>0)&& (pos< GetLengthLinkList(pHead))+1)
    {
        p_new = (pNODE)malloc(sizeof(NODE));
        printf("p_new addr = 0x%x 0x%x\n",p_new,NULL);
        if (p_new == NULL)
        {
            printf("empty");
            return -1;
        }

        while(1)
        {
                if(pos == 0)
                        break;
                pHead = pHead ->pNext;
                pos--;
        }
       
        printf("p_new addr = 0x%x 0x%x\n",p_new,NULL);
//pt = pHead -> pNext ;  //point the insert tail  
        pt = pHead -> pNext ;  //point the insert tail  
        p_new -> Instance = Instance;
        p_new -> pNext = pt ;
        if (pt != NULL) //insert point is last one
        {
                pt->pPre = p_new;
        }
        p_new ->pPre   = pHead ;
        pHead ->pNext  = p_new;
        p_new ->IdName = idname;
        return 1;
    }else{
        return -1;
    }
}
LSTATUS DelNodeLinkList(pNODE pHead,UINT16 pos);
LSTATUS FreeMemory(pNODE pHead);
LSTATUS DisplayLinkList(IN pNODE pHead)
{
    pNODE pt = pHead ->pNext ;
    printf("The List Id Name :\n");
    printf("-- %s --\n",pHead->IdName);
    while(pt != NULL)
    {
            printf("-- %s --\n",pt->IdName);
            pt = pt ->pNext;
    }
    return 0;
}
