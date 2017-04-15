#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>
#include <uart.h>
#include <clock.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <etimer.h>
#include <sys/autostart.h>
#include "led.h"
#include "uart.h"
unsigned int idle_count=0;
void led_init();
PROCESS(blink_process,"Blink");
AUTOSTART_PROCESSES(&blink_process);
PROCESS_THREAD(blink_process,ev,data)
{
  PROCESS_BEGIN();
  while(1)
  {
    static struct etimer et;
    etimer_set(&et,CLOCK_SECOND/10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    //Open LED
    //GPIO_ResetBits(GPIOC,GPIO_Pin_13);
    LED1(ON);
    printf("LED ON\r\n");
    etimer_set(&et,CLOCK_SECOND/10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    //Closs LED
    //GPIO_SetBits(GPIOC,GPIO_Pin_13);
    LED1(OFF);
    printf("LED OFF\r\n");
  }
  PROCESS_END();
 }
int main()
{
  dbg_setup_uart();
  led_init();
  printf("led init\n");
  clock_init();
  process_init();
  process_start(&etimer_process,NULL);
  autostart_start(autostart_processes);
  printf("Process running\r\n");
  while(1){
    do
    {}while(process_run()>0);
    idle_count++;
  }
  return 0;
}