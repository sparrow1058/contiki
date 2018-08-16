/**************************************
 * �ļ���  ��main.c
 * ����    �������ϵ�LED1��˸��         
 * ʵ��ƽ̨��MINI STM32������ ����STM32F103C8T6
 * ��汾  ��ST3.0.0  																										  

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
	    
		SystemInit();	// ����ϵͳʱ��Ϊ72M 	
    io_handle.init();						//io config
		i2c_handle.init();
		shell_Init(115200);        // shell init
	  CLI_AddCmd(&cmdList);     //  add cmd list
	  printf("xc_shell Init successed!\r\n");
    
    //----------------------------------------------------------
    //step4: ��ѭ��
    while(1)
    {
        static uint32_t tick_cnt;
        
        //1> �źŵƣ��ź�1����5100ms���ź�2�����1s
        Delay(10);
				tick_cnt++;

        
        //2> ִ��Shell
        if(shell_rx_rdy){Task_ShellRxGuard(NULL);} 
    }

}

void Delay(__IO u32 nCount)
{
  for(; nCount != 0; nCount--);for(; nCount != 0; nCount--);for(; nCount != 0; nCount--);
} 

