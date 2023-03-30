#include "sys.h"
#include "usart1.h"	
#include "dma.h"
#include "delay.h"
#include "string.h"
#include "function.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
////////////////////////////////////////////////////////////////////////////////// 	  
 

//#define SEND_BUF_SIZE 200	//发送数据长度,最好等于sizeof(TEXT_TO_SEND)+2的整数倍.

u8 SendBuff[SEND_BUF_SIZE]={0};	//发送数据缓冲区 
u8 USART1_length_value=0;
#if EN_USART1_RX   //如果使能了接收	

void Usart1_Send(u8 *buf,u8 len)
{
	u8 t;
  for(t=0;t<len;t++)		//循环发送数据
	{		   
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
			USART_SendData(USART1,buf[t]);
	}	 
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
}

//初始化IO 串口1 
//bound:波特率
void uart1_init(u32 bound){
   //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);   //使能串口1 DMA接收
  USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif
	
}


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
//	u8 Usart1_Rec_Cnt;
  
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
			TIM_SetCounter(TIM10, 0);
			TIM_Cmd(TIM10,ENABLE); 
			
		  USART_ReceiveData(USART1);//读取数据 注意：这句必须要，否则不能够清除中断标志位。我也不知道为啥！
			Stop_send_flag=0;
			USART_ClearITPendingBit(USART1, USART_IT_IDLE);         //清除中断标志
//			printf("usart1 idle irq occured\r\n");
#if 0
			Usart1_Rec_Cnt = SEND_BUF_SIZE-DMA_GetCurrDataCounter(DMA2_Stream5);	//算出接本帧数据长度
 		  USART1_length_value=Usart1_Rec_Cnt;
			MYDMA_Enable(DMA2_Stream5,SEND_BUF_SIZE);			//恢复DMA指针，等待下一次的接收
#endif		
		//***********帧数据处理函数************//
//			Usart1_Send(SendBuff,Usart1_Rec_Cnt);
			//*************************************//

  } 
//	else
//	{
//		printf("usart1 irq occured, not idle\r\n");
//		TIM_SetCounter(TIM10, 0);
//		TIM_Cmd(TIM10, DISABLE);
//	}
		
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif	
//描述：对USART1串口数据求和取余
//返回值：余数
u16 Sum_Rem(u8 *usart1_buff,u8 buff_length)
{
  u32 t,sum_buff=0;
  if(usart1_buff[0]==0x55&&usart1_buff[1]==0xaa)
  {
    for(t=0;t<buff_length-1;t++)
    {
      sum_buff+=usart1_buff[t];
    }
//		printf("Sum_Rem : sum_buff = %x", sum_buff);
  }
  else return 0;
  return sum_buff%256;
}
 



