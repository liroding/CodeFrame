#ifndef MAIN_H_
#define MAIN_H_ 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define IN 
#define OUT

typedef unsigned long long UINT64 ; 
typedef unsigned int       UINT32 ;
typedef unsigned short     UINT16 ;
typedef unsigned char      UINT8 ;
typedef char      	   INT8 ;

typedef UINT8              STATUS ;

//#define offsetof(TYPE,MEMBER)((size_t)&(((TYPE *)0)->MEMBER))
#define container_of(ptr,type,member)( { \
                const typeof(((type *)0)->member) *_mptr = (ptr); \
               (type *)((char *)_mptr - offsetof(type,member)); \
                })


/************** Service Pointer******************/
//handlefile service pointertypedef STATUS (*CFGFILEHANDLE)(IN UINT8 *filename );   
typedef STATUS (CONFIG_FILEHANDLE)(IN UINT8 *filename );   
typedef UINT8 *(GETMEM)(IN UINT64 len /*byte*/);    

typedef struct _COSIMSERVICE{
    CONFIG_FILEHANDLE  *CfgFileHandle;
    GETMEM             *GetMem; 
}COSIM_SERVICE;
/***********************************************/

typedef struct _COSIMINSTANCE{
        UINT8 *Name;
        COSIM_SERVICE   Service;
        UINT8 Test;
}COSIM_INSTANCE;

#endif
