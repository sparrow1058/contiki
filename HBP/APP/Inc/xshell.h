#ifndef __XSHELL_H
#define __XSHELL_H
#include <bsp_type.h>
#include <xconfig.h>
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
typedef bool (*pFunHook)(void *pcBuff,uint16_t len);
typedef struct
{
	const char * const 	pcCmdStr;
	const char * const 	pcHelpStr;
	const pFunHook		pxCmdHook;
	uint8_t				ucExpParam;
}Cmd_Typedef_t;
typedef struct xCOMMAND_INPUT_LIST
{
    const Cmd_Typedef_t         *thisStruct;    //当前指令
    struct xCOMMAND_INPUT_LIST  *pvNext;        //连接到下一个指令   
}Cmd_List_t;
extern uint8_t 	g_Login[5];
extern uint8_t	g_Patch[64];
extern volatile uint16_t debug_ch;
extern void CLI_AddCmd(Cmd_List_t *pvList);
extern void Task_ShellRxGuard(void *pvParameters);

extern volatile uint16_t shell_rx_rdy;		//0 idle, 
extern volatile uint8_t shell_rx_buff[];	//rx buffer

extern void shell_relink(void);
extern void shell_Init(uint32_t baud);
extern bool StrComp(void *buffer,void *StrCmd);
extern bool StrEqual(void *buffer,void *StrCmd);
//Send data
//比较字符串
extern bool StrComp (void * buffer,void * StrCmd);  //包含指令
extern bool StrEqual(void * buffer,void * StrCmd);  //完全相等
//发送数据
extern void shell_SendStr  (void * ptAsc);
extern void shell_SendHex  (void * ptHex,uint16_t size);
extern void shell_PrintfHex(void * buff,uint32_t sadd,uint16_t size);
//xc_data的数据校验
//extern uint16_t Get_CRC16_MODBUS(void * ptBuffer,uint32_t len);
//额外的
extern void SysDelayms(uint16_t ms);



#endif
