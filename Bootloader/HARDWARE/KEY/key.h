#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h" 
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

/*下面的方式是通过直接操作库函数方式读取IO*/
#define SW3 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) //PC8
#define SW4 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)	//PB7 
extern u8  UV1_open_flag,UV2_open_flag;
extern u8 UV1_Life_write_EEPROM_value[2],UV2_Life_write_EEPROM_value[2];//UV灯寿命存储写入数组定义
extern u8 UV1_Life_read_EEPROM_value[2],UV2_Life_read_EEPROM_value[2];//UV灯寿命存储读取数组定义
extern u8 BLE_Mesh_init_flag;//蓝牙模组初始化标志位  
extern u8 BLE_close_flag;//蓝牙模组复位功能关闭标志位
extern u8 hidden_open;//隐身模式标志位
void KEY_Init(void);	//IO初始化
void KEY_Scan(void);
#endif

