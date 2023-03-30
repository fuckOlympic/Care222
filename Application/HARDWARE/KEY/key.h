#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h" 
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
#define SW3 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) //PC8
#define SW4 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)	//PB7 
extern u8  UV1_open_flag,UV2_open_flag;
extern u16 sw3_count,sw4_count;
extern u8 TIM9_flag;
extern u8 togger_mode;//ģʽ�л�����������ס5���л�contuiuns mode/standby mode
extern u8 BLErst_flag;//������λ��־λ
extern u8 UV1_Life_read_EEPROM_value[2], UV2_Life_read_EEPROM_value[2];//UV�������洢��ȡ���鶨��
extern u8 BLE_Mesh_init_flag;//����ģ���ʼ����־λ  
extern u8 BLE_close_flag;//����ģ�鸴λ���ܹرձ�־λ
extern u8 BLE_RST_Enable_flag;
extern u8 hidden_open;//����ģʽ��־λ
extern u8 key_schedule_flag;//Scheduleģʽͨ��APP���ư�����־λ����key_schedule_flag=0ʱ��scheduleģʽ��;����scheduleģʽ�ر�
extern u8 manual_schedule_flag;
extern u8 manual_key_off_flag;
extern u8 manual_key_off_count;
//extern u8 key_sw3_pressed, key_sw3_released;
//extern time_t key_pressed_time, key_released_time;
void KEY_Init(void);	//IO��ʼ��
void KEY_Scan(void);
#endif

