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
#define CMD_SETCONTRASTC    10      //Set the contrast table for highlight.
#define CMD_PRINT           5       //print string to led.
#define CMD_SETSCROLL_SPEED 6       //set scroll speed
#define CMD_SAVE_SETTING    7       //save current settings as default settings.
#define CMD_ADD_FONT        8       //add temp font.
#define CMD_SET_HLIGHT      9       //set which led is to highlight.


#define LED_DISPLAY_BIT     5       //5 bit led display

struct _ezled_ctrl
{
  uint8_t addr;   //EZ-LED module address
  uint8_t cmd;    //command
  uint8_t *data;  //the data for this command.
  uint8_t len;    //data length
};

void disp_uart_char(uint8_t c);
static void (*p_uart_char)(char) = (void (*)(char))disp_uart_char;
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

/**
 * Check string length that one 5bit led display could fit.
*/
static uint8_t ezled_cast_str(const char *pstr){
  uint8_t pos = 0;  //led bit position
  uint8_t len = 0;
  while(*pstr){
    if(pos == LED_DISPLAY_BIT){
      if( *pstr == '.');
      else
        break;
    }
    len ++;
    if(*pstr != '.')
      pos ++;
    pstr ++;
  }
  return len;
}

void ezled_print(const char *pstr){
  uint8_t len_str = 0;
  const char *pfirst = pstr;
  len_str = ezled_cast_str(pstr);
  ezled_send_cmd(0x52, CMD_PRINT, pfirst, len_str);
  pstr += len_str;
  len_str = ezled_cast_str(pstr);
  if(len_str)
    ezled_send_cmd(0x4a, CMD_PRINT, pstr, len_str);
  else
    ezled_send_cmd(0x4a, CMD_PRINT, " ", 1);
}

static int32_t ush_ezled_print(uint32_t argc, char **argv){
  if(argc < 2) return 0;
  ezled_print(argv[1]);
  return 0;
}
USH_REGISTER(ush_ezled_print, print, print to led);

/**
 * Not work...
*/
void ezled_add_font(char c, uint8_t font){
  char buff[10];
  buff[0] = c;
  buff[1] = font;
  ezled_send_cmd(0x4a, CMD_ADD_FONT, buff, 2);
}
static int32_t ush_ezled_add_font(uint32_t argc, char **argv){
  //ush_error_def ush_str2num(const char *pstr, uint32_t len, ush_num_def* num_type, void *value);
  ush_num_def numtype;
  uint32_t value;
  if(argc < 2) return 0;
  if(ush_str2num(argv[2], strlen(argv[2]), &numtype, &value) == ush_error_ok){
		printf("Add font:{%c}=0x%02x\n", argv[1][0], value);
    ezled_add_font(argv[1][0], (uint8_t)value);
	}
	else{
		printf("Error in arguments");
	}
  return 0;
}

/**
 * Highlight selected led bit using position, 0~9
*/
void ezled_hightlight(uint16_t pos, uint8_t contrast_h, uint8_t contrast_l){
  char buff[32];
  for(int i=0; i<LED_DISPLAY_BIT-1; i++){
    buff[i] = contrast_l;
  }
  buff[LED_DISPLAY_BIT-1] = contrast_h;
  ezled_send_cmd(0x52, CMD_SETCONTRASTC, buff, LED_DISPLAY_BIT);
  ezled_send_cmd(0x4a, CMD_SETCONTRASTC, buff, LED_DISPLAY_BIT);
  ezled_send_cmd(0x4a, CMD_SETCONTRASTC, buff, LED_DISPLAY_BIT);
  if(pos<5){
    buff[0] = pos;
    ezled_send_cmd(0x52, CMD_SET_HLIGHT, buff, 1);
    buff[0] = LED_DISPLAY_BIT;
    ezled_send_cmd(0x4a, CMD_SET_HLIGHT, buff, 1);
  }
  else{
    pos -= 5;
    buff[0] = pos;
    ezled_send_cmd(0x4a, CMD_SET_HLIGHT, buff, 1);
    buff[0] = LED_DISPLAY_BIT;
    ezled_send_cmd(0x52, CMD_SET_HLIGHT, buff, 1);
  }
}
static int32_t ush_ezled_highlight(uint32_t argc, char **argv){
  //ush_error_def ush_str2num(const char *pstr, uint32_t len, ush_num_def* num_type, void *value);
  ush_num_def numtype;
  uint32_t value;
  if(argc < 2) return 0;
  if(ush_str2num(argv[1], strlen(argv[1]), &numtype, &value) == ush_error_ok){
		printf("highlight led %d", value);
    ezled_hightlight(value, 0xf0, 0x10);
	}
	else{
		printf("Error in arguments");
	}
  return 0;
}
USH_REGISTER(ush_ezled_highlight, hlight, highlight led 0 to 9);
