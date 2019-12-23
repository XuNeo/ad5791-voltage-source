/**
 * @author Neo Xu (neo.xu1990@gmail.com)
 * @license The MIT License (MIT)
 * 
 * Copyright (c) 2019 Neo Xu
 * 
 * @brief ezled-host, provide protocol to communicate with EZ-LED module(s)
*/
#include "ezled-host.h"
#include "serial_frame.h" //ezled use sframe protocol to transfer data
#include "string.h"
#include "ush.h"

#define CONTRAST_A          90
#define CONTRAST_B          20

#define CMD_SETBLINK        1       //start which led(s) to blink
#define CMD_SETBLINK_SPEED  2       //set the blink speed
#define CMD_SETCONTRASTA    3       //set the contrast level
#define CMD_SETCONTRASTB    4       //set the contrast level
#define CMD_PRINT           5       //print string to led.
#define CMD_SETSCROLL_SPEED 6       //set scroll speed
#define CMD_SAVE_SETTING    7       //save current settings as default settings.
#define CMD_ADD_FONT        8       //add temp font.


struct _ezled_ctrl
{
  uint8_t addr;   //EZ-LED module address
  uint8_t cmd;    //command
  uint8_t *data;  //the data for this command.
  uint8_t len;    //data length
};

void disp_uart_char(uint8_t c);
static void (*p_uart_char)(char) = disp_uart_char;
/**
 * Initialize EZ-LED host with a method to send char to uart.
*/
void ezled_host_init(void (*p)(char)){
  p_uart_char = p;
}

static int8_t ezled_send_cmd(uint8_t addr, uint8_t cmd, const char *data, uint8_t len){
  uint8_t buff[32];
  buff[0] = addr;  //addr
  buff[1] = cmd;
  if(len > 30) len = 30;
  buff[2] = len;
  for(int i=0; i<len;i++){
    buff[3+i] = data[i];
  }
  sframe_encode((void (*)(uint8_t))p_uart_char, buff, len + 3);
  return 0;
}

void ezled_print(const char *pstr){
  uint8_t len = strlen(pstr);
  ezled_send_cmd(0x52, CMD_PRINT, pstr, len>5?5:len);
  if(len>5){
    len -= 5;
    pstr += 5;
    ezled_send_cmd(0x4a, CMD_PRINT, pstr, len>5?5:len);
  }
  else
  {  
    ezled_send_cmd(0x4a, CMD_PRINT, " ", 1);
  }
}
static int32_t ush_ezled_print(uint32_t argc, char **argv){
  if(argc < 2) return 0;
  ezled_print(argv[1]);
  return 0;
}
USH_REGISTER(ush_ezled_print, print, print to led);


