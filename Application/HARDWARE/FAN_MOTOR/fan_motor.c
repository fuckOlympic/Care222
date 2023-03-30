#include "fan_motor.h" 	 

//初始化PC9和PC13为输出口.并使能这两个口的时钟		    
//FAN IO初始化
void FAN_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOC时钟

	GPIO_ResetBits(GPIOC,GPIO_Pin_9| GPIO_Pin_13);//GPIOC9&GPIOC13输出0
	
  //GPIOC9,C13初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
	

//  GPIO_ResetBits(GPIOC,GPIO_Pin_9);//GPIOC9设置高扇1
}






