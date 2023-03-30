#ifndef __PIR_H
#define __PIR_H	 
#include "sys.h" 
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 
/*下面的方式是通过直接操作库函数方式读取IO*/
#define PIR_input 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1) //PC1
void PIR_Init(void);	//IO初始化

#endif
