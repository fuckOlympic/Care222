#include "uv_lamp.h" 	 


//UV灯 IO初始化
void UV_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOC时钟
  
 /***********************************************************/
  //初始化PC0/PC2/PC4/PC6为输出口.并使能这1个口的时钟	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
  
  GPIO_ResetBits(GPIOC,GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);//GPIOC0设置低  P3_UV1
	 
#if 0
 //初始化PC4为输出口.并使能这1个口的时钟
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
  
  GPIO_ResetBits(GPIOC,GPIO_Pin_4);//GPIOC4设置低  P4_UV2
#endif

/**********************************************************/	


/***********************************************************/
  //初始化PB0,PB10,PB12,PC3为输入口.并使能这两个口的时钟	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_10|GPIO_Pin_12; //PB0为LAMP_ON_A对应引脚，PB10，PCB12为单双灯设定开关
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//悬空
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB0
	
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//LAMP_ON_B对应引脚
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//悬空
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC3
/**********************************************************/	

	//初始化PB3为输出口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //PB3为UV灯电源使能信号
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输入模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//悬空
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB0
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);

}






