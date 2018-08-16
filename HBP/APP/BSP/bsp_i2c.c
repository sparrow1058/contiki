#include "stm32f10x.h"
#include "bsp_type.h"
#define I2C

void I2Cx_Init(void)
{
	
	
}
uint16_t I2Cx_Read(uint8_t reg,uint8_t *buff)
{
	uint16_t len=0;
	
	return len;
}
bool  I2Cx_Write(uint8_t reg, uint8_t *buff,uint8_t len)
{
	
	return true;
}
I2C_HANDLE  i2c_handle=
{
	.name			= "i2c",
	.id				=	1,
	.init			=I2Cx_Init,
	.i2cRead	=I2Cx_Read,
	.i2cWrite	=I2Cx_Write,
};

