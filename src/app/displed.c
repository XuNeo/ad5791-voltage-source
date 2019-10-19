#include "serial_frame.h"
#include "string.h"
#include "uart.h"
#include "ush.h"

#define LOG_TAG              "led"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

#define CMD_SETBLINK        1       //start which led(s) to blink
#define CMD_SETBLINK_SPEED  2       //set the blink speed
#define CMD_SETCONTRASTA    3       //set the contrast level
#define CMD_SETCONTRASTB    4       //set the contrast level
#define CMD_PRINT           5       //print string to led.
#define CMD_SETSCROLL_SPEED 6       //set scroll speed
#define CMD_SAVE_SETTING    7       //save current settings as default settings.
#define CMD_ADD_FONT        8       //add temp font.

#define LEDSEGA     0x01
#define LEDSEGB     0x02
#define LEDSEGC     0x04
#define LEDSEGD     0x08
#define LEDSEGE     0x10
#define LEDSEGF     0x20
#define LEDSEGG     0x40
#define LEDSEGDP    0x80

static uint8_t curr_contrast = 3;

uint8_t displed_getcontr(void){
  return curr_contrast;
}

void displed_setcontr(uint8_t contr){
  uint8_t buff[8];  //maximum number to display is 10.123456
  if(contr > 100)
    contr = 100;
  USH_Print("Set contrast to %d\n", contr);
  buff[0] = CMD_SETCONTRASTA;
  buff[1] = 2;
  buff[2] = 0xff;
  buff[3] = contr;
  curr_contrast = contr;
  usart_for_led();
  sframe_encode(uart_char, buff, 4);
  usart_for_ush();
}

//highlight the seledted led. led: 0 to 3
void displed_highlight(uint8_t led){
  uint8_t buff[8];
  if(led>3) return;
  buff[0] = CMD_SETCONTRASTA;
  buff[1] = 2;
  buff[2] = 0;
  usart_for_led();
  for(int8_t i=0;i<4;i++){
    buff[2] = 1<<i;
    buff[3] = led == i?70:5;//contrast_table[-led + 3 + i];
    sframe_encode(uart_char, buff, 4);
  }
  usart_for_ush();
}
void displed_str(char *pstr){
  uint8_t buff[16];  //maximum number to display is 10.123456
  buff[0] = CMD_PRINT;
  buff[1] = strlen(pstr);
  strcpy((char*)&buff[2], pstr);
  buff[buff[1]+2] = '\0';
  USH_Print("DispString:%s\n", &buff[2]);
  usart_for_led();
  sframe_encode(uart_char, buff, buff[1]+2);
  usart_for_ush();
}

void displed_set_blink(uint8_t pos){
  uint8_t buff[4];
  buff[0] = CMD_SETBLINK;
  buff[1] = 1;
  buff[2] = pos;
  usart_for_led();
  sframe_encode(uart_char, buff, 3);
  usart_for_ush();
}

static int32_t ush_disp_str(uint32_t argc, char **argv){
  if(argc < 2) return 0;
  displed_str(argv[1]);
  return 0;
}
USH_REGISTER(ush_disp_str, disp, print string to led);

static int32_t ush_disp_set_contrast(uint32_t argc, char **argv){
  uint32_t contr_value;
  ush_num_def numtype;
  if(argc < 2) return 0;
  if(ush_str2num(argv[1], 1, &numtype, &contr_value) != ush_error_ok)
    return 0;
  displed_setcontr(contr_value);
  return 0;
}
USH_REGISTER(ush_disp_set_contrast, ledcontr, Set led contrast value);

static int32_t ush_disp_save_setting(uint32_t argc, char **argv){
  uint8_t buff[3];
  buff[0] = CMD_SAVE_SETTING;
  buff[1] = 0;
  usart_for_led();
  sframe_encode(uart_char, buff, 2);
  usart_for_ush();
  return 0;
}
USH_REGISTER(ush_disp_save_setting, ledsave, Save led settings);

void displed_default(void){
  uint8_t buff[4];
  buff[0] = CMD_SETCONTRASTA;
  buff[1] = 2;
  buff[2] = 0xff;
  buff[3] = 30;  //contrast to 30%.
  usart_for_led();
  sframe_encode(uart_char, buff, 4);
  //set contrast B
  buff[0] = CMD_SETCONTRASTB;
  buff[1] = 2;
  buff[2] = 0xff; //all
  buff[3] = 3;  //contrast to 3%.
  sframe_encode(uart_char, buff, 4);
  buff[0] = CMD_SETSCROLL_SPEED;
  buff[1] = 1;
  buff[2] = 5;  //scroll speed to 1.
  sframe_encode(uart_char, buff, 3);
  buff[0] = CMD_SETBLINK_SPEED;
  buff[2] = 7;  //blink speed to 4.
  sframe_encode(uart_char, buff, 3);
  usart_for_ush();
}

void displed_addfont(uint8_t ch, uint8_t font){
  uint8_t buff[5];
  buff[0] = CMD_ADD_FONT;
  buff[1] = 2;
  buff[2] = ch;
  buff[3] = font;
  buff[4] = 0;
  usart_for_led();
  sframe_encode(uart_char, buff, 4);
  usart_for_ush();
}

void displed_init(void){
	displed_default();
  displed_addfont('N', LEDSEGA|LEDSEGB|LEDSEGC|LEDSEGE|LEDSEGF);
  displed_addfont('S', LEDSEGA||LEDSEGC|LEDSEGD|LEDSEGF|LEDSEGG);
  displed_addfont('v', LEDSEGC|LEDSEGD|LEDSEGE);
  displed_addfont('U', LEDSEGA|LEDSEGB|LEDSEGC|LEDSEGD|LEDSEGE);
	displed_str("10.123456");
  LOG_I("STM8-LED initialized.");
  //ulog_i("led", "STM8-LED initialized.");
}
USH_REGISTER(displed_init, ledinit, re-init the led);
