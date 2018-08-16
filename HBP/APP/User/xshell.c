#include "stm32f10x.h"
#include "xshell.h"       //包含Shell接口文件
#include "xconfig.h"        //引用配置文件
#include "bsp_type.h"       //包含BSP底层
#include <stdint.h>
#include <stdbool.h>
#include "user_val.h"
/*---------------------* 
*       接收数据
*----------------------*/
#define SHELL_RX_MAX        (256+32)

#define SHELL_TTYx  apis_tty0;
/******************************************************************************
********************************* Shell.h定义 *********************************
******************************************************************************/
/*---------------------* 
*       信号量
*----------------------*/
volatile bool       sem_used_ymodem = false;    //是否启用Ymodem
volatile bool       sem_shell_echo  = false;    //shell是否使用本地回显
volatile bool       sig_recved = false;         //收到数据后闪烁

/*---------------------*
*       接收数据
*----------------------*/
volatile uint16_t   shell_rx_rdy    = 0;        //0:空闲,非零:忙
volatile uint8_t    shell_rx_buff[SHELL_RX_MAX];//接收缓冲

/*---------------------* 
*       字符变量
*----------------------*/
uint8_t     g_Login[5]   = "hbp";   //LOGIN
uint8_t     g_Patch[64]   = "~";
volatile uint16_t   debug_ch = 0;       //调试寄存器,0:全部关闭

/*---------------------* 
*       字符常量
*----------------------*/
/*调试
->  DBG: [ F E D C , B A 9 8 , 7 6 5 4 , 3 2 1 0 ]
    BIN:   0 0 0 0   0 0 0 0   0 0 0 0   0 0 0 0  
    Val: 0x7F42     Debug ON/OFF
*/
const   char dbgchar[] = "->  DBG: [ F E D C   B A 9 8   7 6 5 4   3 2 1 0 ]\r\n    BIN:   ";
const   char dbgack[]  = "->  Debug config succeed!\r\n";
/*---------------------* 
*     连接BSP底层
*----------------------*/
pbFun_Bytex         pbShellRxProcess    = NULL;         //接收处理
//LEDs_HANDLE * const ptLedHandle         = &apis_leds;   //LED接口
TTYx_HANDLE * const ptLocalCmdHandle    = &SHELL_TTYx;  //备用接口
TTYx_HANDLE *       phShell             = NULL;         //主接口

extern bool Shell_LEDx_Service(void * pcBuff, uint16_t len );

/*---------------------* 
*       CLI 结构
*----------------------*/
const Cmd_Typedef_t CLI_LedxMsg=
{
    //识别关键字
    .pcCmdStr   = "led",
    
    //帮助内容
	.pcHelpStr  =
	"[LED contorls]\r\n"
	" led help\t\t- help.\r\n"
	" led show\t\t- Read Led setting.\r\n"
	,
	//处理函数
	.pxCmdHook  = &Shell_LEDx_Service,        //见实体函数
	
	//附带数据
	.ucExpParam = 0,

};

/*---------------------* 
*       CLI 链表节
*----------------------*/
Cmd_List_t  CLI_List_Head   = { &CLI_LedxMsg,   NULL,}; //Shell指令的头
bool Shell_LEDx_Service(void * pcBuff, uint16_t len )
{
    uint8_t *   ptRxd;             //用于接收指令处理
//    int         led_id,msg_id,i;
    
    //1) 处理指令
    //--------------------------------------------------------------------------
    ptRxd = (uint8_t *)pcBuff;
    
   if(StrComp(ptRxd,"help\r\n"))            //指令帮助
    {
        shell_SendStr((void *)CLI_LedxMsg.pcHelpStr);
        return true;
    }
    else return false;
}

