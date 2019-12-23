/**
 * @author Neo Xu (neo.xu1990@gmail.com)
 * @license The MIT License (MIT)
 * 
 * Copyright (c) 2019 Neo Xu
 * 
 * @brief ezled-host, provide protocol to communicate with EZ-LED module(s)
*/
#ifndef _EZLED_HOST_H_
#define _EZLED_HOST_H_

typedef enum{
  BLINK_SPEED0 = 0,  //lowest speed
  BLINK_SPEED1,
  BLINK_SPEED2,
  BLINK_SPEED3,
  BLINK_SPEED4,
  BLINK_SPEED5,
  BLINK_SPEED6,
  BLINK_SPEED7,
  BLINK_SPEED8,
  BLINK_SPEED9,      //highest speed
}blink_speed_def;

#define LED_NO_ONE      11  //an invalid led position, used to disable some feature.

void ezled_host_init(void (*p)(char));
void ezled_print(const char *pstr);
void ezled_hightlight(uint16_t which);
void ezled_set_blink(uint8_t which);

#endif
