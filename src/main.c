#include "stm32f0xx.h"
#include "uart.h"
#include "xPrintf.h"
#include "ad5791.h"

int main(void)
{ 
	RCC_ClocksTypeDef RCC_Clocks;
  uart_init();
  xPrintf("Hello from STM32F030 xPrint.\n");
	RCC_GetClocksFreq(&RCC_Clocks);
	xPrintf("Clock Frequency:\nSysClk:%d\nHCLK:%d\nPCLK:%d\n", RCC_Clocks.SYSCLK_Frequency,\
				RCC_Clocks.HCLK_Frequency, RCC_Clocks.PCLK_Frequency);
  ad5791_init();
	while(1)
	{
	}
}

void TIM3_IRQHandler(void)//2ms
{//499.4Hz
	static unsigned char count_100mS;
	if(TIM3->SR & TIM_IT_Update)	
	{    
		TIM3->SR = ~TIM_FLAG_Update;  
		count_100mS ++;
		if(count_100mS == 250)
		{
			count_100mS = 0;
		}
	}
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)//2ms
{//499.4Hz
	static unsigned char count_100mS;
	if(TIM1->SR & TIM_IT_Update)	
	{    
		TIM1->SR = ~TIM_FLAG_Update;  
		count_100mS ++;
		if(count_100mS == 250)
		{
			count_100mS = 0;
		}
	}
}

void general_delay(void)
{
	int x,y;
	for(x=0;x<1000;x++)
	for(y=0;y<1000;y++);
}
