#ifndef __I2C_EE_H
#define __I2C_EE_H
	#ifdef __I2C_EE_C
		#define EXTE_I2C
	#else
		#define EXTE_I2C extern
	#endif
#include "stm32f10x.h"

#define SCL_H	GPIOB->BSRR = GPIO_Pin_6
#define SCL_L	GPIOB->BRR  = GPIO_Pin_6

#define SDA_H	GPIOB->BSRR = GPIO_Pin_7
#define SDA_L	GPIOB->BRR  = GPIO_Pin_7

#define SDA_read	(GPIOB->IDR  & GPIO_Pin_7)

#define I2C_PageSize			64
#define EEPROM_ADDRESS			0xA0
#define LM75_ADDRESS			0x90

#define I2C_TIMEOUT				0x00200000
#define I2C_TM_ERR				0x0000FFFF
#define I2C_TM_OK				0x00000000

EXTE_I2C u8	I_time;
EXTE_I2C void IIC_Init(void);
EXTE_I2C u16  I2C_EE_ByteWrite(u8* pBuffer, u16 WriteAddr);
EXTE_I2C u16  I2C_EE_PageWrite(u8* pBuffer, u16 WriteAddr, u16 NumByteToWrite);
EXTE_I2C u16  I2C_EE_BufferWrite(u8* pBuffer, u16 WriteAddr, u16 NumByteToWrite);
EXTE_I2C u16  I2C_EE_BufferRead(u8* pBuffer, u16 ReadAddr, u16 NumByteToRead);
EXTE_I2C u16  I2C_EE_WaitEepromStandbyState(void);
EXTE_I2C u16  I2C_LM75_Read(void);

#endif
