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
#include "stm32f0xx.h"

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

#define LEDSEGA     0x01
#define LEDSEGB     0x02
#define LEDSEGC     0x04
#define LEDSEGD     0x08
#define LEDSEGE     0x10
#define LEDSEGF     0x20
#define LEDSEGG     0x40
#define LEDSEGDP    0x80

#define LED_DISPLAY_BIT     5       //5 bit led display

void disp_uart_char(uint8_t c);
static void (*p_uart_char)(char) = (void (*)(char))disp_uart_char;

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

/**
 * Not work...
*/
void ezled_add_font(char c, uint8_t font){
  char buff[10];
  buff[0] = c;
  buff[1] = font;
  ezled_send_cmd(0x4a, CMD_ADD_FONT, buff, 2);
  ezled_send_cmd(0x52, CMD_ADD_FONT, buff, 2);
}

/**
 * Highlight selected led bit using position, 0~9
*/
void ezled_hightlight(uint16_t which){
  char buff[1];
  if(which<5){
    buff[0] = which;
    ezled_send_cmd(0x52, CMD_SET_HLIGHT, buff, 1);
    buff[0] = LED_DISPLAY_BIT;
    ezled_send_cmd(0x4a, CMD_SET_HLIGHT, buff, 1);
  }
  else if(which<10){
    which -= 5;
    buff[0] = which;
    ezled_send_cmd(0x4a, CMD_SET_HLIGHT, buff, 1);
    buff[0] = LED_DISPLAY_BIT;
    ezled_send_cmd(0x52, CMD_SET_HLIGHT, buff, 1);
  }
  else{ //disable highlight.
    buff[0] = (uint8_t)-1;
    ezled_send_cmd(0x4a, CMD_SET_HLIGHT, buff, 1);
    ezled_send_cmd(0x52, CMD_SET_HLIGHT, buff, 1);
  }
}

void ezled_set_blink(uint8_t which){
  char data;
  if(which < LED_DISPLAY_BIT){
    data = 1<<which;
    ezled_send_cmd(0x52, CMD_SETBLINK, &data, 1);
    data = 0;
    ezled_send_cmd(0x4a, CMD_SETBLINK, &data, 1);
  }
  else{
    data = 0;
    ezled_send_cmd(0x52, CMD_SETBLINK, &data, 1);
    which -= 5;
    data = 1<<which;
    ezled_send_cmd(0x4a, CMD_SETBLINK, &data, 1);
  }
}

void ezled_set_contrast(uint8_t contrast_a, uint8_t contrast_b){
  char buff[2];
  buff[0] = 0xff; //all LEDs
  buff[1] = contrast_a;
  ezled_send_cmd(0x52, CMD_SETCONTRASTA, buff, 2);
  ezled_send_cmd(0x4a, CMD_SETCONTRASTA, buff, 2);

  buff[1] = contrast_b;
  ezled_send_cmd(0x52, CMD_SETCONTRASTB, buff, 2);
  ezled_send_cmd(0x4a, CMD_SETCONTRASTB, buff, 2);
}

void ezled_set_hlight_contrast(uint8_t contrast_h, uint8_t contrast_l){
  char buff[32];
  for(int i=0; i<LED_DISPLAY_BIT-1; i++){
    buff[i] = contrast_l;
  }
  //the last data in buffer should be the highest contrast value
  buff[LED_DISPLAY_BIT-1] = contrast_h;
  ezled_send_cmd(0x52, CMD_SETCONTRASTC, buff, LED_DISPLAY_BIT);
  ezled_send_cmd(0x4a, CMD_SETCONTRASTC, buff, LED_DISPLAY_BIT);
}

void ezled_set_blink_speed(blink_speed_def speed){
  char data;
  data = speed;
  ezled_send_cmd(0x52, CMD_SETBLINK_SPEED, &data, 1);
  ezled_send_cmd(0x4a, CMD_SETBLINK_SPEED, &data, 1);
}

/**
 * Initialize EZ-LED host with a method to send char to uart.
*/
void ezled_host_init(void (*p)(char)){
  {
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//USART
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2,&USART_InitStructure);
	
	USART_Cmd(USART2,ENABLE);
  }
  p_uart_char = p;
  ezled_set_contrast(0xf0, 0x00);
  ezled_set_hlight_contrast(0xff, 0x10);
  //disable high light.
  ezled_hightlight(LED_NO_ONE);
  ezled_set_blink(LED_NO_ONE);
  ezled_set_blink_speed(BLINK_SPEED7);
  ezled_add_font('?', LEDSEGA|LEDSEGB|LEDSEGE|LEDSEGG);
  ezled_add_font('U', LEDSEGA|LEDSEGB|LEDSEGE|LEDSEGG);
}


//ush commands
static int32_t ush_ezled_blink(uint32_t argc, char **argv){
  //ush_error_def ush_str2num(const char *pstr, uint32_t len, ush_num_def* num_type, void *value);
  ush_num_def numtype;
  uint32_t value;
  if(argc < 2) return 0;
  if(ush_str2num(argv[1], strlen(argv[1]), &numtype, &value) == ush_error_ok){
		printf("blink led %d", value);
    ezled_set_blink(value);
	}
	else{
		printf("Error in arguments");
	}
  return 0;
}
USH_REGISTER(ush_ezled_blink, blink, blink led 0 to 9);

static int32_t ush_ezled_highlight(uint32_t argc, char **argv){
  //ush_error_def ush_str2num(const char *pstr, uint32_t len, ush_num_def* num_type, void *value);
  ush_num_def numtype;
  uint32_t value;
  if(argc < 2) return 0;
  if(ush_str2num(argv[1], strlen(argv[1]), &numtype, &value) == ush_error_ok){
		printf("highlight led %d", value);
    ezled_hightlight(value);
	}
	else{
		printf("Error in arguments");
	}
  return 0;
}
USH_REGISTER(ush_ezled_highlight, hlight, highlight led 0 to 9);

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
USH_REGISTER(ush_ezled_add_font, font, add led font);

static int32_t ush_ezled_print(uint32_t argc, char **argv){
  if(argc < 2) return 0;
  ezled_print(argv[1]);
  return 0;
}
USH_REGISTER(ush_ezled_print, print, print to led);
