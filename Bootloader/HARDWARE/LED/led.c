#include "led.h" 	 


//LED IO初始化
void LED_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOB时钟
  
 /***********************************************************/
  //初始化PB1,PB14和PB15为输出口.并使能这三个口的时钟	RGB灯带  PB1:R  PB14:G  PB15:B
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
  
  GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1设置高，灯亮R
  GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5设置高，灯亮G
  GPIO_SetBits(GPIOB,GPIO_Pin_15);//GPIOA5设置高，灯亮B
 /***********************************************************/
  
/************************************************************/  
 //初始化PC5，PC10为输出口.并使能这两个口的时钟	D5,D6指示灯 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
  
  GPIO_SetBits(GPIOC,GPIO_Pin_5);//GPIOC5设置高
  GPIO_SetBits(GPIOC,GPIO_Pin_10);//GPIOC10设置高
/**********************************************************/	
	

}






