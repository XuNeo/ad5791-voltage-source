#include "fifo.h"
#include "serial_frame.h"
#include "ush.h"
#include "ad5791.h"
#include "uart.h"
#include "printf.h"

fifo_def uartrx_fifo;
ush_def ush;

static float curr_volt = 10; //current voltage setting.
static void _uart_rx_callback(uint8_t ch){
  fifo_write1B(&uartrx_fifo, ch);
}
/**
 * @brief init voltref related things(sw/hw)
 * @return none.
*/
void voltref_init(void){
	static uint8_t fifobuff[128];
  static char line_buff[128];
	uart_init(115200, _uart_rx_callback);
  fifo_init(&uartrx_fifo, fifobuff, 128);
  ush_init(&ush, line_buff, 128);
  ad5791_init();
  curr_volt = ad5791_set_volt(curr_volt);
}

//return current voltage settings.
float voltref_get_value(void){
  return curr_volt;
}
/**
 * @brief set the volatage.
*/
static int32_t ush_set_volt(uint32_t argc, char **argv){
  float volt;
  float real_volt;
  ush_num_def numtype;
  if(argc < 2) return 0;
  if(*(++argv) == 0) return 0;
  if(ush_str2num(*argv, 128, &numtype, &volt) != ush_error_ok){
    USH_Print("input string is not illegal\n");
  }
  else{
    if(numtype == ush_num_int32)
      volt = *(int32_t*)&volt;
    else if(numtype == ush_num_uint32)
      volt = *(uint32_t*)&volt;
    USH_Print("set output voltage to:%f\n", volt);
    real_volt = ad5791_set_volt(volt);
    USH_Print("Real output voltage is:%f\n", real_volt);
  }
  curr_volt = real_volt;
  hmi_disp_update();
  return 0;
}
USH_REGISTER(ush_set_volt, setvolt, Set the output voltage in V);

/**
 * @brief poll the input from usart and process it.
 * @return none.
*/
void voltref_loop(void){
  uint8_t ch;
  while(fifo_read1B(&uartrx_fifo, &ch) == fifo_err_ok){
    ush_process_input(&ush, (char*)&ch, 1);
  }
}
