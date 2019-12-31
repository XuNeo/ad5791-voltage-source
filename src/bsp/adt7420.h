#ifndef _ADT7420_H_
#define _ADT7420_H_
#include "stdint.h"

#define ADT7420_ADDR (0x48<<1)
void adt7420_init(void);
void adt7420_poll(void);
int32_t adt7420_get_tmp(float *t);

#endif
