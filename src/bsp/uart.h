#ifndef _USART_H_
#define _USART_H_
#include "stm32f0xx.h"

void uart_init(uint32_t baudrate, void(*pfunc)(uint8_t));
void uart_char(uint8_t data);

void usart_for_led(void);
void usart_for_ush(void);

#endif
