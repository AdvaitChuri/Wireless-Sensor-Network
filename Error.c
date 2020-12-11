/*---------------FILE: Error.c-----------------*/

/*
by:   Advait Churi
      ID: 01738336

PURPOSE
Error.c display error type
*/

/*Include micrium and STM headers*/
#include "includes.h"
/*Include Error module header*/
#include "Error.h"
        
void Err(CPU_INT08U *errorType)                    /*Get first charater*/    
{
  BSP_Ser_Printf("\a *** ERROR: %s\n",errorType); /*Print error type*/    
  
}