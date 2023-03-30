#ifndef __FLASH_H
#define __FLASH_H
 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//STM32�ڲ�FLASH��д ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

#include "stm32f4xx.h"
#include "stm32f4xx_flash.h"
#include "sys.h" 

#define BOOT_VERSION "V1.4"
#define FOTA_FILE_SIZE 512

typedef struct _CONFIG_UPGRADE_CONFIG{
	uint32_t upgrade_flag;
	uint32_t block_size;
	uint32_t block_len;
	uint32_t CRC32_value;
	uint32_t upgrade_status;
	uint32_t Old_Vesion[2];
	uint32_t New_Vesion[2];
} CONFIG_UPGRADE_CONFIG;

#define BootAppAddress		    0x08000000
#define ApplicationAddress    0x08010000 //0x08008000
#define UpgradeAppAddress     0x08020000
#define ConfigFlashAddress    0x08030000


//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
 

//FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//����0��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//����1��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//����2��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//����3��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//����4��ʼ��ַ, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//����5��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//����6��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//����7��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//����8��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//����9��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//����10��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//����11��ʼ��ַ,128 Kbytes  

u32 STMFLASH_ReadWord(u32 faddr);		  	//������  
void STMFLASH_Write(u32 *pBuffer,u32 NumToWrite,u32 WriteAddr);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 *pBuffer,u32 NumToRead,u32 ReadAddr);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
						   
#endif

