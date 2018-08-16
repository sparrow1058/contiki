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

 /***************  配置LED用到的I/O口 *******************/
void IO_Init(void)	
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE); // 使能PC端口时钟  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	//选择对应的引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PC端口
  GPIO_SetBits(GPIOC, GPIO_Pin_13 );	 // 关闭所有LED
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
