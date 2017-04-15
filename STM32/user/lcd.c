#include "delay.h"
#include "stm32f10x.h"

#include "lcd.h"
#include "spi.h"
#define USE_HORIZONTAL  1
 
//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

//画笔颜色,背景颜色
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	
void LCD_WR_REG(u16 data)
{
  LCD_CS_CLR;
  LCD_RS_CLR;
  SPI_WriteByte(SPI2,data);
  LCD_CS_SET;
}
void LCD_WR_DATA(u8 data)
{
  LCD_CS_CLR;
  LCD_RS_SET;
  SPI_WriteByte(SPI2,data);
  LCD_CS_SET;
}
void LCD_WR_DATA_16Bit(u16 data)
{
  LCD_CS_CLR;
  LCD_RS_SET;
  SPI_WriteByte(SPI2,data>>8);
  SPI_WriteByte(SPI2,data);
  LCD_CS_SET;
}
void LCD_WriteReg(u16 LCD_Reg,u16 LCD_RegValue) 
{
  LCD_WR_REG(LCD_Reg);
  LCD_WR_DATA(LCD_RegValue);
}
void LCD_WriteRAM_Prepare(void)
{
//  LCD_WR_REG(lcddev.wramcmd);
  LCD_WR_REG(0x2C);     //write gram command
}
void LCD_DrawPoint(u16 x,u16 y) //write a white point
{
  LCD_SetCursor(x,y);
  LCD_WR_DATA_16Bit(POINT_COLOR);
}
void LCD_Clear(u16 Color)
{
  u16 i,j;
  LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);
  for(i=0;i<lcddev.width;i++)
  {
    for(j=0;j<lcddev.height;j++)
      LCD_WR_DATA_16Bit(Color);
  }
}
/*************************************************
函数名：LCD_SetWindows
功能：设置lcd显示窗口，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{
#if USE_HORIZONTAL==1	//使用横屏
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(xStar>>8);
	LCD_WR_DATA(0x00FF&xStar+3);		
	LCD_WR_DATA(xEnd>>8);
	LCD_WR_DATA(0x00FF&xEnd+3);

	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(yStar>>8);
	LCD_WR_DATA(0x00FF&yStar+2);		
	LCD_WR_DATA(yEnd>>8);
	LCD_WR_DATA(0x00FF&yEnd+2);	

#else
	
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(xStar>>8);
	LCD_WR_DATA(0x00FF&xStar+2);		
	LCD_WR_DATA(xEnd>>8);
	LCD_WR_DATA(0x00FF&xEnd+2);

	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(yStar>>8);
	LCD_WR_DATA(0x00FF&yStar+3);		
	LCD_WR_DATA(yEnd>>8);
	LCD_WR_DATA(0x00FF&yEnd+3);	
#endif

	LCD_WriteRAM_Prepare();	//开始写入GRAM				
}   
void LCD_SetCursor(u16 Xpos,u16 Ypos)
{
  LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);
}
  
void LCD_RESET(void)
{
  delay_ms(100);
  //LCD_RST();
  delay_ms(100);
}
  
void LCD_REG_Init()
{
  delay_ms(120);
  LCD_WR_REG(0xB1);
  LCD_WR_DATA(0x01); 
  LCD_WR_DATA(0x2C); 
  LCD_WR_DATA(0x2D); 

  LCD_WR_REG(0xB2); 
  LCD_WR_DATA(0x01); 
  LCD_WR_DATA(0x2C); 
  LCD_WR_DATA(0x2D); 

  LCD_WR_REG(0xB3); 
  LCD_WR_DATA(0x01); 
  LCD_WR_DATA(0x2C); 
  LCD_WR_DATA(0x2D); 
  LCD_WR_DATA(0x01); 
  LCD_WR_DATA(0x2C); 
  LCD_WR_DATA(0x2D); 
  
  LCD_WR_REG(0xB4); //Column inversion 
  LCD_WR_DATA(0x07); 
  
  //ST7735R Power Sequence
  LCD_WR_REG(0xC0); 
  LCD_WR_DATA(0xA2); 
  LCD_WR_DATA(0x02); 
  LCD_WR_DATA(0x84); 
  LCD_WR_REG(0xC1); 
  LCD_WR_DATA(0xC5); 

  LCD_WR_REG(0xC2); 
  LCD_WR_DATA(0x0A); 
  LCD_WR_DATA(0x00); 

  LCD_WR_REG(0xC3); 
  LCD_WR_DATA(0x8A); 
  LCD_WR_DATA(0x2A); 
  LCD_WR_REG(0xC4); 
  LCD_WR_DATA(0x8A); 
  LCD_WR_DATA(0xEE); 
  
  LCD_WR_REG(0xC5); //VCOM 
  LCD_WR_DATA(0x0E); 
  
  LCD_WR_REG(0x36); //MX, MY, RGB mode 				 
  LCD_WR_DATA(0xC8); 
  
  //ST7735R Gamma Sequence
  LCD_WR_REG(0xe0); 
  LCD_WR_DATA(0x0f); 
  LCD_WR_DATA(0x1a); 
  LCD_WR_DATA(0x0f); 
  LCD_WR_DATA(0x18); 
  LCD_WR_DATA(0x2f); 
  LCD_WR_DATA(0x28); 
  LCD_WR_DATA(0x20); 
  LCD_WR_DATA(0x22); 
  LCD_WR_DATA(0x1f); 
  LCD_WR_DATA(0x1b); 
  LCD_WR_DATA(0x23); 
  LCD_WR_DATA(0x37); 
  LCD_WR_DATA(0x00); 	
  LCD_WR_DATA(0x07); 
  LCD_WR_DATA(0x02); 
  LCD_WR_DATA(0x10); 

  LCD_WR_REG(0xe1); 
  LCD_WR_DATA(0x0f); 
  LCD_WR_DATA(0x1b); 
  LCD_WR_DATA(0x0f); 
  LCD_WR_DATA(0x17); 
  LCD_WR_DATA(0x33); 
  LCD_WR_DATA(0x2c); 
  LCD_WR_DATA(0x29); 
  LCD_WR_DATA(0x2e); 
  LCD_WR_DATA(0x30); 
  LCD_WR_DATA(0x30); 
  LCD_WR_DATA(0x39); 
  LCD_WR_DATA(0x3f); 
  LCD_WR_DATA(0x00); 
  LCD_WR_DATA(0x07); 
  LCD_WR_DATA(0x03); 
  LCD_WR_DATA(0x10);  
  
  LCD_WR_REG(0x2a);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x7f);

  LCD_WR_REG(0x2b);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x00);
  LCD_WR_DATA(0x9f);
  
  LCD_WR_REG(0xF0); //Enable test command  
  LCD_WR_DATA(0x01); 
  LCD_WR_REG(0xF6); //Disable ram power save mode 
  LCD_WR_DATA(0x00); 
  
  LCD_WR_REG(0x3A); //65k mode 
  LCD_WR_DATA(0x05); 	
  LCD_WR_REG(0x29);//Display on	
}
//设置LCD参数
//方便进行横竖屏模式切换
void LCD_SetParam(void)
{
  lcddev.wramcmd=0x2C;
#if USE_HORIZONTAL==1
  lcddev.dir=1;
  lcddev.width=128;
  lcddev.height=129;
  lcddev.setxcmd=0x2A;
  lcddev.setycmd=0x2B;
  LCD_WriteReg(0x36,0xA8);
#else//竖屏
  lcddev.dir=0;//竖屏				 	 		
  lcddev.width=128;
  lcddev.height=128;
  lcddev.setxcmd=0x2A;
  lcddev.setycmd=0x2B;	
  LCD_WriteReg(0x36,0xC8);
#endif
  
}
void LCD_Init(void)
{
  SPI2_Init();
  LCD_RESET();
  LCD_REG_Init();
  LCD_SetParam();
  
}

int main(void)
{
	delay_init(72);	     //延时初始化
	LCD_Init();	   //液晶屏初始化
	LCD_Clear(BLACK); //清屏

	POINT_COLOR=WHITE; 
	
	LCD_DrawRectangle(0,0,128-1,128-1);	//画矩形 

	Show_Str(32,5,BLUE,YELLOW,"系统监控",16,0);
	
	Show_Str(5,25,RED,YELLOW,"温度     ℃",24,1);
	
	LCD_ShowNum2412(5+48,25,RED,YELLOW,":32",24,1);

	Show_Str(5,50,YELLOW,YELLOW,"湿度     ％",24,1);
	LCD_ShowNum2412(5+48,50,YELLOW,YELLOW,":20",24,1);

	Show_Str(5,75,WHITE,YELLOW,"电压      Ｖ",24,1);
	LCD_ShowNum2412(5+48,75,WHITE,YELLOW,":3.2",24,1);

	Show_Str(5,100,GREEN,YELLOW,"电流      Ａ",24,1);
	LCD_ShowNum2412(5+48,100,GREEN,YELLOW,":0.2",24,1);	  
  
}