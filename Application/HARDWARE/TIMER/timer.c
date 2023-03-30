#include "timer.h"
#include "led.h"
#include "function.h"
#include "uv_lamp.h"
#include "key.h"
#include "usart.h"
#include "rtc.h"
#include "24cxx.h"
#include "usart1.h"
#include "dma.h"

u32 UV1_life_sec_count=0, UV1_life_min_count=0, UV1_life_hour_count=0;  //UV1 Lamp second/minute/hour Counter
u32 UV2_life_sec_count=0, UV2_life_min_count=0, UV2_life_hour_count=0;  //UV2 Lamp second/minute/hour Counter
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

//u16 str_smart_value=0;
u16 str_duty_value=0;  //UV open seconds in 1500s period
u16 duty_count=0; 		//UV Cycle counter   
//u8 UV_flag=0;
u8 UV1_EEPROM_FLAG=0,UV2_EEPROM_FLAG=0; //UV Lamp Lifetime update flag
u8 UV12_status=0;
u8 execute_Plan_flag=0;//计划执行标志位
u8 Identify_Device_flag=0;//指示灯打开标志位
u16 Identify_Device_count=0;//打开时间计数
u16 RTC_count=0;//RTC时钟计数
u16 BLE_count=0;//蓝牙模组停止使用复位功能计数  
u16 log_count=0;//事件LOG定时读数30秒定时
u8 log_count2=0;//事件LOG定时读数2秒定时
u8 log_flag=0;
u8 log_flag2=0;
//u16 INTO_delay_flag=0,INTO_delay_count1=0;
//u16 wring_life_count=0,wring_life_flag=0;
//u16 wring_life_buff[1]={0};

//Unoccupied_mode and Smart_mode flag and count variables
u16 Unoccupied_mode_count=0;
u8 Unoccupied_mode_flag=0;//检测到人员时将灯暂停的时长，单位为秒
u16 Smart_mode_count=0;
u8 Smart_mode_flag=0;//检测到人员时启动灯后，灯的运行周期数

u16 Manual_count_key=0;

u16 pilot_counter = 0;

uint16_t FAN1_IC1Value = 0;
uint16_t FAN1_Frequency = 0;
uint16_t FAN2_IC1Value = 0;
uint16_t FAN2_Frequency = 0;

u8 uv_turnon_check_flag = 0;

u16 tim13_counter = 0;

extern uint8_t uv_restart_delay_flag;
extern uint8_t uv_restart_delay_count;
extern u8 USART1_length_value;
extern uint8_t app_config_flag;
extern uint8_t ble_connection_status;
extern uint8_t app_config_cnt;

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
//作用：UV灯工作循环周期定时
void TIM7_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);  ///使能TIM7时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseInitStructure);//初始化TIM7
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE); //允许定时器7更新中断
	TIM_Cmd(TIM7,DISABLE); //失能定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM7_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

//定时器7中断服务函数 作用：UV灯工作循环周期定时 DutyCycle判定
void TIM7_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM7,TIM_IT_Update)==SET) //溢出中断
	{
		if((manual_schedule_flag == 0 )&&(uv_restart_delay_flag == 0) && (manual_key_off_flag == 0))
		{
			duty_count++;		
			if(duty_count == UV_TURNON_CHECK_DELAY)
			{
				uv_turnon_check_flag = 1;
			}
			if(duty_count <= str_duty_value)
			{
				if(UV12_status == 0)
				{				
					printf("open lamp: duty_cycle = %d UV12_status = %d\r\n", duty_count, UV12_status);
					UV12_status = 1;
					D8=0;
				}
				//printf("str_duty_value:%d",dev_data.UV_DUTY_CYCLE);
			}
			else if(duty_count< UV_LAMP_PERIOD) //1500)
			{      			
				if(UV12_status == 1)
				{
					printf("close lamp: duty_cycle = %d UV12_status = %d\r\n", duty_count, UV12_status);
					close_uv_lamp();
					UV12_status=0;
					D8=1;
				}      
			}
			else
			{
				duty_count=0;
				UV12_status = 0;
			}
	//  printf("TIM7 is open!\r\n");
			D9=!D9;//D9翻转

		}
		else if((manual_schedule_flag == 1) && (uv_restart_delay_flag == 0) )
		{
			Manual_count_key++;
			if(Manual_count_key == UV_TURNON_CHECK_DELAY)
			{
				uv_turnon_check_flag = 1;
			}
			if(Manual_count_key >= dev_data.Manual_On_Time)
			{
				//key_schedule_flag=0;
				default_set();//standby mode
				Manual_count_key=0;
				printf("Lamp Manual On Timeout\r\n");
			}
		}
		
		if(UV1_open_flag==1)
		{
			UV1_life_sec_count++;
			if(UV1_life_sec_count>=60)
			{
				UV1_life_min_count++;
				UV1_life_sec_count=0;
				printf("UV1_life_min_count:%d\r\n",UV1_life_min_count);
				if(UV1_life_min_count>=60)
				{
					UV1_EEPROM_FLAG=1;
					UV1_Life_Hours++;
					printf("UV1_life_hour_count:%d\r\n",UV1_Life_Hours);					
					UV1_life_min_count=0;
				}
			}
		}
		if(UV2_open_flag==1)
		{
			UV2_life_sec_count++;
			if(UV2_life_sec_count>=60)
			{
				UV2_life_min_count++;
				UV2_life_sec_count=0;
				printf("UV2_life_min_count:%d\r\n",UV2_life_min_count);
				if(UV2_life_min_count>=60)
				{
					UV2_EEPROM_FLAG=1;
					UV2_Life_Hours++;
					printf("UV2_life_hour_count:%d\r\n",UV2_Life_Hours);
					UV2_life_min_count=0;
				}
			}
		}
	}
	TIM_ClearITPendingBit(TIM7,TIM_IT_Update);  //清除中断标志位
}


