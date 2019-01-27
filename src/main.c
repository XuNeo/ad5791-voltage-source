#include "stm32f0xx.h"
#include "hmi.h"
void voltref_init(void);
void voltref_loop(void);

int main(void)
{
	voltref_init();
	hmi_init();
	while(1)
	{
		voltref_loop();
		hmi_poll();
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
