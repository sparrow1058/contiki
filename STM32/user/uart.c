//uart.c
#include "uart.h"
#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"
void dbg_setup_uart()
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  //enable GPIOA clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);
  //PA9 tx1 config
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  
  //PA10 RX1 config
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  
  //USART config
  USART_InitStructure.USART_BaudRate=9600;
  USART_InitStructure.USART_WordLength=USART_WordLength_8b;
  USART_InitStructure.USART_StopBits=USART_StopBits_1;
  USART_InitStructure.USART_Parity=USART_StopBits_1;
  USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
  USART_Init(USART1,&USART_InitStructure);
  //enable USART1
  USART_Cmd(USART1,ENABLE);
}
int fputc(int ch,FILE *f)
{
  USART_SendData(USART1,(uint8_t)ch);
  while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
  return ch;
}