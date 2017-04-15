#ifndef __SPI_H__
#define __SPI_H__
void SPI2_Init();
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet);
u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 TXData);



#endif