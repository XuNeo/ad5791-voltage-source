#include "stm32f0xx.h"
#include "timer.h"
#include "stdbool.h"

#define LOG_TAG              "timer"
#define LOG_LVL              LOG_LVL_INFO
#include <ulog.h>

#define CORE_CLOCK_FREQ (64000000/64000)
#define TIMER_LIST_MAX  10
struct _timer_list{
  bool enable;
  void (*callback)(void);
  uint32_t tick;
  uint32_t next_alarm_tick; //the next alarm time
}timer_list[TIMER_LIST_MAX];

static uint32_t list_len = 0;
static uint32_t curr_tick = 0;
static uint32_t time_per_tick = 0;  //time in ms per tick.

void timer_init(uint32_t period_ms){
  uint32_t timer_value = CORE_CLOCK_FREQ*period_ms/1000;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler = 64000-1; //64MHz clock input. 1kHz clock.
  TIM_TimeBaseStructure.TIM_Period = timer_value-1;        //10ms interrupt period.
  LOG_I("timer_load_value: %d", timer_value);
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM16, &TIM_TimeBaseStructure);
  TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
  TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
  NVIC_InitTypeDef nvic;
  nvic.NVIC_IRQChannel = TIM16_IRQn;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  nvic.NVIC_IRQChannelPriority = 2;
  NVIC_Init(&nvic);
  TIM_Cmd(TIM16, ENABLE);
  time_per_tick = timer_value*1000/CORE_CLOCK_FREQ;
  LOG_I("time_per_tick: %d", time_per_tick);
}

void timer_register(void (*call_back)(void), uint32_t period_ms){
  if(call_back == 0) return;

  LOG_I("Register timer: 0x%08x, period:%dms", call_back, period_ms);
  uint32_t tick = period_ms/time_per_tick;
  for(uint32_t i=0; i< list_len; i++){
    if(timer_list[i].enable == false ||
        timer_list[i].callback == call_back){
      //insert this callback here and return
      timer_list[i].callback = call_back;
      timer_list[i].tick = tick;
      timer_list[i].next_alarm_tick = curr_tick + tick;
      timer_list[i].enable = true;
      return;
    }
  }
  if(list_len == TIMER_LIST_MAX)
    return;
  //add it to end of list
  timer_list[list_len].callback = call_back;
  timer_list[list_len].tick = tick;
  timer_list[list_len].next_alarm_tick = curr_tick + tick;
  timer_list[list_len].enable = true;
  list_len ++;
}

void timer_unlink(void (*call_back)){
  for(uint32_t i=0; i< list_len; i++){
    if(timer_list[i].callback == call_back){
      //replace it with latest tick value and return.
      timer_list[i].enable = false;
      timer_list[i].next_alarm_tick = 0;
      return;
    }
  }
}

void TIM16_IRQHandler(void)//2ms
{
	if(TIM16->SR & TIM_IT_Update)	
	{    
		TIM16->SR = ~TIM_FLAG_Update;  
    curr_tick++;  //it need ~50days until it overflow if period is 1ms.
    for(uint32_t i=0; i<list_len; i++){
      if(timer_list[i].enable == true)
      if(curr_tick > timer_list[i].next_alarm_tick){
        timer_list[i].next_alarm_tick += timer_list[i].tick;
        timer_list[i].callback(); //call function
      }
      LOG_D("TIM16 INT.");
    }
  }
}
