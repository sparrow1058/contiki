#include "bsp_type.h"
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stdio.h"
#include "xconfig.h"
#define TTY0_UART		USART1
#define TTY0_TX_PORT	GPIOA
#define TTY0_RX_PORT	GPIOA
#define TTY0_TX_PIN		GPIO_Pin_9
#define TTY0_RX_PIN		GPIO_Pin_10

//Config the IRQ
#define TTY0_IRQn		USART1_IRQn
#define TTY0_IRQHandler	USART1_IRQHandler

//Enable clock
#define TTY0_RCC_ENABLE()       {RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); \
                                RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);}

#define TTY0_MAX_TX_BYTE	(64)
#define TTY0_MAX_RX_BYTE	(64)
#define TTY0_TRC_PRIO		(13)		//the irq level


static volatile uint16_t    tx_size   = 0;
static volatile uint16_t    tx_rear   = 0;          //空递增
static volatile uint16_t    tx_front  = 0;          //满递增
static volatile uint8_t     tx_buff[TTY0_MAX_TX_BYTE+1];

/*---------------------* 
*       RX Shift
*----------------------*/
static volatile uint16_t    rx_index = 0;
static volatile uint8_t     rx_shift[TTY0_MAX_RX_BYTE+1];
void TTY0_Init(uint32_t baud);
bool TTY0_Send(uint8_t *buf,uint16_t len);
static bool TTY0_SendByte(uint8_t cbyte);

TTYx_HANDLE	apis_tty0=
{
	.name 	="tty0",
	.rxSize	=TTY0_MAX_RX_BYTE,
	.txSize	=TTY0_MAX_TX_BYTE,
	.init 	=&TTY0_Init,
	.api_TxdFrame	=&TTY0_Send,
	.api_TxdByte	=&TTY0_SendByte,
	.inj_RcvFrame	=NULL,
	.inj_TxdReady	=NULL,
	.pvNext			=NULL,
};
void TTY0_Init(uint32_t baud)
{
	GPIO_InitTypeDef	GPIO_InitStruct;
	USART_InitTypeDef	USART_InitStructure;
	NVIC_InitTypeDef	NVIC_UART_Cfg;
	TTY0_RCC_ENABLE();
    //----------------------------- 初始化GPIO口 -----------------------------
    // 配置TX引脚
	GPIO_InitStruct.GPIO_Pin    = TTY0_TX_PIN;
	GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_Init(TTY0_TX_PORT,     &GPIO_InitStruct);
	// 配置RX引脚
	GPIO_InitStruct.GPIO_Pin    = TTY0_RX_PIN;
	GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IPU;
	GPIO_Init(TTY0_RX_PORT,     &GPIO_InitStruct);
	
    //--------------------------- 先定义好数据结构 ---------------------------
    //定义好USART结构体
    USART_InitStructure.USART_BaudRate      = baud;
    USART_InitStructure.USART_WordLength    = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits      = USART_StopBits_1;
    USART_InitStructure.USART_Parity        = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode          = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_BaudRate      = USART_InitStructure.USART_BaudRate;    //防止编译报错
	//NVIC Config
    NVIC_UART_Cfg.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_UART_Cfg.NVIC_IRQChannelSubPriority = TTY0_TRC_PRIO;
    NVIC_UART_Cfg.NVIC_IRQChannel = TTY0_IRQn;
    NVIC_UART_Cfg.NVIC_IRQChannelCmd = ENABLE;
    
    //模式配置
    //--------------------------- 中断方式收发数据 ----------------------------
    USART_Cmd(TTY0_UART, DISABLE);                  //关闭UART
    USART_Init(TTY0_UART, &USART_InitStructure);    //初始化串口
    USART_ITConfig(TTY0_UART, USART_IT_RXNE, ENABLE);
    USART_ITConfig(TTY0_UART, USART_IT_IDLE, ENABLE);
    NVIC_Init(&NVIC_UART_Cfg);                      //配置好NVIC
    USART_Cmd(TTY0_UART, ENABLE);	
}

/******************************************************************************
/ 函数功能:写入一个字节到发送缓冲
/ 修改日期:none
/ 输入参数:none
/ 输出参数:写成功返回1,失败返回0
/ 使用说明:none
******************************************************************************/
static bool TTY0_SendByte(uint8_t cbyte)
{
    if( TTY0_MAX_TX_BYTE  > tx_size ) 
    {
        //将数据写入队列
        //__disable_irq(); 
        //portENTER_CRITICAL();
        CRITICAL_ENTER();
        tx_buff[tx_rear] = cbyte;
        tx_rear = (++tx_rear)%TTY0_MAX_TX_BYTE;
        tx_size++;
        //__enable_irq(); 
        //portEXIT_CRITICAL ();
        CRITICAL_EXIT();
        
        //串口数据发送
        if( !(TTY0_UART->CR1 & USART_CR1_TXEIE) )
        {
            TTY0_UART->CR1 |= USART_CR1_TXEIE;  
        }  
        
        return true;
    }
    else return false;
}

