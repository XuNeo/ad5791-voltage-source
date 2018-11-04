#include "key.h"
#include "stm32f0xx.h"

void key_init(void)
{
  GPIO_InitTypeDef gpio_init;
  TIM_TimeBaseInitTypeDef timerbase_init;
  /* Button-->PB1 */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  gpio_init.GPIO_Mode = GPIO_Mode_IN;
  gpio_init.GPIO_OType = GPIO_OType_OD;
  gpio_init.GPIO_Pin = GPIO_Pin_1;
  gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &gpio_init);
  
  timerbase_init.TIM_ClockDivision = TIM_CKD_DIV1;
}
