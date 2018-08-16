/**************************************
 * 文件名  ：main.c
 * 描述    ：板子上的LED1闪烁。         
 * 实验平台：MINI STM32开发板 基于STM32F103C8T6
 * 库版本  ：ST3.0.0  																										  

*********************************************************/	

#include "stm32f10x.h"
#include "xshell.h"
#include "user_val.h"
#define ON  0
#define OFF 1

volatile uint16_t g_critical_cnt; 
void Delay(__IO u32 nCount); 

int main(void)
{
	    
		SystemInit();	// 配置系统时钟为72M 	
    io_handle.init();						//io config
		i2c_handle.init();
		shell_Init(115200);        // shell init
	  CLI_AddCmd(&cmdList);     //  add cmd list
	  printf("xc_shell Init successed!\r\n");
    
    //----------------------------------------------------------
    //step4: 大循环
    while(1)
    {
        static uint32_t tick_cnt;
        
        //1> 信号灯，信号1闪间5100ms，信号2闪间隔1s
        Delay(10);
				tick_cnt++;

        
        //2> 执行Shell
        if(shell_rx_rdy){Task_ShellRxGuard(NULL);} 
    }

}

void Delay(__IO u32 nCount)
{
  for(; nCount != 0; nCount--);for(; nCount != 0; nCount--);for(; nCount != 0; nCount--);
} 

