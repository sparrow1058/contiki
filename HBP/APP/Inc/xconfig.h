#ifndef __XCONFIG_H
#define __XCONFIG_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
extern volatile uint16_t g_critical_cnt; 
#define CRITICAL_ENTER()      {__disable_irq();g_critical_cnt++;}
#define CRITICAL_EXIT()       {if(g_critical_cnt)(g_critical_cnt--);if(0==g_critical_cnt){__enable_irq();}}
#define CRITICAL_EXIT_FORC()  {g_critical_cnt=0;__enable_irq();}  //只有非OS才能使用
	
#endif