/*
/ 函数功能:FIFO满时的阻塞等待机制
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
#define CALC_CYCLE	28
#define LOOP_CYCLE      6
void SysDelayms(uint16_t ms)
{
    unsigned int cnt;
    //这段计算需要机器周期:CALC_CYCLE= 28
    cnt = ((SystemCoreClock/1000)*ms);  //获得时钟
    if(cnt<CALC_CYCLE)cnt = 0;
    else cnt = (cnt-CALC_CYCLE)/LOOP_CYCLE;
    
    //这个计算需要机器周期:LOOP_CYCLE= 6
     while(cnt--);       //while的延时周期是:LOPP_CYCLE
}


/******************************************************************************
/ 函数功能:注入帧接收处理方法, (中断中调用)
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
static bool cb_FrameRcv(uint8_t * ptByte,uint16_t size)
{
    static  uint16_t    shell_rx_index  = 0;
    int                 i;
    sig_recved = true;
    //---------------------------------------------------------------------
    //数据处理
    if(NULL != pbShellRxProcess) {
        if( pbShellRxProcess(ptByte,size) )return true;
    }
    debug_ch &= 0xFFFE;         //收到任何数据,关闭CPU显示通道
    //---------------------------------------------------------------------
    //Input下,需要进一步拼装数据包,符合要求才能交由处理
    //step2: Input模式下的数据接收
    if( shell_rx_rdy ) {shell_rx_index = 0;}
    else if( (shell_rx_index + size) <= SHELL_RX_MAX )
    {
        //本地回显: "\b"除外
        if( (sem_shell_echo) && ('\b'!=ptByte[0]) )phShell->api_TxdFrame(ptByte,size) ;
        
        //搬移数据
        for(i=0; i<size; i++)
            shell_rx_buff[shell_rx_index + i] = ptByte[i];
        shell_rx_index += size;
    }
    else {shell_rx_index = 0;}
    
    //step3: Input模式下数据处理
    if( (shell_rx_index >= 2) && ('\r' == shell_rx_buff[shell_rx_index-2]) && ('\n' == shell_rx_buff[shell_rx_index-1]) )  
    {
        //设置信号
        shell_rx_rdy = shell_rx_index;
        shell_rx_index = 0;
        //发送数据
    }
    else if( (shell_rx_index > 0) && ('\b' == shell_rx_buff[shell_rx_index-1]) )  //以\b结尾
    {
        if(shell_rx_index <2)   {shell_rx_index = 0;}
        else {
            shell_rx_index -= 2;
            if(sem_shell_echo) phShell->api_TxdFrame("\b \b",3);
        }
    }
    
    //step3: 返回值
    return true;
}

/******************************************************************************
/ 函数功能:注入帧发送处理方法
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
static void cb_TxdReady(void)
{

}

/******************************************************************************
/ 函数功能:printf支持函数
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
PUTCHAR_PROTOTYPE
{
    if(sem_used_ymodem)return ch;   //Ymode阻止printf
    while(false == phShell->api_TxdByte((uint8_t)ch) ){SysDelayms(1);}
    return ch;
}

/******************************************************************************
/ 函数功能:Shell初始化
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void shell_Init(uint32_t baud)
{
    //连接到BSP底层
    phShell = ptLocalCmdHandle;     //指向本地
  //  ptLedHandle->init(pCfg);
    phShell->init(baud);
   
    
    //注入数据处理
    phShell->inj_RcvFrame = &cb_FrameRcv;
    phShell->inj_TxdReady = &cb_TxdReady;
  #if SHELL_USE_YMODEM  
    Ymodem_Init();
  #endif
}

/******************************************************************************
/ 函数功能:字符串发送函数
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void shell_SendStr(void * ptAsc)
{                                   //中断方式
    //--------------------------- 中断方式收发数据 ----------------------------
    uint16_t        size=0;
    uint8_t const   *ptSrc;     //源数据只读不写
    
    //计算字符串的长度
    if(sem_used_ymodem)return;  //Ymode阻止shell输出
    ptSrc = (uint8_t const *)ptAsc;
    while(*ptSrc++){size++;}
    
    //判断字符串是否超过缓冲
    if(size > phShell->txSize)
    {
        ptSrc = (uint8_t const *)ptAsc;
        while(size--)
        {
            while(false == phShell->api_TxdByte(*ptSrc) ){SysDelayms(1);}
            ptSrc++;
        }
    }
    else
    {
        while(false == phShell->api_TxdFrame(ptAsc,size) ){SysDelayms(1);} 
           
    }
}

/******************************************************************************
/ 函数功能:发送Hex数据函数
/ 修改日期:none
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void shell_SendHex(void * ptHex,uint16_t size)
{
    //--------------------------- 中断方式收发数据 ----------------------------
    uint8_t const   *ptSrc;
    
    if(sem_used_ymodem)return;  //Ymode阻止shell输出

    //判断字符串是否超过缓冲
    if(size > phShell->txSize)
    {
        ptSrc = (uint8_t const *)ptHex;
        while(size--)
        {
            while(false == phShell->api_TxdByte(*ptSrc) ){SysDelayms(1);}
            ptSrc++;
        }
    }
    else
    {
        while(false == phShell->api_TxdFrame(ptHex,size) ){SysDelayms(1);}     
    }
}

/******************************************************************************
/ 函数功能:严格的字符串匹配(字符串是否相等)
/ 修改日期:2014/3/5 19:30:22
/ 输入参数:
/   @arg buffer 需要比较的字符串
/   @arg StrCmd 参考字符串
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void shell_PrintfHex(void * buff,uint32_t sadd,uint16_t size)
{
    uint16_t count,div,i,j;
    uint8_t * ptByte = (uint8_t *)buff;
    
    if(sem_used_ymodem)return;  //Ymode阻止shell输出
    if(0 == size)return;
        
    div   = size%16;    //最后一行的字节数
    count = (div==0)?size/16:(size/16 + 1); //要打印的行数
    
    for(i=0; i<count; i++)
    {
        printf("\r\n  [%06X]\t",sadd + i*16);
        if( (div!=0) && (i==(count-1)) )    //最后一行了吗? 不足16字节
        {
            for(j=0; j<div; j++)printf("%02X ",ptByte[i*16+j]);
        }
        else            //是完整行,足够16字节         
        {
            for(j=0; j<16; j++)printf("%02X ",ptByte[i*16+j]);  
        }    
    }
}
/******************************************************************************
/ 函数功能:格式化输入的字符串
/ 修改日期:2014/3/5 19:30:22
/ 输入参数:
/   [IN-OUT]    ptByte  作为数据源输入,格式化后为新的指针输出
/   [IN-OUT]    pslen   作为数据个数输入,格式化后为新的数据个数输出
/   [OUT]       cmdstr  指令字符串输出

/ 输出参数:none
/ 使用说明:如输入"FaT12 ABC\r\n",cmdStr内容为"fat12\0",pcByte内容为"ABC\r\n",tal=5
******************************************************************************/
static bool FormatCmdBuff(uint8_t * * ptBuff,uint16_t *pslen,char * cmdStr)
{ 
    uint8_t   * pcByte;
    uint16_t    tal,i,len;
    
    len = tal = * pslen;
    pcByte      = * ptBuff;
    for(i=0; (i<16)&&(i<len);i++ )
    {
        if( ( (*pcByte >='a') && (*pcByte <= 'z') ) ||      //是小写和数字
            ( (*pcByte >='0') && (*pcByte <= '9') ) )    
        {
            cmdStr[i] = (char)*pcByte++;
            if(tal)tal--;
        }
        else if( (*pcByte >='A') && (*pcByte <= 'Z') )      //是大写 
        {
            cmdStr[i] = (char)(*pcByte++)+0x20;             //变为小写
            if(tal)tal--;    
        }
        else 
        {
            cmdStr[i] = 0; 
            *pcByte++;
            if(tal)tal--;
            break;
        }
    }
    
    * ptBuff = pcByte;
    * pslen  = tal;
    if(tal)return true;
    else   return false;
}

