#ifndef _AD5791_H_
#define _AD5791_H_
#include "stm32f0xx.h"
void ad5791_init(void);
void ad5791_set_code(uint32_t code);
void ad5791_set_volt(float volt);

#endif
