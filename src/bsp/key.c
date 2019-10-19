#include "key.h"
#include "stm32f0xx.h"

void key_init(void){
  GPIO_InitTypeDef gpio_init;
  TIM_ICInitTypeDef TIM_ICInitStruct;
  /* Button-->PB1 */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  gpio_init.GPIO_Mode = GPIO_Mode_IN;
  gpio_init.GPIO_OType = GPIO_OType_OD;
  gpio_init.GPIO_Pin = GPIO_Pin_1;
  gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &gpio_init);

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  /* TIM1 channel 2 pin (PE.11) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Connect TIM pins to AF1 */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_1);
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 511;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* Prescaler configuration */
  TIM_PrescalerConfig(TIM3, 0, TIM_PSCReloadMode_Immediate);
  TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStruct.TIM_ICFilter = 0x0f;
  TIM_ICInit(TIM3, &TIM_ICInitStruct);
  TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
  TIM_ICInit(TIM3, &TIM_ICInitStruct);

  TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI1, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 10-1;        //10ms interrupt period.
  TIM_TimeBaseStructure.TIM_Prescaler = 64000-1; //64MHz clock input. 1kHz clock.
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
  TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);
  NVIC_InitTypeDef nvic;
  nvic.NVIC_IRQChannel = TIM14_IRQn;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  nvic.NVIC_IRQChannelPriority = 2;
  NVIC_Init(&nvic);
  TIM_Cmd(TIM14, ENABLE);
}

uint8_t get_encoder(void){
  return TIM3->CNT>>1;
}

static char Flag_KeyCheck = 0;
uint8_t get_key(void){
	uint16_t keystat = (~GPIOB->IDR)&GPIO_Pin_1; //if key is pressed, the bit is set.
  uint16_t key_value = 0;
  static uint16_t keystat_pre;
  static uint16_t counter;
  static uint8_t flag_key_cont;
  if(Flag_KeyCheck == 0) return 0;
  Flag_KeyCheck = 0;
  if((keystat_pre == keystat)&&(keystat)){// status not changed. and key is pressed.
    counter++;
    if(counter == 60){//0.6second
      counter = 0;
      key_value = KEY_PRESS_L|keystat_pre;
      flag_key_cont = 1;
    }
  }
  else{
    counter = 0;
    if((keystat^keystat_pre)&~keystat){//key value changed and edge is loosing the key.
      if(flag_key_cont){
        flag_key_cont = 0;
      }
      else
        key_value = keystat_pre;  //the key is from ON to off, so we use _pre to get which key is pressed.
    }
  }
  keystat_pre = keystat; //do not include the
  return key_value;
}

#include "ulog.h"

void TIM14_IRQHandler(void)//10ms
{
	if(TIM14->SR & TIM_IT_Update)	
	{    
		TIM14->SR = ~TIM_FLAG_Update;
    Flag_KeyCheck = 1;
    ulog_timer_isr();
	}
}
