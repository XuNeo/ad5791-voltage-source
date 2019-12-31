#include "i2c.h"

static volatile uint8_t IIC_InitFlag =0 ;

void IIC_Delay(uint8_t time)
{
  while(time--);
}

void IIC_Init(void)
{
	//SDA-->PB7 SCL-->PB6
	if(IIC_InitFlag ==0 )
	{
		//GPIO_Init(GPIOB, (GPIO_Pin_TypeDef)(GPIO_PIN_4|GPIO_PIN_5), GPIO_MODE_OUT_OD_HIZ_FAST);
		IIC_InitFlag = 1;
	}
}

void IIC_Start(void){
	SDA_H();
	IIC_Delay(255);
	SCL_H();
	IIC_Delay(255);
	SDA_L();	
	IIC_Delay(255);
	SCL_L();
}
void IIC_Stop(void){
	SCL_L();
	SDA_L();
	IIC_Delay(255);
	SCL_H();
	IIC_Delay(255);
	SDA_H();
	IIC_Delay(255);
}	 
unsigned char IIC_WaitACK(void){
	int i=0-10000;
	SCL_L();
	IIC_Delay(255);
	SDA_H();
	IIC_Delay(255);
	SCL_H();
	IIC_Delay(255);
	while((SDA_Status())&&i)i++;
	SCL_L();
	IIC_Delay(255);
	if(i==0){
		IIC_Stop();
	IIC_Delay(255);
		return 0;
	}
	return 1;
}
void IIC_ACK(void){
	SCL_L();
	IIC_Delay(255);
	SDA_L();
	IIC_Delay(255);
	SCL_H();
	IIC_Delay(255);
	SCL_L();
}

void IIC_NACK(void){
	SCL_L();
	IIC_Delay(255);
	SDA_H();
	IIC_Delay(255);
	SCL_H();
	IIC_Delay(255);
	SCL_L();
}

void IIC_WriteByte(unsigned char byte){
	unsigned char i;
	for(i=0;i<8;i++){
		SCL_L();
		IIC_Delay(255);
		if(byte&0x80)
		{
			SDA_H();
			IIC_Delay(255);
		}
		else
		{
			SDA_L();
			IIC_Delay(255);
		}
		byte<<=1;
		SCL_H();
		IIC_Delay(255);
	}
	SCL_L();
	IIC_Delay(255);
}
unsigned char IIC_ReadByte(void){
	unsigned char i,temp=0;
	SCL_L();
	IIC_Delay(255);
	SDA_H();
	IIC_Delay(255);
	for(i=0;i<8;i++){
		SCL_H();
		IIC_Delay(255);
		temp<<=1;
		temp |=SDA_Status();
		SCL_L();
		IIC_Delay(255);
	}
	return temp;
}
uint8_t IIC_WriteOneByte(uint8_t DEV_Addr,uint16_t addr,uint8_t Wdata)
{
	int i=-10000;
	do
		{
			IIC_Start();
			IIC_WriteByte(IIC_WRITE|DEV_Addr);
			if(IIC_WaitACK())
				break;
		}while(i++);//wait the write operation to complete;
	if(i)
	{
		IIC_WriteByte(addr);
		if(IIC_WaitACK())
		{
			IIC_WriteByte(Wdata);///
			if(IIC_WaitACK())
			{
				IIC_Stop();
			//	IIC_Delay(6000);//delay 5ms min
				return 1;
			}
		}
	}
	IIC_Stop();
	return 0;
}
uint8_t IIC_ReadOneByte(uint8_t DEV_Addr,uint16_t addr,uint8_t* pRdata)
{
	int i=-10000;
	do
	{
		IIC_Start();
		IIC_WriteByte(IIC_WRITE|DEV_Addr);
		if(IIC_WaitACK())
			break;
	}while(i++);//wait the write operation to complete;
	if(i)
	{
		IIC_WriteByte(addr);
		if(IIC_WaitACK())
		{
			IIC_Start();
			IIC_WriteByte(IIC_READ|DEV_Addr);
			if(IIC_WaitACK())
			{
				* pRdata = IIC_ReadByte();
				IIC_NACK();
				IIC_Stop();
				return 1;
			}
		}
	}
	IIC_Stop();
	return 0;
}
