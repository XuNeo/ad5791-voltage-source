#ifndef _HMI_H_
#define _HMI_H_
#include "stdint.h"

void hmi_init(void);
void hmi_poll(void);
void hmi_disp_update(float volt);

#endif
