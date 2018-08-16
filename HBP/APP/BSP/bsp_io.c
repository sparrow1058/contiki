//bsp_io.c
#include "stm32f10x.h"
#include "bsp_type.h"

#define LED1(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_13);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_13)

#define LED2(a)	if (a)	\
					GPIO_SetBits(GPIOB,GPIO_Pin_14);\
					else		\
					GPIO_ResetBits(GPIOB,GPIO_Pin_14)

 /***************  ����LED�õ���I/O�� *******************/
void IO_Init(void)	
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE); // ʹ��PC�˿�ʱ��  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	//ѡ���Ӧ������
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  //��ʼ��PC�˿�
  GPIO_SetBits(GPIOC, GPIO_Pin_13 );	 // �ر�����LED
}
void gpio_set(uint8_t id,uint8_t val)
{
}
uint8_t gpio_get(uint8_t id)
{
	return 0;
}
void sysReboot(void)
{
	
}	
IO_HANDLE  io_handle=
{
	.name=	"io",
	.init=	IO_Init,
	.set=		gpio_set,
	.get=		gpio_get,
	.reboot	= sysReboot,
};
