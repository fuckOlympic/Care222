#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/16
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
#define UV_flag_ON 1;
#define UV_flag_OFF 0;
extern u16 str_smart_value,str_duty_value,duty_count;
extern u8 UV_flag,UV12_status;
extern u8 Identify_Device_flag;//ָʾ�ƴ򿪱�־λ
extern u8 UV1_EEPROM_FLAG,UV2_EEPROM_FLAG;
extern u8 execute_Plan_flag;//�ƻ�ִ�б�־λ
extern u32 UV1_life_sec_count,UV1_life_min_count,UV1_life_hour_count,UV2_life_sec_count,UV2_life_min_count,UV2_life_hour_count;
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM9_Int_Init(u16 arr,u16 psc);
void TIM4_Int_Init(u16 arr,u16 psc);
void RX_Packet_Decode(void);
#endif
