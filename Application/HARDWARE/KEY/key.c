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

/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
#include "eeprom.h"
/***Added end***/

//u16 sw3_count=0,sw4_count=0;
u8 TIM9_flag=0;
u8 BLErst_flag=0;//������λ��־λ
u8 togger_mode=0;//ģʽ�л�����������ס5���л�contuiuns mode/standby mode
u8 BLE_Mesh_init_flag=0;
u8  UV1_open_flag=0,UV2_open_flag=0;//UV�ƿ���ʱ���־λ
//u16 UV1_Life_write_EEPROM_value[2]={0},UV2_Life_write_EEPROM_value[2]={0};//UV�������洢д�����鶨��
u8 UV1_Life_read_EEPROM_value[2]={0},UV2_Life_read_EEPROM_value[2]={0};//UV�������洢��ȡ���鶨��
u8 BLE_close_flag=0;//����ģ�鸴λ���ܹرձ�־λ
u8 BLE_RST_Enable_flag=1;
u8 standby_continuous_flag=0,hidden_normal_flag=0;  
u8 hidden_open=0;//����ģʽ��־λ
u8 key_schedule_flag=0;//Scheduleģʽͨ��APP���ư�����־λ����key_schedule_flag=0ʱ��scheduleģʽ��;����scheduleģʽ�ر�
u8 manual_schedule_flag = 1;
u8 key_sw3_pressed = 0, key_sw3_released = 0;
time_t key_pressed_time, key_released_time;
u8 manual_key_off_flag = 0;
u8 manual_key_off_count = 0;

/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
extern uint8_t AT24CXX_exist_flag;
extern uint16_t uvlamp_fan_status;
/***Added end***/

//������ʼ������
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOB,GPIOCʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //SW3��Ӧ����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC8
	
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//SW4��Ӧ����PB7
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//����
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB7
#if 1 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource8);
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;	//?????
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif	
} 
#if 1
void EXTI9_5_IRQHandler(void)
{
	
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		printf("Enter EXTI9_5_IRQHandler\r\n");
	}
	if(SW3)
	{
		if((key_sw3_pressed == 0) && (key_sw3_released == 0))
		{
//			printf("SW3 pressed\r\n");
			key_sw3_pressed = 1;
			key_pressed_time = RTC_To_UTC();
		}
	}
	else
	{
		if(key_sw3_pressed == 1)
		{
//			printf("SW3 released\r\n");
			key_sw3_pressed = 0;			
			key_released_time = RTC_To_UTC();
			if(key_released_time == key_pressed_time)
			{
				key_sw3_released = 0;
			}
			else
			{
				key_sw3_released = 1;
			}
		}
	}

	EXTI_ClearITPendingBit(EXTI_Line8);
}
#endif