//通用定时器9中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//作用：用于获取UV灯寿命定时
void TIM9_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9,ENABLE);  ///使能TIM9时钟       
	TIM_TimeBaseInitStructure.TIM_Period = arr;         //自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM9,&TIM_TimeBaseInitStructure);//初始化TIM9
	TIM_ITConfig(TIM9,TIM_IT_Update,ENABLE); //允许定时器9更新中断
	TIM_Cmd(TIM9,DISABLE); //失能定时器9

	NVIC_InitStructure.NVIC_IRQChannel=TIM1_BRK_TIM9_IRQn; //定时器9中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);       
}

//定时器9中断服务函数 
//作用：用于获取UV灯寿命定时
void TIM1_BRK_TIM9_IRQHandler(void)       
{
	if(TIM_GetITStatus(TIM9,TIM_IT_Update)==SET) //溢出中断
  {
#if 0		
//		D9 = !D9;
		if(UV1_open_flag==1)
		{
			UV1_life_sec_count++;
			if(UV1_life_sec_count>=120)
			{
				UV1_life_min_count++;
//      UV1_EEPROM_FLAG=1;
//			UV1_life_hour_count++;
				UV1_life_sec_count=0;
				printf("UV1_life_min_count:%d\r\n",UV1_life_min_count);
				if(UV1_life_min_count>=60)
				{
					UV1_EEPROM_FLAG=1;
//					UV1_life_hour_count++;
					UV1_Life_Hours++;
					printf("UV1_life_hour_count:%d\r\n",UV1_Life_Hours);					
					UV1_life_min_count=0;
				}
			}
		}
		if(UV2_open_flag==1)
		{
			UV2_life_sec_count++;
			if(UV2_life_sec_count>=120)
			{
				UV2_life_min_count++;
//      UV2_EEPROM_FLAG=1;
//			UV2_life_hour_count++;
				UV2_life_sec_count=0;
				printf("UV2_life_min_count:%d\r\n",UV2_life_min_count);
				if(UV2_life_min_count>=60)
				{
					UV2_EEPROM_FLAG=1;
//					UV2_life_hour_count++;
					UV2_Life_Hours++;
					printf("UV2_life_hour_count:%d\r\n",UV2_Life_Hours);
					UV2_life_min_count=0;
				}
			}
		}
//		if(Identify_Device_flag==1)//指示灯闪烁标志位
//		{
//			Identify_Device_count++;
//			if(Identify_Device_count==10)
//			{
//				Identify_Device_flag=0;//指示灯打开/关闭标志位
//				Identify_Device_count=0;//指示灯计数
//				TIM_Cmd(TIM9,DISABLE); //失能定时器9
//			}
//		}
	//	 printf("TIM9 is open!\r\n");
#endif
	}
	TIM_ClearITPendingBit(TIM9, TIM_IT_Update  );  //清除TIM9更新中断标志   
}

//通用定时器4中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器4!
//作用：RTC时钟获取
void TIM4_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///使能TIM4时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//初始化TIM4
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //允许定时器4更新中断
	TIM_Cmd(TIM4,ENABLE); //使能定时器4
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //定时器4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

