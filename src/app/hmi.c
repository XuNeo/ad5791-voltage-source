#include "hmi.h"
#include "key.h"
#include "displed.h"
#include "printf.h"

/**
 * Get input from key/encoder and control display and peripherals.
*/
#define MAINMENU_0  0
#define MAX_MAIN_MENU 2

float voltref_get_value(void);
static int32_t mainmenu, submenu, menudepth = 1;
static uint8_t flag_update_disp;  //the flag means that display should be updated.
void hmi_init(void){
  mainmenu = 0;
  submenu = 0;
  key_init();
	displed_init();
}

static void disp_update(void){
  #define MAX_CONTR_INDEX 3
  char str[8];
  if(flag_update_disp == 0) return;
  flag_update_disp = 0;
  switch (mainmenu){
    case 0:
      //if(menudepth == 0)
      { //display current voltage setting.
        float volt;
        volt = voltref_get_value();
        snprintf(str, 8, "%.6f", volt);
        displed_str(str);
      }
      break;
    case 1:
      if(menudepth == 0)
        displed_str("CONt."); //contraset
      else{
        snprintf(str, 8, "C%03d", displed_getcontr());
        displed_str(str);
        if(menudepth == 1){
          displed_set_blink(0); //disable all.
          displed_highlight(3-submenu);
        }
        if(menudepth == 2){
          displed_set_blink(1<<(3-submenu));
        }
      }
      break;
    case 2:
      displed_str("HEL.O"); //contraset
    default:
      break;
  }
}

static inline void _mainmenu_update(int8_t delta){
  if(delta)
    flag_update_disp = 1;//if main menu changed, we always need to update screen.
  mainmenu += delta;
  if(mainmenu > MAX_MAIN_MENU)
    mainmenu = MAX_MAIN_MENU;
  if(mainmenu < 0)
    mainmenu = 0;
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

static void hmi_process_key(int8_t encoder, uint8_t key){
  if(menudepth == 0){//going through the main menu
    _mainmenu_update(encoder);
    if(key == KEY_OK){ //go to submemue;
      menudepth = 1;
      flag_update_disp = 1;
      submenu = 0;
      return;
    }
  }
  if(key == (KEY_PRESS_L|KEY_OK)){
    if(menudepth){
      menudepth --;
      flag_update_disp = 1;
      submenu = 0;
    }
    return;
  }
  switch (mainmenu){
    case MAINMENU_0:
      if(menudepth == 1){ //go through voltage in 1.
        if(encoder){//adjust submenu position
          submenu += encoder;
          if(submenu < 0) submenu = 0;
          if(submenu > 7) submenu = 7;
        }
      }
      else if(menudepth == 2){//set voltage.
        if(encoder){//adjusting value
          float volt = voltref_get_value(); //get current settings.
          volt = volt + ipow(10, 8-submenu)*encoder;
        }
      }
      
      break;
    case 1: //set contrast.
      if(menudepth == 1){//go through value
        if(encoder){//adjust submenu position
          submenu += -encoder;
          if(submenu < 0) submenu = 0;
          if(submenu > 2) submenu = 2;
        }
        if(key == KEY_OK){
          flag_update_disp = 1;
          menudepth = 2;
        }
      }
      else if(menudepth == 2){
        if(encoder){
          int8_t contr = displed_getcontr();
          contr += encoder*ipow(10, submenu);
          if(contr < 100&&contr>0)
            displed_setcontr(contr);
        }
        if(key == KEY_OK)
          menudepth = 1;
      }
      flag_update_disp = 1;
      break;
    case 2:
      break;
    default:
      break;
  }
}

void hmi_disp_update(void){
  flag_update_disp = 1;
  disp_update();
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
    printf("Encode delta:%d\n", temp);
    disp_update();
  }
}
