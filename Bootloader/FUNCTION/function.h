#ifndef __FUNCTION_H
#define __FUNCTION_H
#include "sys.h"
extern u8 Stop_send_flag; //ֹͣ����ָ��״̬��־λ
extern u32 SN_count;//���к���������
extern u32 RTC_time_value;//RTCʱ����ֵ����
extern RTC_TimeTypeDef RTC_TimeStruct;
extern u8 Plan_happen;
extern u8 BLE_PLAN_tab_flag;//BLEģ��ͼƻ�ģʽ�ڶ�ʱ��9�л�ʹ�ñ�־λ
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
void execute_Plan(void);//ִ�мƻ�
void Device_Init(void);
void Unoccupied_Mode(void);
void Identify_Device(void);
#endif  