/******************************************************************************
/ 函数功能:将一个Buff写入队列中
/ 修改日期:none
/ 输入参数:none
/ 输出参数:写成功返回1,失败返回0
/ 使用说明:none
******************************************************************************/
static bool TxFIFO_GiveBuff(uint8_t * buf,uint16_t len)
{
    uint16_t i;
    
    if(TTY0_MAX_TX_BYTE  >= (len + tx_size) ) 
    {
        //将数据写入队列
        //__disable_irq(); 
        //portENTER_CRITICAL();
        CRITICAL_ENTER();
        for(i=0;i<len; i++)
        {
            tx_buff[tx_rear] = buf[i];
            tx_rear = (++tx_rear)%TTY0_MAX_TX_BYTE;
            tx_size++;
        }
        //__enable_irq(); 
        //portEXIT_CRITICAL ();
        CRITICAL_EXIT();
        
        return true;
    }
    else return false;
}

/******************************************************************************
/ 函数功能:从FIFO中读出1字节的数据(不能在中断中调用,会发生临界问题)
/ 修改日期:none
/ 输入参数:none
/ 输出参数:写成功返回1,失败返回0
/ 使用说明:none
******************************************************************************/
static bool TxFIFO_TakeByte(uint8_t * buf)
{
    if(tx_size > 0)
    {
        //__disable_irq();
        //portENTER_CRITICAL();
        //CRITICAL_ENTER();
        *buf = tx_buff[tx_front];
        tx_front = (tx_front+1)%TTY0_MAX_TX_BYTE;
        if(--tx_size == 0){tx_front = tx_rear = 0;}
        //__enable_irq();
        //portEXIT_CRITICAL ();
        //CRITICAL_EXIT();
        
        return true;
    }
    else return false;
}

/******************************************************************************
/ 函数功能:短数据(DMA)数据发送服务
/ 修改日期:2014/1/4 20:52:41
/ 输入参数:none
/ 输出参数:DMA忙返回false, 否者启动数据发送并返回True;
/ 使用说明:none
******************************************************************************/
bool TTY0_Send(uint8_t * buf,uint16_t len)
{
    if( (len==0) && (len > TTY0_MAX_TX_BYTE) )return false;
        
    if( TxFIFO_GiveBuff(buf,len) )
    {
        //启动中断发送
        if( !(TTY0_UART->CR1 & USART_CR1_TXEIE) )
        {
            TTY0_UART->CR1 |= USART_CR1_TXEIE;  
        } 
        return true;  
    }
    return false;
}

/******************************************************************************
/ 函数功能:中断收发数据 (实现帧接收和帧发送数据)
/ 修改日期:2014/1/4 20:52:41
/ 输入参数:none
/ 输出参数:DMA忙返回false, 否者启动数据发送并返回True;
/ 使用说明:none
******************************************************************************/
void TTY0_IRQHandler(void)
{
    uint8_t     rxd_reg,txd_reg;
    uint16_t    isr_reg;

    //--------------------------- 中断方式收发数据 ----------------------------
    isr_reg = TTY0_UART->SR;
    
    //RXNE 接收中断
    if( (TTY0_UART->CR1 & USART_CR1_RXNEIE) && (isr_reg & USART_SR_RXNE) )
    {
        rxd_reg = TTY0_UART->DR;
        if( rx_index < TTY0_MAX_RX_BYTE)
        {
            rx_shift[rx_index++] = rxd_reg;
        }
        else
        {
            rx_shift[rx_index++] = rxd_reg;
            rx_index = rx_index;
            //回调处理数据
            //----------------------------------------
            if( (rx_index) && (apis_tty0.inj_RcvFrame != NULL) )
                apis_tty0.inj_RcvFrame((uint8_t*)rx_shift,rx_index);
            //----------------------------------------
            rx_index = 0;
        }
    }
    
    //IDLE 空闲中断
    if( (TTY0_UART->CR1 & USART_CR1_IDLEIE) && (isr_reg & USART_SR_IDLE) )
    {
        TTY0_UART->SR;
        TTY0_UART->DR;
        
        //回调处理数据
        //----------------------------------------
        if( (rx_index) && (apis_tty0.inj_RcvFrame != NULL) )
            apis_tty0.inj_RcvFrame((uint8_t*)rx_shift,rx_index);
        //----------------------------------------
        rx_index = 0;
    }
    
    //TEX 发送完成中断   
    if( (TTY0_UART->CR1 & USART_CR1_TXEIE) && (isr_reg & USART_SR_TXE ) )
    {
        if(TxFIFO_TakeByte(&txd_reg) )
        {
            TTY0_UART->DR = txd_reg;            //发送数据    
        }
        else
        {
            TTY0_UART->CR1 &= ~USART_CR1_TXEIE; //关闭发送完毕中断
            
            //回调处理数据
            //----------------------------------------
            if(apis_tty0.inj_TxdReady != NULL)
                apis_tty0.inj_TxdReady();
            //----------------------------------------
        }         
    }
}
								
