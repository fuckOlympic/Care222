#ifndef __FAN_MOTOR_H
#define __FAN_MOTOR_H
#include "sys.h"

//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


//LED�˿ڶ���
#define FAN1_ENABLE PCout(9)	// PC9
#define FAN2_ENABLE PCout(13)	// PC13	 
#define FAN1_EN_STS PCin(9)
#define FAN2_EN_STS PCin(13)

void FAN_Init(void);//��ʼ��		 				    
#endif
