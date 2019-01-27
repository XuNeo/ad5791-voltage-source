#ifndef _DISP_LED_H_
#define _DISP_LED_H_

void displed_init(void);
void displed_str(char *pstr);
uint8_t displed_getcontr(void);
void displed_setcontr(uint8_t contr);
void displed_set_blink(uint8_t pos);
void displed_highlight(uint8_t led);

#endif
