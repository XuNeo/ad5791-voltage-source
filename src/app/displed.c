#include "serial_frame.h"
#include "string.h"
#include "ush.h"
#include "stm32f0xx.h"

#define LOG_TAG              "led"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

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

#define LEDSEGA     0x01
#define LEDSEGB     0x02
#define LEDSEGC     0x04
#define LEDSEGD     0x08
#define LEDSEGE     0x10
#define LEDSEGF     0x20
#define LEDSEGG     0x40
#define LEDSEGDP    0x80

static uint8_t curr_contrast = 3;

void disp_uart_char(uint8_t c){
	USART_SendData(USART2, (unsigned char) c);
	while (!(USART2->ISR & USART_FLAG_TC));
}

uint8_t displed_getcontr(void){
  return curr_contrast;
}

void displed_setcontr(uint8_t contr){
  uint8_t buff[8];  //maximum number to display is 10.123456
  if(contr > 100)
    contr = 100;
  USH_Print("Set contrast to %d\n", contr);
  buff[0] = 0;  //addr
  buff[1] = CMD_SETCONTRASTA;
  buff[2] = 3;
  buff[3] = 0xff;
  buff[4] = contr;
  curr_contrast = contr;
  sframe_encode(disp_uart_char, buff, 5);
}

//highlight the seledted led. led: 0 to 3
void displed_highlight(uint8_t led){
  uint8_t buff[8];
  if(led>3) return;
  buff[0] = 0;  //addr
  buff[1] = CMD_SETCONTRASTA;
  buff[2] = 3;
  buff[3] = 0;
  for(int8_t i=0;i<5;i++){
    buff[3] = 1<<i;
    buff[4] = led == i?CONTRAST_A:CONTRAST_B;//contrast_table[-led + 3 + i];
    sframe_encode(disp_uart_char, buff, 5);
  }
}

void displed_str(char *pstr){
  uint8_t buff[16];  //maximum number to display is 10.123456
  buff[0] = 0;  //addr
  buff[1] = CMD_PRINT;
  buff[2] = strlen(pstr);
  strcpy((char*)&buff[3], pstr);
  buff[buff[2]+3] = '\0';
  LOG_D("DispString:%s\n", &buff[3]);
  sframe_encode(disp_uart_char, buff, buff[2]+3);
}

void displed_set_blink(uint8_t pos){
  uint8_t buff[4];
  buff[0] = 0;  //addr
  buff[1] = CMD_SETBLINK;
  buff[2] = 1;
  buff[3] = pos;
  sframe_encode(disp_uart_char, buff, 4);
}

static int32_t ush_disp_str(uint32_t argc, char **argv){
  if(argc < 2) return 0;
  displed_str(argv[1]);
  return 0;
}
USH_REGISTER(ush_disp_str, disp, print string to led);

//static int32_t ush_disp_set_contrast(uint32_t argc, char **argv){
//  uint32_t contr_value;
//  ush_num_def numtype;
//  if(argc < 2) return 0;
//  if(ush_str2num(argv[1], 1, &numtype, &contr_value) != ush_error_ok)
//    return 0;
//  displed_setcontr(contr_value);
//  return 0;
//}
//USH_REGISTER(ush_disp_set_contrast, ledcontr, Set led contrast value);

//static int32_t ush_disp_save_setting(uint32_t argc, char **argv){
//  uint8_t buff[3];
//  buff[0] = CMD_SAVE_SETTING;
//  buff[1] = 0;
//  usart_for_led();
//  sframe_encode(uart_char, buff, 2);
//  usart_for_ush();
//  return 0;
//}
//USH_REGISTER(ush_disp_save_setting, ledsave, Save led settings);

void displed_default(void){
  uint8_t buff[5];
  buff[0] = 0;  //addr
  buff[1] = CMD_SETCONTRASTA;
  buff[2] = 2+1;
  buff[3] = 0xff;
  buff[4] = CONTRAST_A;
  sframe_encode(disp_uart_char, buff, 5);
  //set contrast B
  buff[1] = CMD_SETCONTRASTB;
  buff[2] = 2+1;
  buff[3] = 0xff; //all
  buff[4] = CONTRAST_B;
  sframe_encode(disp_uart_char, buff, 5);
  buff[1] = CMD_SETSCROLL_SPEED;
  buff[2] = 1+1;
  buff[3] = 5;  //scroll speed to 1.
  sframe_encode(disp_uart_char, buff, 4);
  buff[1] = CMD_SETBLINK_SPEED;
  buff[3] = 7;  //blink speed to 4.
  sframe_encode(disp_uart_char, buff, 4);
  buff[1] = 2;
  buff[2] = 0;
  sframe_encode(disp_uart_char, buff, 4);
}

void displed_addfont(uint8_t ch, uint8_t font){
  uint8_t buff[6];
  buff[0] = 0;  //addr
  buff[1] = CMD_ADD_FONT;
  buff[2] = 2;
  buff[3] = ch;
  buff[4] = font;
  sframe_encode(disp_uart_char, buff, 5);
}

void displed_init(void){
  
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
  
	displed_default();
  displed_addfont('N', LEDSEGA|LEDSEGB|LEDSEGC|LEDSEGE|LEDSEGF);
  displed_addfont('S', LEDSEGA|LEDSEGC|LEDSEGD|LEDSEGF|LEDSEGG);
  displed_addfont('v', LEDSEGC|LEDSEGD|LEDSEGE);
  displed_addfont('U', LEDSEGB|LEDSEGC|LEDSEGD|LEDSEGE|LEDSEGF);
	displed_str("    ");
  LOG_I("STM8-LED initialized.");
  //ulog_i("led", "STM8-LED initialized.");
}
//USH_REGISTER(displed_init, ledinit, re-init the led);
