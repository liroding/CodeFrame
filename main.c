#include <stdio.h>
#include "main.h"

COSIM_INSTANCE cosimintance;

void main()
{

    printf("hello world \n"); 
    cosimintance.Test = 0x0;
    printf("addr = 0x%x \n",(UINT32 *)(container_of(&cosimintance.Test,COSIM_INSTANCE,Test)));
    //init platform 
    
        
}