//定时器4中断服务函数 作用：UV灯工作循环周期定时
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //溢出中断
	{		
//		if(BLE_PLAN_tab_flag==1)
		if(manual_schedule_flag == 0)
		{
			RTC_count++;
			if(RTC_count==5)
			{
				RTC_count=0;
				execute_Plan_flag=1;//计划执行开启标志位  置1启动计划
			}
		}
		if(uv_restart_delay_flag)
		{
			uv_restart_delay_count++;
			if(uv_restart_delay_count == UV_RESTART_DELAY)
			{
				uv_restart_delay_flag = 2;
				uv_restart_delay_count = 0;
			}
		}
		
		if(manual_key_off_flag)
		{
			manual_key_off_count++;
			if(manual_key_off_count == UV_RESTART_DELAY)
			{
				manual_key_off_flag = 0;
				manual_key_off_count = 0;
			}
		}
//		if(BLE_PLAN_tab_flag!=5)
		if(BLE_RST_Enable_flag == 1)
		{
			BLE_count++;
			if(BLE_count==300)
			{
				printf("BLE key reset func timeout\r\n");
//				BLE_close_flag=5;
				BLE_RST_Enable_flag = 0;
				BLE_count=0;
			}
		}
		if((app_config_flag == 0) && (ble_connection_status == 1))
		{
			if(app_config_cnt < 180)
			{
				app_config_cnt++;
			}
		}
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
//		printf("TIM4 is open!\r\n");
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //清除中断标志位
}
//通用定时器5中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器5!
//作用：事件LOG上报定时
void TIM5_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///使能TIM5时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);//初始化TIM7
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); //允许定时器5更新中断
	TIM_Cmd(TIM5,ENABLE); //使能定时器5
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; //定时器5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}

//定时器5中断服务函数 作用：事件LOG上报工作循环周期定时
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET) //溢出中断
	{
//		D8 = !D8;
		log_count++;
		log_count2++;
		if(log_count==50*1)
		{
			log_flag=1;//当log_flag置1时，触发事件LOG上报
			log_count=0;//计数变量清零
		}
		if(log_count2==5*5)//第一次上报事件LOG为2秒周期
		{
			log_flag2=1;//当log_flag2置1时，触发事件LOG上报
			log_count2=0;//计数变量清零
		}

//		if((dev_data.First_Connect_Flag == 0) &&(BurnIn_mode == 0))
		if(BurnIn_mode == 0)
		{
			if(pilot_lamp_mode == P1_BT_UNPAIR )
			{
				if((pilot_counter % 6) == 0)
				{
					turnON_RED();
				}
				else if((pilot_counter % 6) == 3)
				{
					turn_all_off();
				}
				pilot_counter++;
				if(pilot_counter == 60000)
					pilot_counter = 0;
			}
		}
		else
		{
			if(pilot_counter >0)
				pilot_counter = 0;
		}
		
//		if(Identify_Device_flag == 1)
//		{
//			if((Identify_Device_count%2) == 1)
//					turn_all_off();
//			else
//					turnON_GREEN();
//		}
//    printf("TIM5_OPEN\r\n");
	}
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);  //清除中断标志位
}
//通用定时器6中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器6!
//作用：WRING LIFE
void TIM6_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);  ///使能TIM6时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);//初始化TIM6
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE); //允许定时器6更新中断
	TIM_Cmd(TIM6,ENABLE); //使能定时器6
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM6_DAC_IRQn; //定时器6中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

//定时器6中断服务函数 作用：warning
void TIM6_DAC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)==SET) //溢出中断
	{
#if 0
		INTO_delay_flag++;
		
		if(INTO_delay_flag>=30)
		{
			INTO_delay_count1=1;
			INTO_delay_flag=0;
//			if(BurnIn_mode == 0)
//				printf("get_once_time\r\n");
		}			
//		if(unoccupied_flag==1)
//		{
//			Unoccupied_mode_count++;
//		}
#endif
#if 0
		if(key_schedule_flag==2)
		{
			Manual_count_key++;
			if(Manual_count_key>=Manual_time_count)
			{
				key_schedule_flag=0;
				default_set();//standby mode
				Manual_count_key=0;
//				TIM_Cmd(TIM4,ENABLE); //使能定时器4,因为schedule的运行需要用到
				printf("into TIM6\r\n");
			}
		}
#endif
//		printf("Manual_count_key:%d,Manual_time_count:%d\r\n",Manual_count_key,Manual_time_count);
//		printf("TIM6 is open\r\n");  
	}
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update);  //清除中断标志位
}


void TIM13_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	
	  /* TIM13 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
	
  /* TIM4 chennel1 configuration : PA.06 */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Connect TIM pin to AF9 */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM13);
	
	  /* Enable the TIM13 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_TIM13_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	TIM_TimeBaseStruct.TIM_Period=9999;
	TIM_TimeBaseStruct.TIM_Prescaler=999;
	TIM_TimeBaseStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM13,&TIM_TimeBaseStruct);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_PWMIConfig(TIM13, &TIM_ICInitStructure);
	
	  /* Select the TIM13 Input Trigger: TI1FP1 */
  TIM_SelectInputTrigger(TIM13, TIM_TS_TI1FP1);

  /* Select the slave Mode: Reset Mode */
  TIM_SelectSlaveMode(TIM13, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIM13,TIM_MasterSlaveMode_Enable);
	TIM_SetCounter(TIM13, 0);
	 /* TIM enable counter */
  TIM_Cmd(TIM13, ENABLE);

  /* Enable the CC1 Interrupt Request */
   TIM_ITConfig(TIM13, TIM_IT_CC1, ENABLE);
}

