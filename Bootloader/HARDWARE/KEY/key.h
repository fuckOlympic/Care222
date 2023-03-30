#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h" 
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
#define SW3 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) //PC8
#define SW4 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)	//PB7 
extern u8  UV1_open_flag,UV2_open_flag;
extern u8 UV1_Life_write_EEPROM_value[2],UV2_Life_write_EEPROM_value[2];//UV�������洢д�����鶨��
extern u8 UV1_Life_read_EEPROM_value[2],UV2_Life_read_EEPROM_value[2];//UV�������洢��ȡ���鶨��
extern u8 BLE_Mesh_init_flag;//����ģ���ʼ����־λ  
extern u8 BLE_close_flag;//����ģ�鸴λ���ܹرձ�־λ
extern u8 hidden_open;//����ģʽ��־λ
void KEY_Init(void);	//IO��ʼ��
void KEY_Scan(void);
#endif

