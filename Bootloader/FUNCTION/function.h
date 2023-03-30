#ifndef __FUNCTION_H
#define __FUNCTION_H
#include "sys.h"
extern u8 Stop_send_flag; //停止发送指令状态标志位
extern u32 SN_count;//序列号自增变量
extern u32 RTC_time_value;//RTC时钟数值变量
extern RTC_TimeTypeDef RTC_TimeStruct;
extern u8 Plan_happen;
extern u8 BLE_PLAN_tab_flag;//BLE模组和计划模式在定时器9切换使用标志位
extern u8 Device_data[18];
extern u16 Duty_cycle_read_buff[2];
void GetAll_Status(void);
void Search_Info(void);
void BLE_Mesh(void);
void Connect_Status(void);
void Switch_UV(void);
void Smart_Mode(void);
void Duty_Cycle(void);
void Device_Mode(void);
void Get_Time(void);
void Life_Time(void);
void Plan(void);
void execute_Plan(void);//执行计划
void Device_Init(void);
void Unoccupied_Mode(void);
void Identify_Device(void);
#endif  

