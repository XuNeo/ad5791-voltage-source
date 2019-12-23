#include "hmi.h"
#include "key.h"
#include "displed.h"
#include "timer.h"
#include "printf.h"
#include "stdbool.h"
#include "ezled-host.h"

#define LOG_TAG              "hmi"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

/**
 * Get input from key/encoder and control display and peripherals.
*/
float voltref_get_value(void);
float voltref_set_value(float volt);

const char *setting_menu[]={
  "1. sEt uOLt",
  "2. sEt CODE",
  "3. CAL",
  "4. sHO t",
};
const uint8_t cursor_range[][2]={
//cursor offset,  number of sub menu
  {2, 7},
  {3, 5},
  {2, 7},
  {0xff, 0xff},
};

double volt_set=10.0, volt_disp=10.0f, volt_vref = 10.9f, board_temp=30.123;
uint32_t code_set=0x12345;

static int16_t main_menu = 0;
static int16_t sub_menu = 0;
static int16_t menu_level = 0;  //0: root, 1: main menu, 2: sub menu
static uint16_t b_refresh_menu = 1;
#define MENU_LEVEL_ROOT     0 //root menu, used to display the real output voltage.
#define MENU_LEVEL_SHOW_MENU  1 //setting menu
#define MENU_LEVEL_SHOW_VALUE    2 //show the setting value
#define MENU_LEVEL_ADJ_VALUE   3 //adjust the setting value

void hmi_timer(void){
  static uint16_t count_100ms = 0;
  count_100ms ++;
  if(count_100ms == 13){
    count_100ms = 0;
    LOG_D("swap volt display content\n");
  }
}

void hmi_init(void){
  void disp_uart_char(uint8_t c);
  key_init();
	//displed_init();
  ezled_host_init((void(*)(char))disp_uart_char);
  timer_register(hmi_timer, 100);
}

static uint32_t ipow(uint32_t x, uint32_t y){
  uint32_t res = x;
  if(y == 0) return 1;
  if(y == 1) return res;
  y--;
  while(y--)
    res *= x;
  return res;
}

static void menu_refresh(void){
  char buff[32];
  if(!b_refresh_menu) return;
  b_refresh_menu = 0;
  if(menu_level == MENU_LEVEL_ROOT){  //root menu
    //show the real volate
    uint8_t len;
    sprintf(&buff[1], "%.6fu .", volt_disp);
    buff[0] = ' ';
    len = strlen(buff);
    if(len == 12)
      ezled_print(buff);
    else
      ezled_print(buff+1);
    ezled_hightlight(LED_NO_ONE);
    ezled_set_blink(9);
  }
  else if(menu_level == MENU_LEVEL_SHOW_MENU){
    // in setting menu
    ezled_print(setting_menu[main_menu]);
    ezled_hightlight(LED_NO_ONE);
    ezled_set_blink(LED_NO_ONE);
  }
  else {
    switch (main_menu){
      case 0: //setting voltage
        if(volt_set > 9.999999)
          sprintf(buff, "s%.6fu", volt_set);
        else
          sprintf(buff, "s %.6fu", volt_set);
        break;
      case 1: //setting code
        sprintf(buff, "0h %05x", code_set);
        break;
      case 2: //calibrate reference voltage.
        if(volt_vref > 9.999999)
          sprintf(buff, "r%.6fu", volt_vref);
        else
          sprintf(buff, "r %.6fu", volt_vref);
        break;
      case 3: //show temperature
        sprintf(buff, "%.3f c", board_temp);
        break;
      default:
        break;
    }
    ezled_print(buff); //print setting voltage value;
    if(cursor_range[main_menu][0] == 0xff){
      //do not display cursor;
      ezled_hightlight(LED_NO_ONE);
      ezled_set_blink(LED_NO_ONE);
    }
    else{
      uint8_t cursor = sub_menu+cursor_range[main_menu][0];
      if(menu_level == MENU_LEVEL_SHOW_VALUE){
        ezled_hightlight(cursor);
        ezled_set_blink(LED_NO_ONE);
      }
      else{//adjusting number now. should blink some led.
        ezled_set_blink(cursor);
        ezled_hightlight(LED_NO_ONE);
      }
    }
  }
}

static double float_adjust(double value, double max, int16_t encoder, int16_t position){
  double scale = 1e-6;
  double delta = scale*ipow(10, 6-position)*encoder;
  delta += value;
  if(delta < max)
    value = delta;
  if(value<0)
    value = 0;
  return value;
}

static void menu_navigate(int8_t encoder, uint8_t key){
  if(encoder || key)
    b_refresh_menu = 1;

  if(key == KEY_OK){
    if(menu_level == MENU_LEVEL_ADJ_VALUE){
      //value adjusting has been finished. Enter is pressed.
      //action now and exit to main menu.
      menu_level = MENU_LEVEL_SHOW_VALUE;
    }
    else if(menu_level == MENU_LEVEL_SHOW_VALUE && main_menu == 3){
      menu_level = 0; //directly exit from temperature menu.
    }
    else{
      menu_level ++;
    }
    LOG_I("menu level:%d",menu_level);
  }
  else if(key == (KEY_OK|KEY_PRESS_L)){
//    //long press to exit current menu-level
//    if(menu_level){
//      menu_level --;
//      //sub_menu = 0; //clear sub menu, retain main menu
//    }
    menu_level = 0;
    LOG_I("menu level:%d",menu_level);
  }
  else{
    //check encoder
    if(encoder){//encoder changed
      if(menu_level == MENU_LEVEL_ROOT){
        //ignore now. should be a quick set to voltage.
      }
      else if(menu_level == MENU_LEVEL_SHOW_MENU){
        //adjust main menu
        main_menu += encoder;
        if(main_menu < 0)
          main_menu = 0;
        else if(main_menu >= sizeof(setting_menu)/sizeof(char*)){
          main_menu = sizeof(setting_menu)/sizeof(char*)-1; //set to last menu.
        }
      LOG_I("main menu:%d",main_menu);
      }
      else if(menu_level == MENU_LEVEL_SHOW_VALUE){
        if(cursor_range[main_menu][0] == 0xff){

        }
        else{
          //adjust sub menu
          sub_menu += encoder;
          //assume the maximum value has 10 position.
          if(sub_menu < 0)sub_menu = 0;
          else if(sub_menu >= cursor_range[main_menu][1])sub_menu = cursor_range[main_menu][1]-1;
          LOG_I("sub menu:%d",sub_menu);
        }
      }
      else if(menu_level == MENU_LEVEL_ADJ_VALUE){
        //adjust current value
        switch(main_menu){
          case 0://set voltage
            volt_set = float_adjust(volt_set, volt_vref, encoder, sub_menu);
            volt_disp = voltref_set_value(volt_set);
            break;
          case 1: //set code
            break;
          case 2:// calibration
            volt_vref = float_adjust(volt_vref, 15.0f, encoder, sub_menu);
            break;
          default:
          break;
        }
      }
    }
  }
}

void hmi_disp_update(float volt){
  b_refresh_menu = 1;
  volt_disp = volt_set = volt;
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
    // hmi_process_key(temp, key);
    menu_navigate(temp, key);
    LOG_D("Encode delta:%d\n", temp);
  }
  // disp_update();
  menu_refresh();
}
