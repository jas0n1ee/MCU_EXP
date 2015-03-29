#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
/*Useful Segment Hex implement*/
#define SEG_N0   0x82 
#define SEG_N1   0xEE 
#define SEG_N2   0x07 
#define SEG_N3   0x46 
#define SEG_N4   0x6A  
#define SEG_N5   0x52 
#define SEG_N6   0x12 
#define SEG_N7   0xE6 
#define SEG_N8   0x02 
#define SEG_N9   0x42  
unsigned int SEG_BUF[10]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9}; 
unsigned int SEG_SGN[4]={0xDF,0xFF,0x9F,0x3F}; //_,|_,|-
unsigned int SEG_SCAN=0;
unsigned int KEY_SCAN=0;
unsigned int SEG_DISP[4]={0xFF,0xFF,0xFF,0xFF};
unsigned int KEY_SCAN_READY=0;
unsigned int keyboard[10]=0; 
unsigned int key_count=0;
unsigned int status=0;
unsigned int key_pressed=0;
unsigned int secound_count=0;
unsigned int speed=4;
unsigned int blink=0;
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
 

void GPIO_Setup(void)
{
	//Enable LED OUTPUT	
	DrvGPIO_Open(E_GPC,12, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPC,13, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPC,14, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPC,15, E_IO_OUTPUT);
 
 	//Enable KeyBoard IO INPUT
 	DrvGPIO_Open(E_GPA,0, E_IO_INPUT);
 	DrvGPIO_Open(E_GPA,1, E_IO_INPUT);
 	DrvGPIO_Open(E_GPA,2, E_IO_INPUT);
 	DrvGPIO_Open(E_GPA,3, E_IO_INPUT);
 	DrvGPIO_Open(E_GPA,4, E_IO_INPUT);
 	DrvGPIO_Open(E_GPA,5, E_IO_INPUT);
	
	//Enable Segment OUTPUT
	DrvGPIO_Open(E_GPC,4,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPC,5,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPC,6,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPC,7,E_IO_OUTPUT);

	DrvGPIO_Open(E_GPE,0,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPE,1,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPE,2,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPE,3,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPE,4,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPE,5,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPE,6,E_IO_OUTPUT);
	DrvGPIO_Open(E_GPE,7,E_IO_OUTPUT);

	/*set GPB15 input mode*/	
  	DrvGPIO_Open(E_GPB,15, E_IO_INPUT);
}
void Clear_Segment_Bits(void)
{
	DrvGPIO_ClrBit(E_GPC,4);
	DrvGPIO_ClrBit(E_GPC,5);
	DrvGPIO_ClrBit(E_GPC,6);
	DrvGPIO_ClrBit(E_GPC,7);
}
void show_seven_segment(unsigned char no, unsigned char number)
{
    unsigned char temp,i;
	temp=number;
	Clear_Segment_Bits();
	DrvGPIO_SetBit(E_GPC,4+no );
	for(i=0;i<8;i++)
    {
		if((temp&0x01)==0x01)
		   DrvGPIO_SetBit(E_GPE,i);
		   else
		   DrvGPIO_ClrBit(E_GPE,i);
	   	temp=temp>>1;
	}
}
void scankey(unsigned char n)
{
	unsigned char i;
	DrvGPIO_ClrBit(E_GPA,3+n);
	for(i=0;i<3;i++)
		keyboard[3*n+3-i]=1-DrvGPIO_GetBit(E_GPA,i);
	KEY_SCAN_READY=(n==2)?1:0;
	DrvGPIO_SetBit(E_GPA,3+n);
}
void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{ 
	TIMER0->TISR.TIF =1;
	SEG_SCAN=(SEG_SCAN==3)?0:SEG_SCAN+1;
	KEY_SCAN=(KEY_SCAN==2)?0:KEY_SCAN+1;
	show_seven_segment(3-SEG_SCAN,SEG_DISP[SEG_SCAN]);
	scankey(KEY_SCAN);
	if (KEY_SCAN_READY)
	{
		unsigned int i=0;
		for(i=1;i<10;i++)
			key_count+=keyboard[i];		//count and see if multiple key pressed
		if (key_count==1) 
		{
			key_pressed++;
			//if (status) delay2(240000);
			if (key_pressed==10)
			{
				for(i=1;i<10;i++)
					status=(keyboard[i]==1)?i:status;
				key_pressed=0;
			}
		}
		KEY_SCAN_READY=0;
		key_count=0;
	}
	secound_count=(secound_count==600)?1:secound_count+1;
	if((secound_count%300)==0&&(speed==16||speed==1))
	{
		blink=1-blink;
		SEG_DISP[1]=SEG_SGN[blink];
	}
}


void Timer_initial(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
  SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;				//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=0;		// Set Prescale [0~255]
	TIMER0->TCMPR  = 40000;			// Set TICR(TCMP) [0~16777215]
									//5ms

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;				//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;			//Reset up counter
	TIMER0->TCSR.CEN = 1;				//Enable Timer0

  TIMER0->TCSR.TDR_EN=1;			// Enable TDR function	
 
}

/*---------------------------------------------------------------*/
/*   MAIN function                                               */
/*---------------------------------------------------------------*/    

int main (void)
{	

	unsigned int count=15;
 	unsigned int old_stauts=0;
	unsigned int i;
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
	Timer_initial();

 	while(1)
	{
		old_stauts=(status==4||status==6)?status:old_stauts;
		speed=(speed<16&&status==2)?speed*2:speed;
		speed=(speed>1&&status==8)?speed/2:speed;
		status=old_stauts;
		switch(status)
		{
			case 0:
				DrvGPIO_ClrBit(E_GPC,12);
				DrvGPIO_ClrBit(E_GPC,13);
				DrvGPIO_ClrBit(E_GPC,14);
				DrvGPIO_ClrBit(E_GPC,15);
				/*
				SEG_DISP[0]=SEG_SGN[1];
				SEG_DISP[1]=SEG_SGN[0];
				SEG_DISP[2]=SEG_BUF[speed/10];
				SEG_DISP[3]=SEG_BUF[speed%10];
				*/
				//while(old_stauts==status);
				break;
			case 4:
				count=15;
				SEG_DISP[0]=SEG_SGN[2];
				SEG_DISP[1]=SEG_SGN[0];
				SEG_DISP[2]=SEG_BUF[speed/10];
				SEG_DISP[3]=SEG_BUF[speed%10];
		   		while (old_stauts==status)
				{
					count=(count==12)?15:(count-1);
					DrvGPIO_ClrBit(E_GPC,count);
		 			delay2(1200000/speed);
					DrvGPIO_SetBit(E_GPC,count);
				}
				break;
			case 6:
				count=12;
				SEG_DISP[0]=SEG_SGN[3];
				SEG_DISP[1]=SEG_SGN[0];
				SEG_DISP[2]=SEG_BUF[speed/10];
				SEG_DISP[3]=SEG_BUF[speed%10];
				while(old_stauts==status)
				{
					count=(count==15)?12:(count+1);
					DrvGPIO_ClrBit(E_GPC,count);
		 			delay2(1200000/speed);
					DrvGPIO_SetBit(E_GPC,count);
				}
		}
	}
}




