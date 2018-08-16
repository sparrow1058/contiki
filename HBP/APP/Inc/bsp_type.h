#ifndef __BSP_TYPE_H
#define __BSP_TYPE_H
#include "stdint.h"
#include <stdbool.h>
#include <stddef.h>

typedef void    (*pvFunDummy)(void);
//输入整行,输出逻辑
typedef void    (*pvFunVoid) (void);
typedef void    (*pvFunBool) (bool     bVal);
typedef void    (*pvFunChar) (uint8_t  cVal);
typedef void    (*pvFunShort)(uint16_t sVal);
typedef void    (*pvFunWord) (uint32_t wVal);

//输入整行,输出逻辑
typedef bool    (*pbFunVoid) (void);
typedef bool    (*pbFunBool) (bool     bVal);
typedef bool    (*pbFunChar) (uint8_t  cVal);
typedef bool    (*pbFunShort)(uint16_t sVal);
typedef bool    (*pbFunWord) (uint32_t wVal);

//输入整形指针,输出逻辑
typedef bool    (*pbFun_pVoid) (void * pVoid);
typedef bool    (*pbFun_pChar) (uint8_t  * pStr);
typedef bool    (*pbFun_pShort)(uint16_t * pShor);
typedef bool    (*pbFun_pWord) (uint32_t * pWord);

//输入数据帧,输出逻辑
typedef bool    (*pbFun_Buffx)(void * pcBuff, uint16_t len );
typedef bool    (*pbFun_Bytex)(uint8_t * pcByte, uint16_t len );
typedef void     (*pvFun_ioSet)(uint8_t id,uint8_t val);
typedef uint8_t		(*pvFun_ioGet)(uint8_t id);

// For i2C struct
typedef uint16_t		(*pvFun_I2CRead) (uint8_t reg,uint8_t *buff);
typedef bool		(*pbFun_I2CWrite)	(uint8_t reg,uint8_t *buff,uint8_t len);

// For ADC struct
typedef  bool	(*pbFun_ADCCtrl) (bool ctrl);

typedef struct TTYx_HANDLE_STRUCT
{
	const char *const name;		//handle name
	const uint16_t	rxSize;
	const uint16_t	txSize;
	
	//step1: callback function
	const pvFunWord		init;
	const pbFun_Bytex	api_TxdFrame;
	const pbFunChar		api_TxdByte;
	
	//setp2 receive callback function
	pbFun_Bytex		inj_RcvFrame;
	pvFunDummy		inj_TxdReady;
	//setp 3:
	struct TTYx_HANDLE_STRUCT *pvNext;
}TTYx_HANDLE;
typedef struct IO_HANDLE_STRUCT 
{
    const char  * const name;   //驱动器名
    const uint8_t       count;  //LED数量
    
    //------------------------------------------------------
    const pvFunVoid  init;   //注入结构体.
    const pvFun_ioSet  set;    //写配置
    const pvFun_ioGet  get;    //获得配置
    //------------------------------------------------------
    //step3: 扩展函数
    const pvFunVoid     reboot;     //重启MCU

}IO_HANDLE;
typedef struct I2C_HANDLE_STRUCT
{
	const char * const name;
	const uint8_t 	id;			//witch i2c used
	//const pv
	const pvFunVoid init;
	const pvFun_I2CRead i2cRead;
	const pbFun_I2CWrite i2cWrite;
	
}I2C_HANDLE;
typedef struct ADC_HANDLE_STRUCT
{
	const char * const name;			//ADC name	
	const uint8_t	id;							//which adc channel used
	const uint8_t status;					//ADC Convert Status	
	const pvFunVoid init;					//ADC Init
	const pbFun_ADCCtrl	ctrl; 		//enable or disable ADC
}ADC_HANDLE;
	
#endif
