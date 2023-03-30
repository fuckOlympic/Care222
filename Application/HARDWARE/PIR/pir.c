#include "pir.h"
#include "delay.h" 	 

//PIR初始化函数
void PIR_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOC时钟
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //PIR对应引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//悬空，因为硬件上已作上拉处理
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC1
 
} 





















