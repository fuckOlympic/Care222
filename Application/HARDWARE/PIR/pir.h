#ifndef __PIR_H
#define __PIR_H	 
#include "sys.h" 
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 
/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
#define PIR_input 		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1) //PC1
void PIR_Init(void);	//IO��ʼ��

#endif
