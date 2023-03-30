#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "usart1.h"	
#include "led.h"
#include "beep.h"
#include "key.h"
#include "timer.h"
#include "myiic.h"
#include "24cxx.h"
#include "dma.h"
#include "string.h"
#include "function.h"
#include "rtc.h"
#include <stdio.h>
#include <time.h>
#include "flash.h"

//描述：消毒灯Bootloader源代码
//作者：pngao
//日期：2021-8-19
//版本：v1.4

/******************************??????????*********************************
|0x8000000  |
|           |
|           |
|           |
******************************??????????*********************************/
//TOTAL 256K
//Bootloader        : 0x08000000 --- 0x08008000   0x08000  Total Size: 32K
//EEPROM Emulation  : 0x08008000 --- 0x08010000   0x08000  Total Size: 32K 
//APP1              : 0x08010000 --- 0x08020000   0x10000  Total Size: 64K 
//UPGRADE APP       : 0x08020000 --- 0x08030000   0x10000  Total Size: 64K 
//FLASH             : 0x08030000 --- 0x08040000   0x10000  Total Size: 64K 
int g_upgrade_status = 0;				//upgrade status 
uint8_t packet_received = 0;  //pcaket received flag , set to 1 when new packet arrived
uint16_t seq_num = 0;   		//packet sequence number
CONFIG_UPGRADE_CONFIG conf;
typedef void (*APP_FUNC)();  

/*
__asm void MSR_MSP(uint32_t addr)
{
    MSR MSP, r0
    BX r14;
}
*/
void iap_load_app(uint32_t app_addr)
{
    APP_FUNC jump2app;
    
    printf("\r\nFirst word : 0x%X\r\n",(*(uint32_t*)app_addr));

    if(((*(uint32_t *)app_addr)&0x2FFF8000) == 0x20000000)
    {
						TIM_Cmd(TIM3,DISABLE);  //TIM_Cmd(TIM4,DISABLE);
		        printf("IAP load APP!!!\r\n");
   
						MSR_MSP(app_addr);
			
           __disable_irq();
			     //__set_PRIMASK(1);
					 //NVIC_DisableIRQ(SysTick_IRQn);
   
           jump2app = (void (*)())*(__IO uint32_t*) (app_addr + 4);   
           __set_MSP(*(__IO uint32_t*) app_addr);  
           jump2app();	 
    }
    else
    {
        printf("APP Not Found! 0x%X \r\n",((*(uint32_t *)app_addr)&0x2FFFE000));
    }
}

void GPIO_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
		
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOC时钟
	
	//GPIOB初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOB,GPIO_Pin_2);
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_3| GPIO_Pin_4);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
		
  //GPIOC初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_9| GPIO_Pin_10 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_9 | GPIO_Pin_13);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

//主函数
int main(void)
{ 
//	CONFIG_UPGRADE_CONFIG conf;
	uint32_t fromaddr,toaddr;
	uint32_t upgrade_timeout;
	uint16_t new_version, old_version;
	int i = 0;
	char buffer[1024] ={0,};
	int g_config_upgrade_block = 0; 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);		//延时初始化 
	GPIO_init();
	uart_init(115200);	//串口初始化波特率为115200
  uart1_init(230400);
	printf("\r\nEnter Bootloader\r\n");
	
	TIM3_Int_Init(1000-1,840-1);
 
  MYDMA_Config(DMA2_Stream5,DMA_Channel_4,(u32)&USART1->DR,(u32)SendBuff,SEND_BUF_SIZE);//DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
  MYDMA_Enable(DMA2_Stream5,SEND_BUF_SIZE);	//开启一次DMA传输

	STMFLASH_Read((uint32_t *)&conf, sizeof(conf), ConfigFlashAddress);
	printf(" <------Software Version : %s-------> \r\n",BOOT_VERSION);

#if 0
  conf.block_size = FOTA_FILE_SIZE;//g_config_upgrade_block;
	conf.upgrade_flag = 0x55AA;
//	conf.len = g_config_upgrade_block;
#endif

	new_version = (conf.New_Vesion[0] << 8) | conf.New_Vesion[1];
	old_version = (conf.Old_Vesion[0] << 8) | conf.Old_Vesion[1];
	printf("old_version = %x  new_version = %x \r\n", old_version, new_version);
	
	if((0x55AA == conf.upgrade_flag) && (new_version > old_version) &&(old_version > 0))
	{
		printf("should upgrade !\r\n");
		printf("New FW size = %d\r\n", conf.block_len);
	//MCU OTA MCU 返回
	//	TX_OTA_Replay_Packet(conf.FrameNum,0x00); //0x00 成功 0x01失败
	//
		conf.block_size = FOTA_FILE_SIZE;
		// get upgrade firmware from app and save to flash
		g_upgrade_status = 0;
		upgrade_timeout = 0;
		while((0 == g_upgrade_status) && (upgrade_timeout < 15000))
		{
			if(packet_received == 1)
			{
				//printf("Received packet!\r\n");
				upgrade_timeout = 0;
				RX_Packet_Decode();
//				MYDMA_Enable(DMA2_Stream5,SEND_BUF_SIZE);			//恢复DMA指针，等待下一次的接收
				packet_received = 0;
//				printf("New packet received\r\n");
			}
			else
				upgrade_timeout++;
			delay_ms(1);	
		}
		printf("CONFIG_UPGRADE_CONFIG, flag:%d , size :%d \r\n",conf.upgrade_flag , conf.block_size);
		
		//Update firmware from upgrade APP sector to APP sector
		if(1 == g_upgrade_status)
		{
			printf("Begin update app sector!\r\n");
					
			fromaddr = UpgradeAppAddress;
			toaddr = ApplicationAddress;		
			if((conf.block_len%conf.block_size) == 0)
				g_config_upgrade_block = conf.block_len/conf.block_size;
			else
				g_config_upgrade_block = conf.block_len/conf.block_size+1;
		  
			printf("g_config_upgrade_block = %d \r\n",g_config_upgrade_block);
			
			//data copy from upgrade app sector to app sector
			for (i = 0; i< g_config_upgrade_block  ; i ++)
			{
				STMFLASH_Read((uint32_t *)buffer,conf.block_size / 4, fromaddr);
				printf(" 0x%x 0x%x %d %d \r\n",fromaddr, toaddr, i, conf.block_size);
				STMFLASH_Write((uint32_t *)buffer,conf.block_size / 4 ,toaddr);
				fromaddr += conf.block_size;
				toaddr += conf.block_size;
			}
			
			printf("upgrade is completed!\r\n");
			seq_num++;
			//send ota finish packet to app
			TX_OTA_Finish_Packet(seq_num,0x00);
		}
		else
		{
			printf("Receive firmware error and upgrade failed !\r\n");
		}
#if 1		
		conf.upgrade_flag = 0xFFFFFFFF;
		conf.block_size = 0;
		STMFLASH_Write((uint32_t *)&conf,sizeof(conf),ConfigFlashAddress);
#endif
	}
	else
	{
			printf("need not upgrade !\r\n");
	}

  iap_load_app(ApplicationAddress);
 
	return  0;
}

