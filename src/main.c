#include "stm32f0xx.h"
#include "hmi.h"
#include "timer.h"

#define LOG_TAG              "main"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

void voltref_init(void);
void voltref_loop(void);
int ulog_init(void);
int ulog_console_backend_init(void);

int main(void)
{
  {int i=1000000;while(i--);}
#ifdef RT_USING_ULOG
  ulog_console_backend_init();
  ulog_init();
#endif
	timer_init(10);	//10ms period timer
	voltref_init();
	hmi_init();
  LOG_D("Loop starts here.");
	while(1)
	{
		voltref_loop();
		hmi_poll();
	}
}

#include "ush.h"
/**
 * @brief set the volatage.
*/
static int32_t sytem_reboot(uint32_t argc, char **argv){
  NVIC_SystemReset();
  return 0;
}
USH_REGISTER(sytem_reboot, reboot, reset system);

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