/******************************************************************************
/ 函数功能:严格的字符串匹配(字符串是否相等)
/ 修改日期:2014/3/5 19:30:22
/ 输入参数:
/   @arg buffer 需要比较的字符串
/   @arg StrCmd 参考字符串
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
static void shell_PrintfGuide(void)
{

    shell_SendStr("\r\nAT:OK!\r\n");
}

/******************************************************************************
/ 函数功能:接受数据处理
/ 修改日期:2015/6/17 15:43:42
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void Task_ShellRxGuard(void * pvParameters)
{
    uint16_t        len,i;
    char            cmdStr[16+1];   //指令关键字字符串
    uint8_t       * argbuff;        //参数起始缓冲
    uint16_t        argsize;        //参数字节个数
    uint8_t         u_cmd_type;     //指令类型标记
    bool            b_cmd_result;   //处理结果标记
    Cmd_List_t    * pvCmdList;      //指令链表指针
    while(1)
    {
        int tmp_val;
        
        //----------------------------------------------------------------------
        //step1: 未等到信号继续等待
        if(0==shell_rx_rdy)break;

        //----------------------------------------------------------------------
        //step2: Ymodem处理接受的数据帧
      #if SHELL_USE_YMODEM
        //if( (sem_used_ymodem) && (shell_rx_rdy) ) {shell_rx_rdy=0;    //非RTOS下用
        if(sem_used_ymodem) {
            Ymodem_Service();       //启动Ymodem接收服务 
            
            shell_rx_rdy = 0;    
            if(false == sem_used_ymodem)shell_PrintfGuide();     
            break;  
        }
      #endif
        
        //----------------------------------------------------------------------
        //step3: 为无效指令直接跳过
        len = shell_rx_rdy;
        if( (len < 2) || ('\r' != shell_rx_buff[len-2]) || ('\n' != shell_rx_buff[len-1]))
        {
            //本流程结束
            shell_rx_rdy = 0;   
            break;
        }
        
        //----------------------------------------------------------------------
        //step4: 得到指令的头部,和参数起始地址,和参数字节数
        argbuff = (uint8_t *)shell_rx_buff;         //参数起始Byte[]
        argsize    = len;                           //参数字节数,包含\r\n
        FormatCmdBuff(&argbuff,&argsize,cmdStr);    //格式化
				printf("cmd=%s \n",cmdStr);
        //----------------------------------------------------------------------
        //step5: 判断指令类型
        if(2 == len)u_cmd_type = 0;     //空指令
        else if(cmdStr[0] == 0)         //无关键字继续等待帧接收信号
        {
            shell_rx_rdy = 0;
            break;   
        }
        else if( StrComp(cmdStr,"help") )
        {
            u_cmd_type = 1;     //帮助指令
        }
        else if( StrComp(cmdStr,"reboot") )
        {
            u_cmd_type = 0;     //不需要进一步处理 
            shell_SendStr("->MCU Reboot Execution!\r\n");
            SysDelayms(250);
//            ptLedHandle->reboot();  //重启MCU        
        }
        else if( StrComp(cmdStr,"debug") ) {
            if      ('+' == argbuff[0] ) {  //通道打开
                if( (1==sscanf((void*)(argbuff+1),"%x",&tmp_val)) && (tmp_val<16) ) {
                    debug_ch |= (1<<tmp_val); 
                    shell_SendHex((void*)dbgack,sizeof(dbgack)-1);   
                }
            }
            else if ('-' == argbuff[0] ) {  //通道关闭
                if( (1==sscanf((void*)(argbuff+1),"%x",&tmp_val)) && (tmp_val<16) ) {
                    debug_ch &= (uint16_t)~(1<<tmp_val);
                    shell_SendHex((void*)dbgack,sizeof(dbgack)-1);     
                }  
            }
            else if ('=' == argbuff[0] ) {  //直接赋值
                if( (1==sscanf((void*)(argbuff+1),"%x",&tmp_val)) && (tmp_val<65536) ) {
                    debug_ch = (uint16_t)tmp_val;
                    shell_SendHex((void*)dbgack,sizeof(dbgack)-1); 
                }
            }

            //显示通道配置值
            shell_SendHex((void*)dbgchar,sizeof(dbgchar)-1);
            for(i=16; i!=0; i--)
            {
                if(debug_ch & (1<<(i-1)) )shell_SendStr("1");
                else shell_SendStr("0");
                if( 1 == (i%4) )shell_SendStr("   ");
                else shell_SendStr(" ");
            }
            printf("\r\n    Val: 0x%04X",debug_ch);
            if(debug_ch) shell_SendStr("\t\tDebug ON\r\n");
            else shell_SendStr("\t\tDebug OFF\r\n");  
            
            //标记
            u_cmd_type = 0;     //不需要进一步处理   
        }else 
			u_cmd_type = 255;  //普通指令
        
        //----------------------------------------------------------------------
        //step6: 处理指令
        if(0 == u_cmd_type)     //是前导测试
        {
            b_cmd_result = true;   
        }
        else    //其他指令处理
        {
            b_cmd_result = false;       //处理结果标记
            pvCmdList = &CLI_List_Head; //得到表头指针
            while(1)                    //按链表查询指令
            {
                //判断指令列表是否为空,为空则退出循环
                if( pvCmdList->thisStruct == NULL)break;

                //按类型处理数据
                if(1 == u_cmd_type)     //处理帮助指令
                {
                    shell_SendStr((void *)pvCmdList->thisStruct->pcHelpStr);
                    b_cmd_result = true;    
                }else if(255 == u_cmd_type)
                {
                    if( StrComp(cmdStr,(void *)pvCmdList->thisStruct->pcCmdStr) &&
                       (pvCmdList->thisStruct->pxCmdHook != NULL)  )
                    {
                        //调用处理函数处理指令
                        b_cmd_result = pvCmdList->thisStruct->pxCmdHook((void *)argbuff,argsize);
                        if(b_cmd_result)break;      //处理成功直接退出,无需查询下一链表
                    }    
                }
                
                //指向下一个指令表
                if(pvCmdList->pvNext != NULL)pvCmdList = pvCmdList->pvNext;
                else break;
            }    
        }
        shell_rx_rdy = 0;       //处理完毕清除信号
        
        //----------------------------------------------------------------------
        //step7: 无效指令显示
        if(false == b_cmd_result)
        {
            int sum = argsize>=2?argsize-2:0;
                
            printf("\r\nAT: Cmd \"%s\" Error!\t[%d]=\"",cmdStr,sum);
            
            //格式化为可显示数据
            for(i=0; i<sum; i++)
            {
                if( (argbuff[i] < 32) || (argbuff[i] > 126) || (i>=32))
                {
                    argbuff[i++]='.';
                    argbuff[i++]='.';
                    argbuff[i++]='.';
                    break;         
                }
            }
            argbuff[i]='\0';
            
            shell_SendStr(argbuff);
            shell_SendStr("\"\r\n");    
        }
        
        //----------------------------------------------------------------------
        //step8: 发送前导
        shell_PrintfGuide();
    }
}

/******************************************************************************
/ 函数功能:向链表中添加帮助文件
/ 修改日期:2014/3/5 19:30:22
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void CLI_AddCmd(Cmd_List_t * pvList)
{
    Cmd_List_t   * pvCmdList;
    
    pvCmdList = &CLI_List_Head;  //得到表头指针
    while(1)    //按链表查询指令
    {
        if( pvCmdList->thisStruct == NULL)      //当前表空,直接填写
        {
            pvCmdList->thisStruct = pvList->thisStruct;
            pvCmdList->pvNext = NULL;
            break; 
        }
        else if(pvCmdList->pvNext == NULL)      //到了链表尾部码
        {
            pvCmdList->pvNext = pvList;         //将目标链表连接到本表的尾部
            break;
        }
        else pvCmdList = pvCmdList->pvNext;     //指向下一链表
    }
}

/******************************************************************************
/ 函数功能:普通的比较(是否包含字符串)
/ 修改日期:2014/3/5 19:30:22
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
bool StrComp(void * buffer,void * StrCmd)
{
    uint8_t i;
    uint8_t * ptBuf;
    uint8_t * ptCmd;
    
    ptBuf = (uint8_t *)buffer;
    ptCmd = (uint8_t *)StrCmd;
    for(i=0; i<255; i++)
    {
        if(ptCmd[i])
        {
            if(ptBuf[i] != ptCmd[i])return false;
        }
        else 
        {
            if(i)return true;
            else return false;    
        }
    }
    return false;
}

/******************************************************************************
/ 函数功能:严格的字符串匹配(字符串是否相等)
/ 修改日期:2014/3/5 19:30:22
/ 输入参数:
/   @arg buffer 需要比较的字符串
/   @arg StrCmd 参考字符串
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
bool StrEqual(void * buffer,void * StrCmd)
{
    volatile uint8_t i;
    uint8_t * ptBuf;
    uint8_t * ptCmd;
    
    ptBuf = (uint8_t *)buffer;
    ptCmd = (uint8_t *)StrCmd;
    for(i=0; i<255; i++)
    {
        if(ptCmd[i])
        {
            if(ptBuf[i] != ptCmd[i])return false;
        }
        else 
        {
            break;  
        }
    }
    
    //比较是否匹配
    if(ptBuf[i] != ptCmd[i])return false;   //比较字符串的尾字节('\0')是否匹配
    else if(0 == i)return false;
    else return true;
}

