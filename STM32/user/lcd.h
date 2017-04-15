#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				//LCD ID
	u8  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	u16	 wramcmd;		//开始写gram指令
	u16  setxcmd;		//设置x坐标指令
	u16  setycmd;		//设置y坐标指令	 
}_lcd_dev;
extern _lcd_dev lcddev;	//管理LCD重要参数
extern u16 POINT_COLOR;
extern u16 BACK_COLOR;
#define LCD_CTRL        GPIOB   //define lcd data pin
#define LCD_RS          GPIO_Pin_1
#define LCD_CS          GPIO_Pin_12
#define LCD_SCL         GPIO_Pin_13
#define LCD_SDA         GPIO_Pin_15
//Define LCD set
#define LCD_CS_SET      LCD_CTRL->BSRR=LCD_CS
#define LCD_RS_SET      LCD_CTRL->BSRR=LCD_RS
#define LCD_SDA_SET     LCD_CTRL->BSRR=LCD_SDA
#define LCD_SCL_SET     LCD_CTRL->BSRR=LCD_SCL
// #define LCD_RST_SET     LCD_CTRL->BSRR=LCD_RST
// #define LCD_LED_SET     LCD_CTRL->BSRR=LCD_LED
//define LCD clear
#define	LCD_CS_CLR  	LCD_CTRL->BRR=LCD_CS    
#define	LCD_RS_CLR  	LCD_CTRL->BRR=LCD_RS    
#define	LCD_SDA_CLR  	LCD_CTRL->BRR=LCD_SDA    
#define	LCD_SCL_CLR  	LCD_CTRL->BRR=LCD_SCL    
//#define	LCD_RST_CLR  	LCD_CTRL->BRR=LCD_RST    
//#define	LCD_LED_CLR  	LCD_CTRL->BRR=LCD_LED 
// the front color type
#define WHITE   0xFFFF
#define BLACK   0x0000
#define BLUE    0x001F
#define BRED    0xF81F
#define GRED    0xFFE0
#define GBLUE   0x07FF
#define RED     0xF800
#define MAGENTA 0xF81F
#define GREEN   0x07E0
#define CYAN    0x7FFF
#define YELLOW  0xFFE0
#define BROWN   0xBC40
#define BRRED   0xFC07
#define GRAY    0x8430

//GUI color
#define DARKBLUE        0x01CF
#define LIGHTBLUE       0x7D7C
#define GRAYBLUE        0x5458

#define LIGHTGREEN      0x841F
#define LGRAY           0xEF5B
#define LGRAYBLUE       0xA651
#define LBBLUE          0x2B12
//some lcd function
void LCD_Init(void);
//void LCD_DisplayOn(void);
//void LCD_DisplayOff(void);
void LCD_WR_REG(u16 data);
void LCD_WR_DATA(u8 data);
void LCD_WR_DATA_16Bit(u16 data);
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue);
void LCD_WriteRAM_Prepare(void);
void LCD_DrawPoint(u16 x,u16 y);
void LCD_RESET(void);
void LCD_Init(void);
void LCD_Clear(u16 Color);
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_SetParam(void);

#endif
