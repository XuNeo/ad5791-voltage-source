#include "hmi.h"
#include "key.h"
#include "displed.h"
#include "timer.h"
#include "printf.h"
#include "stdbool.h"

#define LOG_TAG              "hmi"
#define LOG_LVL              LOG_LVL_INFO
#include <ulog.h>

/**
 * Get input from key/encoder and control display and peripherals.
*/
float voltref_get_value(void);
float voltref_set_value(float volt);

static bool flag_update_disp = true;  //the flag means that display should be updated.
static int16_t cursor_pos = 0;
static bool b_cursor_selected = false;
static double volt_disp = 0;    //the voltage on screen
static float volt_real = 0;     //the real voltage set to DAC

void hmi_timer(void){
  static uint16_t count_100ms = 0;
  count_100ms ++;
  if(count_100ms == 13){
    count_100ms = 0;
    LOG_D("swap volt display content\n");
  }
}

void hmi_init(void){
  key_init();
	displed_init();
  timer_register(hmi_timer, 100);
}

static void disp_update(void){
  #define MAX_CONTR_INDEX 3
  char str[16];
  if(flag_update_disp == false) return;
  flag_update_disp = false;
  char *pstr = str;
  snprintf(str, 16, "%.6fv", b_cursor_selected?volt_disp:volt_real);
  if(b_cursor_selected){
    displed_set_blink(1<<(cursor_pos>3?cursor_pos-4:cursor_pos));
  }
  else{
    displed_set_blink(0); //no blink
    displed_highlight(4); //no led is highlighted.
    displed_highlight(cursor_pos>3?cursor_pos-4:cursor_pos);
  }
  if(cursor_pos>3)
    pstr = str+5;
  else
    str[5] = '\0';
  displed_str(pstr);
}

uint32_t ipow(uint32_t x, uint32_t y){
  uint32_t res = x;
  if(y == 0) return 1;
  if(y == 1) return res;
  y--;
  while(y--)
    res *= x;
  return res;
}

static void hmi_process_key(int8_t encoder, uint8_t key){
  flag_update_disp = true;
  if(key == KEY_OK){
    //toggle between selected mode.
    b_cursor_selected = !b_cursor_selected;
  }
  else if(encoder){
    if(b_cursor_selected){ //adjust voltage
      double delta = 1e-6*ipow(10, 6-cursor_pos)*encoder;
      delta += volt_disp;
      if(delta < 10.003f && delta>0)
        volt_disp = delta;
      if(volt_disp != voltref_get_value()){
        LOG_I("Set voltage to %fV",volt_disp);
        volt_real = voltref_set_value(volt_disp); //use the current voltage to set voltage reference.
        LOG_I("Real voltage is %fV",volt_real);
        printf("Real voltage is %fV\n",volt_real);
      }
    }
    else{ //change cursor position
      cursor_pos += encoder;
      if(cursor_pos<0)
        cursor_pos = 0;
      if(cursor_pos > 6)
        cursor_pos = 6;
    }
  }
}

void hmi_disp_update(float volt){
  flag_update_disp = 1;
  volt_disp = volt;
  volt_real = volt;
}

void hmi_poll(void){
  static uint8_t key_pre;
  static uint8_t encoder_pre;
  uint8_t key = get_key();
  uint8_t encoder = get_encoder();

  if((encoder != encoder_pre) || (key != key_pre)){
    int8_t temp;
    temp = (int8_t)(encoder - encoder_pre);
    encoder_pre = encoder;
    key_pre = key;
    hmi_process_key(temp, key);
    LOG_D("Encode delta:%d\n", temp);
  }
  disp_update();
}
