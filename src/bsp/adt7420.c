#include "adt7420.h"
#include "i2c.h"
#include "printf.h"
#include "ush.h"
#include "timer.h"

#define MOVING_AVG_SIZE		32
#define adt7420_write_reg(addr, data)\
	IIC_WriteOneByte(ADT7420_ADDR,addr,data);

static int16_t temp_buff[MOVING_AVG_SIZE];
static uint32_t buff_index = 0;
static float latest_temp;
static int16_t b_tmp_ready = 0;
static int16_t b_read_tmp_now = 0;

//static uint8_t adt7420_readid(void){
//	uint8_t data;
//	IIC_ReadOneByte(ADT7420_ADDR,0x0b,&data);
//	return data;
//}

static float _adt7420_read_temp(void){
	static int16_t b_buffer_inited = 0;
  int16_t temp;
	uint8_t data;
	IIC_ReadOneByte(ADT7420_ADDR,0,&data);
  temp = data;
  temp <<= 8;
	IIC_ReadOneByte(ADT7420_ADDR,1,&data);
  adt7420_write_reg(3, 0x80|(1<<5));  //16bit resolution. one shot 
  temp |= data;
	if(!b_buffer_inited){
		b_buffer_inited = 1;
		for(int i=0; i<MOVING_AVG_SIZE; i++){
			temp_buff[i] = temp;
		}
	}
	temp_buff[buff_index++] = temp;
	if(buff_index == MOVING_AVG_SIZE)
		buff_index = 0;
	//get the average temperature;
	int32_t sum = 0;
	for(int i=0; i<MOVING_AVG_SIZE; i++)
		sum += temp_buff[i];
  latest_temp = sum*0.0078f/MOVING_AVG_SIZE;
	b_tmp_ready = 1;
	return latest_temp;
}

static void cmd_read_temp(void){
  USH_Print("temp: %f\n", latest_temp);
}
USH_REGISTER(cmd_read_temp, readtemp, read latest temperature);

static void adt7420_timer(void){
	b_read_tmp_now = 1;
}

void adt7420_init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//SDA
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	//SCL
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	//initialize buffer
	for(int i=0; i<MOVING_AVG_SIZE; i++){
		temp_buff[i] = 0;
	}

  IIC_Init();
  adt7420_write_reg(3, 0x80|(1<<5));  //16bit resolution. one shot 
  _adt7420_read_temp();
	timer_register(adt7420_timer, 500);	//500ms
}

void adt7420_poll(void){
	if(b_read_tmp_now){
		b_read_tmp_now = 0;
		_adt7420_read_temp();
	}
}

/**
 * get the latest temperature
*/
int32_t adt7420_get_tmp(float *t){
	*t = latest_temp;
	if(b_tmp_ready){
		b_tmp_ready = 0;
		return 1;
	}
	return 0;
}
