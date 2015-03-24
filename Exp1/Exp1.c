#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"

uint32_t i=0 ; 
uint32_t count=12;
 
void delay2 (uint32_t count) 
{  
		uint32_t j   ;

	for(j=0;j<count;j++);
}
 
__asm  delay1 (uint32_t count)
{  
		MOVS r1,r0 
	  MOVS r2,#0 
loop1 
		ADDS r2,r2,#1 
		CMP  r2,r1 
		BCC loop1 
		BX lr
}
 
void EINT1Callback(void)
{
	i++;
	delay1(10000);
	if(i>=3) i=0;
}
/*---------------------------------------------------------------*/
/*   MAIN function                                               */
/*---------------------------------------------------------------*/    

int main (void)
{	
	/*unlock the protected registers */  
		DrvSYS_UnlockProtectedReg();
	
	/* Enable the 12MHz oscillator oscillation */
 		DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
 
	/* Waiting for 12M Xtal stalble */
 		DrvSYS_Delay(5000);
 
/* HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator */
 		DrvSYS_SelectHCLKSource(0);	
	
	/*lock the protected registers */
		DrvSYS_LockProtectedReg();
	
	/*set GPA12 output mode*/	
		DrvGPIO_Open(E_GPC,12, E_IO_OUTPUT);
		DrvGPIO_Open(E_GPC,13, E_IO_OUTPUT);
		DrvGPIO_Open(E_GPC,14, E_IO_OUTPUT);
		DrvGPIO_Open(E_GPC,15, E_IO_OUTPUT);
 
  /*set GPB15 input mode*/	
  	DrvGPIO_Open(E_GPB,15, E_IO_INPUT);

/* Configure external interrupt */
    DrvGPIO_EnableEINT1(E_IO_FALLING, E_MODE_EDGE, EINT1Callback);

  while(1)
	{
    while (i==0)
		{
			count=(count==12)?15:(count-1);
			DrvGPIO_ClrBit(E_GPC,count);
 			delay2(1200000);
			DrvGPIO_SetBit(E_GPC,count);
		}
		while(i==1)
		{
			count=(count==15)?12:(count+1);
			DrvGPIO_ClrBit(E_GPC,count);
 			delay2(1200000);
			DrvGPIO_SetBit(E_GPC,count);
		}
		while(i==2)
		{
			DrvGPIO_ClrBit(E_GPC,count);
 			delay2(1200000);
			DrvGPIO_SetBit(E_GPC,count);
			delay2(1200000);
		}
	} 
}




