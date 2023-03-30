#include "key.h"
#include "delay.h" 
#include "usart.h"
#include "function.h"
#include "uv_lamp.h" 
#include "timer.h"
#include "24cxx.h"
#include "usart1.h"
#include "rtc.h"
#include <stdio.h>

u16 sw3_count=0,sw4_count=0;
u8 TIM9_flag=0;
u8 BLE_Mesh_init_flag=0;
u8  UV1_open_flag=0,UV2_open_flag=0;//UV�ƿ���ʱ���־λ
u8 UV1_Life_write_EEPROM_value[2]={0},UV2_Life_write_EEPROM_value[2]={0};//UV�������洢д�����鶨��
u8 UV1_Life_read_EEPROM_value[2]={0},UV2_Life_read_EEPROM_value[2]={0};//UV�������洢��ȡ���鶨��
u8 BLE_close_flag=0;//����ģ�鸴λ���ܹرձ�־λ
u8 standby_continuous_flag=0,hidden_normal_flag=0;  
u8 hidden_open=0;//����ģʽ��־λ
//������ʼ������
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOB,GPIOCʱ��
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //SW3��Ӧ����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC8
	
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//SW4��Ӧ����PB7
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//����
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB7
 
} 

//����ɨ��
void KEY_Scan(void)
{
	u8 i=0;
//����SW4 5~10�����countineģʽ
	while(SW4==1)
		{
			sw4_count++;
			printf("sw4_count:%d\r\n",sw4_count);
			delay_ms(1000);
		}
		if(BLE_close_flag!=5)
		{
			BLE_PLAN_tab_flag=5;
			
			if(sw4_count>=15)//����15�����ϣ���λ����ģ��
			{
				sw4_count=0;
				TIM_Cmd(TIM4,DISABLE); //ʧ�ܶ�ʱ��4���ƻ�ģʽ�ر�
				if(hidden_open==0)
				{
				BLE_Mesh();
					while(i!=3)
					{
						GPIO_SetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
						GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
						GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
						delay_ms(300);
						GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
						GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
						GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
						delay_ms(300);
						i++;
					}
//				GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
//				GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
//				GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B	
				printf("time up\r\n");
			}
		}
	}
		if(sw4_count>=5&&sw4_count<=10)//����5��10�룬�ֶ��л�Continuous/standbyģʽ
			{
				sw4_count=0;
				
				if(hidden_open==0)
				{
					standby_continuous_flag=!standby_continuous_flag;
					if(standby_continuous_flag==1)//continuous_modeģʽ����
					{
						TIM_Cmd(TIM4,DISABLE); //ʧ�ܶ�ʱ��4���ƻ�ģʽ�ر�
						while(i!=2)
							{
								GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
								GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
								GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
								delay_ms(300);
								GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
								GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
								GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
								delay_ms(300);
								i++;
							}
							AT24CXX_Read(1,Duty_cycle_read_buff,2);//�Ӵ洢���ж���Duty_cycleֵ
						  str_duty_value=(Duty_cycle_read_buff[0]*256+Duty_cycle_read_buff[1])*1.5;//Duty-Cycleֵ���㵽��ʱ��ִ��
						  duty_count=0;//TIM3����������0
						  TIM_Cmd(TIM3,ENABLE);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
							GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
							GPIO_SetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
							printf("continuous\r\n");	
					}
					else//standby_mode����
					{
							TIM_Cmd(TIM3,DISABLE);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
							GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
					}
			 }
			}
	  if(sw4_count<=3&&sw4_count>=1)//�̰�1��3�룬�л�����/����ģʽ
			{
				sw4_count=0;
				hidden_normal_flag=!hidden_normal_flag;
				TIM_Cmd(TIM4,DISABLE); //ʧ�ܶ�ʱ��4���ƻ�ģʽ�ر�
				if(hidden_normal_flag==1)
				{
					hidden_open=1;//����ģʽ����
					while(i!=3)
						{
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
							GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
							delay_ms(300);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
							GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
							delay_ms(300);
							i++;
						}
				}
				else
				{
					hidden_open=0;//����ģʽ�ر�
					while(i!=3)
						{
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
							GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
							delay_ms(300);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
							GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
							delay_ms(300);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
							GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
							i++;
						}
				}
				printf("hidded\r\n");
			}

//	else 
//	{
//		//SW4:����ģ�����ø�λ
//			if(SW4==1)
//			{
//				delay_ms(200);
//				if(SW4==1)
//				{
//					
//				
//					while(SW4==1);
//				}
//			}
//	}
//	if(UV12_status==1)
//	{
//		u8 str_UV12_status[9]={0x55,0xAA,0x01,0x00,0x05,0x22,0x01};  //֡��ʽ
//		SN_count++;
//  str_UV12_status[3]=0x00;
//  str_UV12_status[4]=SN_count;
////		Usart1_Send(str_UV12_status,sizeof(str_UV12_status));
//	}
//���ڴ洢UV1��������EEPROM��־λ
//  if(UV1_EEPROM_FLAG==1)
//  {
//    UV1_EEPROM_FLAG=0;
//    UV1_Life_write_EEPROM_value[0]=UV1_life_min_count;
////    AT24CXX_Write(0,(u8*)UV1_Life_write_EEPROM_value,sizeof(UV1_Life_write_EEPROM_value));    //AT24LC64д������ 
//  }
//  
////���ڴ洢UV2��������EEPROM��־λ
//  if(UV2_EEPROM_FLAG==1)
//  {
//    UV2_EEPROM_FLAG=0;
//    UV2_Life_write_EEPROM_value[0]=UV2_life_min_count;
////    AT24CXX_Write(2,(u8*)UV2_Life_write_EEPROM_value,sizeof(UV2_Life_write_EEPROM_value));    //AT24LC64д������ 
//  }
//  
////��ȡUV1�Ƿ��ڴ�״̬  ���UV1�򿪣���ʱ��9ʹ�ܣ�UV1_open_flag��1
//  if(LAMP_ON_A==0)
//    {
//      if(TIM9_flag==0)
//      TIM_Cmd(TIM9,ENABLE); //ʹ�ܶ�ʱ��9
//      UV1_open_flag=1;
//      TIM9_flag=1;
////      printf("UV1 is open\r\n");
//      delay_ms(500);
//    }
//  else
//    {
//      UV1_open_flag=0;
////      printf("UV1 is close\r\n");
//      delay_ms(500);
//    }
////��ȡUV2�Ƿ��ڴ�״̬  ���UV2�򿪣���ʱ��9ʹ�ܣ�UV2_open_flag��1
//  if(LAMP_ON_B==0)
//    {
////      TIM_Cmd(TIM9,ENABLE); //ʹ�ܶ�ʱ��9
//      UV2_open_flag=1;
////      printf("UV2 is open\r\n");
//      delay_ms(500);
//    }
//  else
//    {
//      TIM_Cmd(TIM9,DISABLE); //ʧ�ܶ�ʱ��9
//      UV2_open_flag=0;
////      printf("UV2 is close\r\n");
//      delay_ms(500);
//    }
}




















