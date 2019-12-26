#ifndef LIST_H_
#define LIST_H_
#include "main.h"

typedef UINT8  LSTATUS;

/*------------------------------------------

 NULL<-- [Head] <--> [Instance A ] <--> [Instance B] <--> [Tail] -- Null
                           |                  |                                              
                           |                  |
                       ————————           ————————  
                  Define Data Struct  Define Data Struct
------------------------------------------*/


typedef struct _NODE{
        struct _NODE *pPre;
        struct _NODE *pNext;
        void         *Instance;
        UINT8        Id;
}NODE,*pNODE;

pNODE CreatLinkList(void);
LSTATUS IsEmptyLinkList(IN pNODE pHead);


UINT8   GetLengthLinkList(IN pNODE pHead);
LSTATUS InsertNodeLinkList(IN pNODE pHead,IN UINT16 pos,IN void * Instance);
LSTATUS DelNodeLinkList(IN pNODE pHead,IN UINT16 pos);
LSTATUS FreeMemory(IN pNODE pHead);


#endif
