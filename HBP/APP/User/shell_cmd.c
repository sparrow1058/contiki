//at cmd shell
#include "xshell.h"
#include "stdio.h"
#include "bsp_type.h"
#define uchar unsigned char 

//THE AT CMDS
#define AT_HVER		0
#define AT_SVER		1	
#define AT_HYVER	2
#define AT_HP_ON	3
#define AT_HP_OFF	4
#define AT_HR		5	
#define AT_BP		6
#define AT_ECGON	7
#define AT_ECGOFF	8
#define AT_UPF		9

char * cmdStrList[]={
	"AT+HVER",
	"AT+SVER",
	"AT+HYVER",
	"AT+HP_ON",
	"AT+HP_OFF",
	"AT+HR",
	"AT+BP",
	"AT+ECGON",
	"AT+ECGOFF",
	"AT+UPF",
};
extern bool Shell_CMD_Service(void * pcBuff,uint16_t len);
const Cmd_Typedef_t CLI_CmdMsg=
{
	"cmd",
	"AT CMDS"
	"More information",
	&Shell_CMD_Service,
	0,
};
Cmd_List_t cmdList={&CLI_CmdMsg,NULL};


uint8_t getCmdType(uint8_t * buff)
{
	uint8_t * pbuf=buff;
	int i;
	uint8_t ret=0xFF;
	for(i=0;i<sizeof(cmdStrList);i++)
	{
		if(StrComp(pbuf,cmdStrList[i]))
		{
			ret= i;
			break;
		}
	}
	return ret;
}
bool Shell_CMD_Service(void * pcBuff,uint16_t len)
{
	uint8_t *ptRxd;
//	int 	i;
	uint8_t cmdType;
	//uint16_t retVal;
//	uint8_t buff[32];
	ptRxd=(uint8_t *)pcBuff;
	cmdType=getCmdType(ptRxd);
	printf("AT CMD =%d \n",cmdType);
	//switch(cmdType)
	
	return true;
}
