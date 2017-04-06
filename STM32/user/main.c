#include "stm32f10x.h"
#include "led.h"
void Delay(__IO u32 nCount);
int main(void)
{
  SystemInit();                //set the system clock
  LED_GPIO_Config();    //init led
  while(1)
  {
    LED1(ON);
    Delay(0x400000);
    LED1(OFF);
    Delay(0x400000);
  }
}
void Delay(__IO u32 nCount)
{
  for(;nCount!=0;nCount--);
}