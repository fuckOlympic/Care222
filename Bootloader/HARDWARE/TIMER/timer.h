#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/6/16
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
#define UV_flag_ON 1;
#define UV_flag_OFF 0;
extern u16 str_smart_value,str_duty_value,duty_count;
extern u8 UV_flag,UV12_status;
extern u8 Identify_Device_flag;//指示灯打开标志位
extern u8 UV1_EEPROM_FLAG,UV2_EEPROM_FLAG;
extern u8 execute_Plan_flag;//计划执行标志位
extern u32 UV1_life_sec_count,UV1_life_min_count,UV1_life_hour_count,UV2_life_sec_count,UV2_life_min_count,UV2_life_hour_count;
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM9_Int_Init(u16 arr,u16 psc);
void TIM4_Int_Init(u16 arr,u16 psc);
void RX_Packet_Decode(void);
#endif
