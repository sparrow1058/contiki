#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "spi.h"
u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 TXData)
{
  unsigned int retry=0;
  while((SPI2->SR&1<<1)==0)
  {
    retry++;
    if(retry>2000) return 0;
  }
  SPI2->DR=TXData;
  retry=0;
  while((SPI2->SR&1<<0)==0)
  {
    retry++;
    if(retry>2000) return 0;
  }
  return SPI2->DR;
}
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
{
  SPIx->CR1&=0xFFC7;
  if(SpeedSet==1)
  {
    SPIx->CR1|=SPI_BaudRatePrescaler_2;
  }else
  {
    SPIx->CR1|=SPI_BaudRatePrescaler_2;
  }
  SPIx->CR1|=1<<6;
}

void SPI2_Init(void)
{
  SPI_InitTypeDef       SPI_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  //config SPI2 pin
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB,ENABLE);
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
  
  GPIO_ResetBits(GPIOB,GPIO_Pin_12);
  GPIO_ResetBits(GPIOB,GPIO_Pin_1);     //
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
  SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode=SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial=7;
  SPI_Init(SPI2,&SPI_InitStructure);
  
  //Enable SPI2
  SPI_Cmd(SPI2,ENABLE);
}