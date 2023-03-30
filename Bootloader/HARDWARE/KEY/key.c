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
u8  UV1_open_flag=0,UV2_open_flag=0;//UV灯开启时间标志位
u8 UV1_Life_write_EEPROM_value[2]={0},UV2_Life_write_EEPROM_value[2]={0};//UV灯寿命存储写入数组定义
u8 UV1_Life_read_EEPROM_value[2]={0},UV2_Life_read_EEPROM_value[2]={0};//UV灯寿命存储读取数组定义
u8 BLE_close_flag=0;//蓝牙模组复位功能关闭标志位
u8 standby_continuous_flag=0,hidden_normal_flag=0;  
u8 hidden_open=0;//隐身模式标志位
//按键初始化函数
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOB,GPIOC时钟
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //SW3对应引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//悬空
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC8
	
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//SW4对应引脚PB7
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//悬空
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB7
 
} 

//按键扫描
void KEY_Scan(void)
{
	u8 i=0;
//长按SW4 5~10秒进入countine模式
	while(SW4==1)
		{
			sw4_count++;
			printf("sw4_count:%d\r\n",sw4_count);
			delay_ms(1000);
		}
		if(BLE_close_flag!=5)
		{
			BLE_PLAN_tab_flag=5;
			
			if(sw4_count>=15)//长按15秒以上，复位蓝牙模块
			{
				sw4_count=0;
				TIM_Cmd(TIM4,DISABLE); //失能定时器4，计划模式关闭
				if(hidden_open==0)
				{
				BLE_Mesh();
					while(i!=3)
					{
						GPIO_SetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
						GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
						GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
						delay_ms(300);
						GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
						GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
						GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
						delay_ms(300);
						i++;
					}
//				GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
//				GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
//				GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B	
				printf("time up\r\n");
			}
		}
	}
		if(sw4_count>=5&&sw4_count<=10)//长按5到10秒，手动切换Continuous/standby模式
			{
				sw4_count=0;
				
				if(hidden_open==0)
				{
					standby_continuous_flag=!standby_continuous_flag;
					if(standby_continuous_flag==1)//continuous_mode模式开启
					{
						TIM_Cmd(TIM4,DISABLE); //失能定时器4，计划模式关闭
						while(i!=2)
							{
								GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
								GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
								GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
								delay_ms(300);
								GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
								GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
								GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
								delay_ms(300);
								i++;
							}
							AT24CXX_Read(1,Duty_cycle_read_buff,2);//从存储器中读出Duty_cycle值
						  str_duty_value=(Duty_cycle_read_buff[0]*256+Duty_cycle_read_buff[1])*1.5;//Duty-Cycle值换算到定时器执行
						  duty_count=0;//TIM3变量计数置0
						  TIM_Cmd(TIM3,ENABLE);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
							GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
							GPIO_SetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
							printf("continuous\r\n");	
					}
					else//standby_mode开启
					{
							TIM_Cmd(TIM3,DISABLE);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
							GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
					}
			 }
			}
	  if(sw4_count<=3&&sw4_count>=1)//短按1到3秒，切换隐身/正常模式
			{
				sw4_count=0;
				hidden_normal_flag=!hidden_normal_flag;
				TIM_Cmd(TIM4,DISABLE); //失能定时器4，计划模式关闭
				if(hidden_normal_flag==1)
				{
					hidden_open=1;//隐身模式开启
					while(i!=3)
						{
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
							GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
							delay_ms(300);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
							GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
							delay_ms(300);
							i++;
						}
				}
				else
				{
					hidden_open=0;//隐身模式关闭
					while(i!=3)
						{
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
							GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
							delay_ms(300);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
							GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
							delay_ms(300);
							GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
							GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
							GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
							i++;
						}
				}
				printf("hidded\r\n");
			}

//	else 
//	{
//		//SW4:蓝牙模组重置复位
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
//		u8 str_UV12_status[9]={0x55,0xAA,0x01,0x00,0x05,0x22,0x01};  //帧格式
//		SN_count++;
//  str_UV12_status[3]=0x00;
//  str_UV12_status[4]=SN_count;
////		Usart1_Send(str_UV12_status,sizeof(str_UV12_status));
//	}
//用于存储UV1灯寿命的EEPROM标志位
//  if(UV1_EEPROM_FLAG==1)
//  {
//    UV1_EEPROM_FLAG=0;
//    UV1_Life_write_EEPROM_value[0]=UV1_life_min_count;
////    AT24CXX_Write(0,(u8*)UV1_Life_write_EEPROM_value,sizeof(UV1_Life_write_EEPROM_value));    //AT24LC64写入数据 
//  }
//  
////用于存储UV2灯寿命的EEPROM标志位
//  if(UV2_EEPROM_FLAG==1)
//  {
//    UV2_EEPROM_FLAG=0;
//    UV2_Life_write_EEPROM_value[0]=UV2_life_min_count;
////    AT24CXX_Write(2,(u8*)UV2_Life_write_EEPROM_value,sizeof(UV2_Life_write_EEPROM_value));    //AT24LC64写入数据 
//  }
//  
////读取UV1是否处于打开状态  如果UV1打开，则定时器9使能，UV1_open_flag置1
//  if(LAMP_ON_A==0)
//    {
//      if(TIM9_flag==0)
//      TIM_Cmd(TIM9,ENABLE); //使能定时器9
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
////读取UV2是否处于打开状态  如果UV2打开，则定时器9使能，UV2_open_flag置1
//  if(LAMP_ON_B==0)
//    {
////      TIM_Cmd(TIM9,ENABLE); //使能定时器9
//      UV2_open_flag=1;
////      printf("UV2 is open\r\n");
//      delay_ms(500);
//    }
//  else
//    {
//      TIM_Cmd(TIM9,DISABLE); //失能定时器9
//      UV2_open_flag=0;
////      printf("UV2 is close\r\n");
//      delay_ms(500);
//    }
}




















