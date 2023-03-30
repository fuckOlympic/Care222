#include "timer.h"
//#include "led.h"
//#include "function.h"
//#include "key.h"
#include "usart.h"
#include "rtc.h"
#include "usart1.h"
#include  "flash.h"

//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
//���ã�UV�ƹ���ѭ�����ڶ�ʱ

extern int g_upgrade_status; 
extern uint8_t packet_received;
extern CONFIG_UPGRADE_CONFIG conf;
extern uint16_t seq_num;
uint16_t recv_data_packet_num = 0;
uint32_t last_addr = 0;

void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,DISABLE); //ʧ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

void print_hex(u8 *buf , int len)
{
	int i = 0;
	printf("RX:");
	for(i = 0;i< len; i ++)
	{
		printf(" %02X",buf[i]);
	}
	printf("\r\n");
}

void RX_Packet_Decode()
{
	u32 toAddr = UpgradeAppAddress;
	u32 pixAddr = 0; //,FrameNum;
	u16 sum_result;
	u8 result;
	int len = 0;
	print_hex(SendBuff,USART1_length_value);
	if(SendBuff[0]==0xaa&&SendBuff[1]==0x55)
	{
		seq_num++;
		//FrameNum = SendBuff[3]<<8|SendBuff[4];
		// 200MS ��ʱ
		if(0x08 == SendBuff[5]) //����
		{
			recv_data_packet_num++;
			//�̼����ݴ���	ģ�鷢��	0xAA55 0x01	0xXXXX	0x08	
			//0xXX+1	data[0]-data[3]:�̼�ƫ����;
			pixAddr = (SendBuff[7] <<24) | (SendBuff[8] <<16) | (SendBuff[9] <<8) |SendBuff[10];
			//data[4]-data[0xXX]:�̼����ݣ�<=128bytes��;	У���	0x01
//			if(recv_data_packet_num == 1)
//			{
//				//toAddr = UpgradeAppAddress;
//				print_hex(SendBuff,USART1_length_value);
//			}
			
			len = SendBuff[6] - 4;
			toAddr = UpgradeAppAddress + pixAddr - len;
			//parity check
			sum_result = Sum_Rem(SendBuff, SendBuff[6]+7);
//			printf("Received %d data packet, pixAddr = %x\r\n", recv_data_packet_num, pixAddr);
			if(sum_result == SendBuff[SendBuff[6]+7])
			{
				//Compare new adress with old address to prevent duplicate writing
				//Previously written data will be lost if  duplicate writing happens
				if(toAddr > last_addr)
				{
					if((len % 4) == 0)
						STMFLASH_Write((u32 *)(SendBuff + 11) ,len/4,toAddr);
					else
						STMFLASH_Write((u32 *)(SendBuff + 11) ,len/4+1,toAddr);
//					printf("toAddr = %08x, len = %d\r\n",toAddr,len);
				}
				else
				{
					printf("Received repeated packet!\r\n");
					print_hex(SendBuff,USART1_length_value);
				}
				result = 0;
				last_addr = toAddr;
			}
			else
			{
				result = 1;
				printf("Received packet has wrong data, Need retry!\r\n");
			}	
			TX_OTA_Transfer_Replay_Packet(seq_num,result);
			
			//End of firmware transfer?
			if((result == 0) &&(pixAddr  >= conf.block_len))
			{
				g_upgrade_status = 1;
				printf("Firmware download complete! Size = %x\r\n", (pixAddr + len + 1));
			}
		}
		else if(0x09 == SendBuff[5]) //�̼��ϱ��ظ�
		{
			printf("Received APP response for CMD09");
//			TX_OTA_Finish_Packet(FrameNum,0x00);			
		}	
		/***Added by pngao@20210705***/
		else if(0x07 == SendBuff[5]) //MCU OTA����֪ͨ�ظ�
		{
			TX_OTA_Replay_Packet(seq_num, 0x00);
		}

		/******/
	}
}
//��ʱ��3�жϷ����� ���ã�UV�ƹ���ѭ�����ڶ�ʱ
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
    	//RX_Packet_Decode();
			packet_received = 1 ;
			//printf("TIM3_IRQHandler \r\n");
			TIM_Cmd(TIM3,DISABLE); 
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}


//ͨ�ö�ʱ��9�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//���ã����ڻ�ȡUV��������ʱ
void TIM9_Int_Init(u16 arr,u16 psc)
{
        TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9,ENABLE);  ///ʹ��TIM9ʱ��       
        TIM_TimeBaseInitStructure.TIM_Period = arr;         //�Զ���װ��ֵ
        TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
        TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
        TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
        TIM_TimeBaseInit(TIM9,&TIM_TimeBaseInitStructure);//��ʼ��TIM9
        TIM_ITConfig(TIM9,TIM_IT_Update,ENABLE); //����ʱ��9�����ж�
        TIM_Cmd(TIM9,DISABLE); //ʧ�ܶ�ʱ��9

        NVIC_InitStructure.NVIC_IRQChannel=TIM1_BRK_TIM9_IRQn; //��ʱ��9�ж�
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00; //��ռ���ȼ�0
        NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
        NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
        NVIC_Init(&NVIC_InitStructure);       
}

//��ʱ��9�жϷ����� 
//���ã����ڻ�ȡUV��������ʱ
void TIM1_BRK_TIM9_IRQHandler(void)       
{
   if(TIM_GetITStatus(TIM9,TIM_IT_Update)==SET) //����ж�
    {
      
       printf("TIM9 is open!\r\n");
    }
    TIM_ClearITPendingBit(TIM9, TIM_IT_Update  );  //���TIM9�����жϱ�־   
}

//ͨ�ö�ʱ��4�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��4!
//���ã�RTCʱ�ӻ�ȡ
void TIM4_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///ʹ��TIM4ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//��ʼ��TIM4
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //����ʱ��4�����ж�
	TIM_Cmd(TIM4,ENABLE); //ʹ�ܶ�ʱ��4
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //��ʱ��4�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

//��ʱ��4�жϷ����� ���ã�UV�ƹ���ѭ�����ڶ�ʱ
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //����ж�
	{
		
		printf("TIM4 is open!\r\n");
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //����жϱ�־λ
}


