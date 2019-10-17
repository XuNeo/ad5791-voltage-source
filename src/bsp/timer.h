#ifndef _TIMER_H_
#define _TIMER_H_
#include "stdint.h"

void timer_init(uint32_t period_ms);
void timer_register(void (*call_back)(void), uint32_t period_ms);
void timer_unlink(void (*call_back));

#endif
