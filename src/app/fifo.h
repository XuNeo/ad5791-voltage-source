#ifndef _FIFO_H_
#define _FIFO_H_
#include "stdint.h"
#include "stm32f0xx.h"

#define FIFO_DIS_INT() NVIC_DisableIRQ(USART1_IRQn)//disableInterrupts() //disable interrupt
#define FIFO_EN_INT()  NVIC_EnableIRQ(USART1_IRQn)//enableInterrupts() //enable interrupt again.

typedef enum
{
	fifo_err_ok = 0,
	fifo_err_full = -1,
	fifo_err_empty = -2,
  fifo_err_nullp = -3,  //null pointer provided.
}fifo_err_def;

typedef struct
{
  uint8_t *pbuff;
  uint32_t buff_size;
  uint32_t write_index;
  uint32_t read_index;
  uint32_t data_count;
}fifo_def;

void fifo_init(fifo_def *pfifo, uint8_t *pbuff, uint32_t len);
uint32_t fifo_status(fifo_def *pfifo);
fifo_err_def fifo_write1B(fifo_def *pfifo, uint8_t ch);
fifo_err_def fifo_read1B(fifo_def *pfifo, uint8_t *ch);
fifo_err_def fifo_write(fifo_def *pfifo, uint8_t *pbuff, uint32_t *plen);
fifo_err_def fifo_read(fifo_def *pfifo, uint8_t *pbuff, uint32_t *plen);

#endif
