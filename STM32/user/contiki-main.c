#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>
#include <uart.h>
#include <clock.h>
#include "contiki.h"
#include <sys/process.h>
#include <sys/procinit.h>
#include <etimer.h>
#include <sys/autostart.h>
#include "led.h"
#include "uart.h"
static process_event_t event_data_ready;
unsigned int idle_count=0;
PROCESS(print_hello_process,"hello");
PROCESS(blink_led_process,"led");
AUTOSTART_PROCESSES(&print_hello_process,&blink_led_process);  //set process auto start
PROCESS_THREAD(print_hello_process,ev,data)
{
  PROCESS_BEGIN();
  static struct etimer timer; 
  etimer_set(&timer,CLOCK_CONF_SECOND/10);
  printf("**Create hello process**\n");
  event_data_ready=process_alloc_event();       //alloc a event
  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_TIMER);  //wait for timer event
    printf("hello world \n");
    process_post(&blink_led_process,event_data_ready,NULL);
    etimer_reset(&timer);       //reset the timer
  }
  PROCESS_END();
}
PROCESS_THREAD(blink_led_process,ev,data)
{
  PROCESS_BEGIN();
  static unsigned char led_state=0;
  printf("Create led process");
  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev==event_data_ready);
    led_state=1-led_state;
    printf("led blink %d \n",led_state);
    LED1(led_state);
  }
  PROCESS_END();
}
int main()
{
  dbg_setup_uart();
  led_init();
  printf("contiki-main\n");
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

}