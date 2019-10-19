#include "hmi.h"
#include "key.h"
#include "displed.h"
#include "timer.h"
#include "printf.h"
#include "stdbool.h"
/**
 * Get input from key/encoder and control display and peripherals.
*/
typedef enum _menu{
  menu_home = 0,
  menu_set_volt,
  menu_set_contrast,
  menu_last,
}main_menu_id_t;

typedef enum _set_volt_menu{
  volt_menu_x10 = 0,
  volt_menu_x1,
  volt_menu_x0p1,
  volt_menu_x0p01,
  volt_menu_x0p001,
  volt_menu_x0p0001,
  volt_menu_x0p00001,
  volt_menu_x0p000001,
  volt_menu_last,
}volt_menu_id_t;

#define MAX_MAIN_MENU menu_last

float voltref_get_value(void);
static int32_t submenu;
static main_menu_id_t main_menu;
static bool b_in_sub_menu = false;
static bool flag_update_disp;  //the flag means that display should be updated.
static bool b_volt_disp_decimal;
void hmi_timer(void){
  static uint16_t count_100ms = 0;
  count_100ms ++;
  if(count_100ms == 13){
    count_100ms = 0;
    b_volt_disp_decimal = !b_volt_disp_decimal;
    flag_update_disp = true;
    printf("swap volt display content\n");
  }
}

void hmi_init(void){
  main_menu = menu_home;
  key_init();
	displed_init();
  timer_register(hmi_timer, 100);
}

static void disp_update(void){
  #define MAX_CONTR_INDEX 3
  char str[16];
  if(flag_update_disp == false) return;
  flag_update_disp = false;
  switch (main_menu){
    case menu_home:
      { //display current voltage setting.
        float volt;
        volt = voltref_get_value();
        {
          char *pstr = str;
          snprintf(str, 16, "%.6f", volt);
          if(b_volt_disp_decimal)
            pstr = str+5;
          else
            str[5] = '\0';
          displed_str(pstr);
        }
      }
      break;
    case menu_set_volt:
      displed_str("sEt ");
      break;
    case menu_set_contrast:
      displed_str("CONt.");
      // if(menudepth == 0)
      //   displed_str("CONt."); //contraset
      // else{
      //   snprintf(str, 8, "C%03d", displed_getcontr());
      //   displed_str(str);
      //   if(menudepth == 1){
      //     displed_set_blink(0); //disable all.
      //     displed_highlight(3-submenu);
      //   }
      //   if(menudepth == 2){
      //     displed_set_blink(1<<(3-submenu));
      //   }
      // }
      break;
    default:
      break;
  }
}

static inline void _mainmenu_update(int8_t delta){
  if(delta)
    flag_update_disp = 1;//if main menu changed, we always need to update screen.
  main_menu += delta;
  if(main_menu >= MAX_MAIN_MENU)
    main_menu = MAX_MAIN_MENU-1;
  if(main_menu < 0)
    main_menu = 0;
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
  if(!b_in_sub_menu){//going through the main menu
    _mainmenu_update(encoder);
    if(key == KEY_OK){ //go to submemue;
      b_in_sub_menu = 1;
      submenu = 0;
      flag_update_disp = 1;
      return;
    }
  }
  else if(key == (KEY_PRESS_L|KEY_OK)){ //return from sub_menu to main menu
    flag_update_disp = 1;
    submenu = 0;
    b_in_sub_menu = false;
    return;
  }
  switch (main_menu){
    case menu_home:
        if(encoder){//adjust submenu position
          submenu += encoder;
          if(submenu < 0) submenu = 0;
          if(submenu > 7) submenu = 7;
        }
      // else if(menudepth == 2){//set voltage.
      //   if(encoder){//adjusting value
      //     float volt = voltref_get_value(); //get current settings.
      //     volt = volt + ipow(10, 8-submenu)*encoder;
      //   }
      // }
    break;
    case menu_set_volt:
      
      break;
    // case 1: //set contrast.
    //   if(menudepth == 1){//go through value
    //     if(encoder){//adjust submenu position
    //       submenu += -encoder;
    //       if(submenu < 0) submenu = 0;
    //       if(submenu > 2) submenu = 2;
    //     }
    //     if(key == KEY_OK){
    //       flag_update_disp = 1;
    //       menudepth = 2;
    //     }
    //   }
    //   else if(menudepth == 2){
    //     if(encoder){
    //       int8_t contr = displed_getcontr();
    //       contr += encoder*ipow(10, submenu);
    //       if(contr < 100&&contr>0)
    //         displed_setcontr(contr);
    //     }
    //     if(key == KEY_OK)
    //       menudepth = 1;
    //   }
    //   flag_update_disp = 1;
    //   break;
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
  }
  disp_update();
}
