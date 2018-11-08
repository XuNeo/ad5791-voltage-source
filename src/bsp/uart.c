#include "uart.h"

static uint8_t buart_init_done = 0;
void uart_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//USART
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStructure);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	
	//USART_SWAPPinCmd(USART1,ENABLE);
	
	USART_Cmd(USART1,ENABLE);
	//interrupt configure
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_Init(&NVIC_InitStructure);//	USART_String("at\r\n");
}

void uart_char(char data)
{
  if(!buart_init_done){
    buart_init_done = 1;
    uart_init();
  }
	USART_SendData(USART1, (unsigned char) data);// USART1 ???? USART2 ?
	while (!(USART1->ISR & USART_FLAG_TXE));
}

#include "xshell.h"
xShell_st xShell;

void USART1_IRQHandler(void)
{
	volatile unsigned char c;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
    xShell_InputChar(&xShell,USART1->RDR);
		//CBuff_Write(&cbuff,USART1->RDR);
	}
}