//����ɨ��
void KEY_Scan(void)
{
	u8 i=0;
#if 1
	if(key_sw3_pressed == 1)
	{
		if(schedule_flag==0)//schedule�ƻ�ģʽ��
		{
			key_schedule_flag=0;
//			TIM_Cmd(TIM4,ENABLE); //ʹ�ܶ�ʱ��4,��Ϊschedule��������Ҫ�õ�
//			printf("key_open schedule55\r\n");
		}
		else
		{
			key_schedule_flag=1;//schedule�ƻ�ģʽ�ر�
//		TIM_Cmd(TIM4,DISABLE); //ʧ�ܶ�ʱ��4,��ֹschedule
//			printf("key_close schedule55\r\n");
		}
	}
	if(key_sw3_released == 1)
	{
		int32_t pressed_sec;
		pressed_sec = key_released_time - key_pressed_time;
		printf("key pressed %d s \r\n", pressed_sec);
		if(pressed_sec >= 10)
		{
			if(dev_data.First_Connect_Flag == 1)
			{
	//			if(BLE_close_flag != 5)
				if(BLE_RST_Enable_flag == 1)
				{
					for(i = 0; i< 3; i++)
					{
						turn_all_off();
						delay_ms(500);
						turnON_RED();
						delay_ms(500);
					}
					//clear schedule data and set manual mode
					PLAN_Clear();
					if(manual_schedule_flag == 0)
					{
						Manual_Schedule_Mode_Set(1);
					}

					BLErst_flag=1;//������λ��1
					BLE_Mesh(1);
					default_set();
					pilot_lamp_mode = P1_BT_UNPAIR;
	//				TIM_Cmd(TIM4,DISABLE); //ʧ�ܶ�ʱ��4���ƻ�ģʽ�ر�
				}
				else
				{
					printf("Over 300s passed after system boot, can not reset BLE module\r\n");
				}
			}
			else
			{
				printf("Device has been already unpaired\r\n");
			}
		}
		else if((pressed_sec < 10) && (pressed_sec >= 5) && dev_data.Manual_Key_Enable_Flag)
		{
			//key_schedule_flag=0;
			LampFlash_GREEN(3);			
		}
		else if((pressed_sec >= 1) && (pressed_sec<=3)&&dev_data.Manual_Key_Enable_Flag )
		{
			if(dev_data.First_Connect_Flag == 1)
			{
				uint32_t event_time;
				printf("Manual_Turn_Off_By_ResetButton\r\n");
				
				default_set();
				if(manual_schedule_flag == 0)
				{
					Manual_Schedule_Mode_Set(1);
				}
				//Manual Turn off by reset button Event log
				log_event_data[current_event_cnt].event_type = Manual_Turn_Off_By_ResetButton;
				event_time = RTC_To_UTC();
				log_event_data[current_event_cnt].event_time = event_time;
				current_event_cnt++;
			}
		}
		key_sw3_released = 0;
		key_released_time = 0;
		key_pressed_time = 0;
	}
#else
	//����SW3 5~10�����countineģʽ	
	while(SW3==1)
	{
		sw3_count++;
		if(schedule_flag==0)//schedule�ƻ�ģʽ��
		{
			key_schedule_flag=0;
//			TIM_Cmd(TIM4,ENABLE); //ʹ�ܶ�ʱ��4,��Ϊschedule��������Ҫ�õ�
			printf("key_open schedule55\r\n");
		}
		else
		{
			key_schedule_flag=1;//schedule�ƻ�ģʽ�ر�
//		TIM_Cmd(TIM4,DISABLE); //ʧ�ܶ�ʱ��4,��ֹschedule
			printf("key_close schedule55\r\n");
		}
		printf("sw3_count:%d\r\n",sw3_count);
		delay_ms(1000);
	}
//		if(key_schedule_flag==1)
		
	if(BLE_close_flag!=5)
	{
//	BLE_PLAN_tab_flag=5;
//	printf("into key\r\n");
		if(sw3_count>=10)//����10�����ϣ���λ����ģ���MCU
		{
			sw3_count=0;
			BLErst_flag=1;//������λ��1
			BLE_Mesh(1);
			default_set();
			
//			TIM_Cmd(TIM4,DISABLE); //ʧ�ܶ�ʱ��4���ƻ�ģʽ�ر�
			if(hidden_open==0)
			{
//				BLE_Mesh(1);
				while(i!=3)
				{
					turn_all_off();
					delay_ms(300);
					turnON_RED();
					delay_ms(300);
					i++;
				}
				printf("time up\r\n");
			}
		}
	}

	if(sw3_count>=5&&sw3_count<10&&key_schedule_flag==1)//����5��10�룬��װ��Ա���ƾ��Ƿ��ڹ���
	{
		sw3_count=0;
		key_schedule_flag=0;
		while(i!=3)
		{
			turnON_GREEN();
			delay_ms(300); 
			turn_all_off();
			delay_ms(300);
			i++;
		}					
	}
	if(sw3_count<=3&&sw3_count>=1&&key_schedule_flag==1)//�̰�1��3�룬ǿ���˳�schedule״̬������standby״̬
	{
		sw3_count=0;
		default_set();	
	}

#endif
//���ڴ洢UV1��������EEPROM��־λ
  if(UV1_EEPROM_FLAG==1)
  {
		uint8_t UV1_Life[2];
    UV1_EEPROM_FLAG=0; 
		UV1_Life[1] = UV1_Life_Hours & 0xff;
		UV1_Life[0] = (UV1_Life_Hours >>8) & 0xff;
		UV1_Life_read_EEPROM_value[0] = UV1_Life[0];
		UV1_Life_read_EEPROM_value[1] = UV1_Life[1];
		/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(UV1_LIFETIME_ADDR, UV1_Life, 2);
		else
		/***Added end***/
 			AT24CXX_WriteBytes(UV1_LIFETIME_ADDR, UV1_Life, 2);    //AT24LC64д������ 
		printf("UV1 write successful:%d,%02X,%02X\r\n",UV1_Life_Hours,UV1_Life_read_EEPROM_value[0],UV1_Life_read_EEPROM_value[1]);
  }
  
//���ڴ洢UV2��������EEPROM��־λ
  if(UV2_EEPROM_FLAG==1)
  {
		uint8_t UV2_Life[2];
    UV2_EEPROM_FLAG=0;
		UV2_Life[1]= UV2_Life_Hours & 0xff;
		UV2_Life[0]= (UV2_Life_Hours >> 8) & 0xff;
		UV2_Life_read_EEPROM_value[0] = UV2_Life[0];
		UV2_Life_read_EEPROM_value[1] = UV2_Life[1];
		
		/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(UV2_LIFETIME_ADDR, UV2_Life, 2);
		else
		/***Added end***/	
			AT24CXX_WriteBytes(UV2_LIFETIME_ADDR, UV2_Life, 2);    //AT24LC64д������ 
		printf("UV2 write successful:%d,%02X,%02X\r\n",UV2_Life_Hours,UV2_Life_read_EEPROM_value[0],UV2_Life_read_EEPROM_value[1]);
  }
	
	if(Plan_running && (UV12_status == 1) && LAMP_ON_A && LAMP_ON_B)
	{
		if((uv_lamp_opened == 0)) // && (uvlamp_fan_status & (UV1_TURNON_FAILURE|UV2_TURNON_FAILURE) != 0x3) &&(uvlamp_fan_status & (FAN1_SPEED_ERROR|FAN2_SPEED_ERROR)))
		{
			if(single_double_lamp_flag == 1)
			{
				open_single_lamp();
			}
			else
			{
				printf("key_scan open lamp : ");
				open_double_lamp();
			}
		}
	}
	
	if(((uvlamp_fan_status&(TEMP_ERROR|FAN1_SPEED_ERROR|FAN2_SPEED_ERROR|UV1_TURNON_FAILURE|UV2_TURNON_FAILURE)) != 0)||(UV1_Life_Hours > dev_data.UV_EOL_MAXIMUM)||(UV2_Life_Hours > dev_data.UV_EOL_MAXIMUM))
	{
		if(pilot_lamp_mode != P1_BT_UNPAIR)
		{
			pilot_lamp_mode = P2_BT_DEV_FAULT;
			turnON_RED();
		}
	}
	else
	{
		if(pilot_lamp_mode == P2_BT_DEV_FAULT)
		{
			switch(Device_mode)
			{
				case MODE_CMNS:
					pilot_lamp_mode = P9_Disinfection;
					turnON_BLUE();
					break;
				case MODE_CMS:
					pilot_lamp_mode = STEALTH_MODE;
					turn_all_off();
					break;
				case MODE_SBNS:
					pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
					turnON_GREEN();
					break;
				case MODE_SBS:
					pilot_lamp_mode = STEALTH_MODE;
					turn_all_off();
					break;
			}
		}
	}
	
}




















