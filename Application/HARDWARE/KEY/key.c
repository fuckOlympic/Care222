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
u8 BLErst_flag=0;//蓝牙复位标志位
u8 togger_mode=0;//模式切换，物理按键按住5秒切换contuiuns mode/standby mode
u8 BLE_Mesh_init_flag=0;
u8  UV1_open_flag=0,UV2_open_flag=0;//UV灯开启时间标志位
//u16 UV1_Life_write_EEPROM_value[2]={0},UV2_Life_write_EEPROM_value[2]={0};//UV灯寿命存储写入数组定义
u8 UV1_Life_read_EEPROM_value[2]={0},UV2_Life_read_EEPROM_value[2]={0};//UV灯寿命存储读取数组定义
u8 BLE_close_flag=0;//蓝牙模组复位功能关闭标志位
u8 BLE_RST_Enable_flag=1;
u8 standby_continuous_flag=0,hidden_normal_flag=0;  
u8 hidden_open=0;//隐身模式标志位
u8 key_schedule_flag=0;//Schedule模式通过APP控制按键标志位，当key_schedule_flag=0时，schedule模式打开;否则schedule模式关闭
u8 manual_schedule_flag = 1;
u8 key_sw3_pressed = 0, key_sw3_released = 0;
time_t key_pressed_time, key_released_time;
u8 manual_key_off_flag = 0;
u8 manual_key_off_count = 0;

/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
extern uint8_t AT24CXX_exist_flag;
extern uint16_t uvlamp_fan_status;
/***Added end***/

//按键初始化函数
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOB,GPIOC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //SW3对应引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//悬空
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC8
	
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//SW4对应引脚PB7
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//悬空
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB7
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

//按键扫描
void KEY_Scan(void)
{
	u8 i=0;
#if 1
	if(key_sw3_pressed == 1)
	{
		if(schedule_flag==0)//schedule计划模式打开
		{
			key_schedule_flag=0;
//			TIM_Cmd(TIM4,ENABLE); //使能定时器4,因为schedule的运行需要用到
//			printf("key_open schedule55\r\n");
		}
		else
		{
			key_schedule_flag=1;//schedule计划模式关闭
//		TIM_Cmd(TIM4,DISABLE); //失能定时器4,终止schedule
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

					BLErst_flag=1;//蓝牙复位置1
					BLE_Mesh(1);
					default_set();
					pilot_lamp_mode = P1_BT_UNPAIR;
	//				TIM_Cmd(TIM4,DISABLE); //失能定时器4，计划模式关闭
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
	//长按SW3 5~10秒进入countine模式	
	while(SW3==1)
	{
		sw3_count++;
		if(schedule_flag==0)//schedule计划模式打开
		{
			key_schedule_flag=0;
//			TIM_Cmd(TIM4,ENABLE); //使能定时器4,因为schedule的运行需要用到
			printf("key_open schedule55\r\n");
		}
		else
		{
			key_schedule_flag=1;//schedule计划模式关闭
//		TIM_Cmd(TIM4,DISABLE); //失能定时器4,终止schedule
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
		if(sw3_count>=10)//长按10秒以上，复位蓝牙模块和MCU
		{
			sw3_count=0;
			BLErst_flag=1;//蓝牙复位置1
			BLE_Mesh(1);
			default_set();
			
//			TIM_Cmd(TIM4,DISABLE); //失能定时器4，计划模式关闭
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

	if(sw3_count>=5&&sw3_count<10&&key_schedule_flag==1)//长按5到10秒，安装人员检查灯具是否在工作
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
	if(sw3_count<=3&&sw3_count>=1&&key_schedule_flag==1)//短按1到3秒，强制退出schedule状态，进入standby状态
	{
		sw3_count=0;
		default_set();	
	}

#endif
//用于存储UV1灯寿命的EEPROM标志位
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
 			AT24CXX_WriteBytes(UV1_LIFETIME_ADDR, UV1_Life, 2);    //AT24LC64写入数据 
		printf("UV1 write successful:%d,%02X,%02X\r\n",UV1_Life_Hours,UV1_Life_read_EEPROM_value[0],UV1_Life_read_EEPROM_value[1]);
  }
  
//用于存储UV2灯寿命的EEPROM标志位
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
			AT24CXX_WriteBytes(UV2_LIFETIME_ADDR, UV2_Life, 2);    //AT24LC64写入数据 
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




