/**
  * @brief  This function handles TIM13 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM8_UP_TIM13_IRQHandler(void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);

  /* Clear TIM4 Capture compare interrupt pending bit */
  TIM_ClearITPendingBit(TIM13, TIM_IT_CC1);
//	printf("enter TIM8_UP_TIM13_IRQHandler\r\n");
  /* Get the Input Capture value */
  FAN1_IC1Value = TIM_GetCapture1(TIM13);

  if (FAN1_IC1Value != 0)
  {
    /* Frequency computation 
       TIM4 counter clock = (RCC_Clocks.HCLK_Frequency)/2000 */

    FAN1_Frequency = (RCC_Clocks.HCLK_Frequency)/2000 / FAN1_IC1Value;
		TIM_SetCounter(TIM13, 0);
  }
  else
  {
    FAN1_Frequency = 0;
  }
	
}

void TIM14_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	
	  /* TIM14 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	
  /* TIM14 chennel1 configuration : PA.07 */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Connect TIM pin to AF9 */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM14);
	
	  /* Enable the TIM14 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM8_TRG_COM_TIM14_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStruct.TIM_Period=9999;
	TIM_TimeBaseStruct.TIM_Prescaler=999;
	TIM_TimeBaseStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM14,&TIM_TimeBaseStruct);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_PWMIConfig(TIM14, &TIM_ICInitStructure);
	
	  /* Select the TIM14 Input Trigger: TI1FP1 */
  TIM_SelectInputTrigger(TIM14, TIM_TS_TI1FP1);

  /* Select the slave Mode: Reset Mode */
  TIM_SelectSlaveMode(TIM14, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIM14,TIM_MasterSlaveMode_Enable);
	
	 /* TIM enable counter */
  TIM_Cmd(TIM14, ENABLE);
	
	TIM_SetCounter(TIM14, 0);

  /* Enable the CC1 Interrupt Request */
  TIM_ITConfig(TIM14, TIM_IT_CC1, ENABLE);
	
}

/**
  * @brief  This function handles TIM13 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);

  /* Clear TIM4 Capture compare interrupt pending bit */
  TIM_ClearITPendingBit(TIM14, TIM_IT_CC1);
//	printf("enter TIM8_TRG_COM_TIM14_IRQHandler\r\n");
  /* Get the Input Capture value */
  FAN2_IC1Value = TIM_GetCapture1(TIM14);
//	printf("TIM8_TRG_COM_TIM14_IRQHandler : FAN2_IC1Value = %d\r\n", FAN2_IC1Value);
  if (FAN2_IC1Value != 0)
  {
    /* Frequency computation 
       TIM4 counter clock = (RCC_Clocks.HCLK_Frequency)/2000 */
    FAN2_Frequency = (RCC_Clocks.HCLK_Frequency)/2000 / FAN2_IC1Value;
		TIM_SetCounter(TIM14, 0);
  }
  else
  {
    FAN2_Frequency = 0;
  }
}

//通用定时器10中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//作用：用于获取UV灯寿命定时
void TIM10_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10,ENABLE);  ///使能TIM9时钟       
	TIM_TimeBaseInitStructure.TIM_Period = arr;         //自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM10,&TIM_TimeBaseInitStructure);//初始化TIM10
	TIM_ITConfig(TIM10,TIM_IT_Update,ENABLE); //允许定时器9更新中断
	TIM_Cmd(TIM10,DISABLE); //失能定时器10

	NVIC_InitStructure.NVIC_IRQChannel=TIM1_UP_TIM10_IRQn; //定时器10中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);       
}

//定时器10中断服务函数 
//作用：用于UART1空闲判断
void TIM1_UP_TIM10_IRQHandler(void)       
{
	if(TIM_GetITStatus(TIM10,TIM_IT_Update)==SET) //溢出中断
  {
		u8 Usart1_Rec_Cnt;
		Usart1_Rec_Cnt = SEND_BUF_SIZE-DMA_GetCurrDataCounter(DMA2_Stream5);	//算出接本帧数据长度
 		USART1_length_value=Usart1_Rec_Cnt;
		MYDMA_Enable(DMA2_Stream5,SEND_BUF_SIZE);			//恢复DMA指针，等待下一次的接收
	}
	TIM_ClearITPendingBit(TIM10, TIM_IT_Update);  //清除TIM9更新中断标志   
	TIM_SetCounter(TIM10, 0);
	TIM_Cmd(TIM10,DISABLE); 
}

