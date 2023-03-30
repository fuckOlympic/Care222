#include "sys.h"
#include "usart1.h"	
#include "dma.h"
#include "delay.h"
#include "string.h"
#include "function.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
////////////////////////////////////////////////////////////////////////////////// 	  
 

//#define SEND_BUF_SIZE 200	//�������ݳ���,��õ���sizeof(TEXT_TO_SEND)+2��������.

u8 SendBuff[SEND_BUF_SIZE]={0};	//�������ݻ����� 
u8 USART1_length_value=0;
#if EN_USART1_RX   //���ʹ���˽���	

void Usart1_Send(u8 *buf,u8 len)
{
	u8 t;
  for(t=0;t<len;t++)		//ѭ����������
	{		   
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
			USART_SendData(USART1,buf[t]);
	}	 
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
}

//��ʼ��IO ����1 
//bound:������
void uart1_init(u32 bound){
   //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);   //ʹ�ܴ���1 DMA����
  USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//��������ж�

	//Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

#endif
	
}


void USART1_IRQHandler(void)                	//����1�жϷ������
{
//	u8 Usart1_Rec_Cnt;
  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
			TIM_SetCounter(TIM10, 0);
			TIM_Cmd(TIM10,ENABLE); 
			
		  USART_ReceiveData(USART1);//��ȡ���� ע�⣺������Ҫ�������ܹ�����жϱ�־λ����Ҳ��֪��Ϊɶ��
			Stop_send_flag=0;
			USART_ClearITPendingBit(USART1, USART_IT_IDLE);         //����жϱ�־
//			printf("usart1 idle irq occured\r\n");
#if 0
			Usart1_Rec_Cnt = SEND_BUF_SIZE-DMA_GetCurrDataCounter(DMA2_Stream5);	//����ӱ�֡���ݳ���
 		  USART1_length_value=Usart1_Rec_Cnt;
			MYDMA_Enable(DMA2_Stream5,SEND_BUF_SIZE);			//�ָ�DMAָ�룬�ȴ���һ�εĽ���
#endif		
		//***********֡���ݴ�����************//
//			Usart1_Send(SendBuff,Usart1_Rec_Cnt);
			//*************************************//

  } 
//	else
//	{
//		printf("usart1 irq occured, not idle\r\n");
//		TIM_SetCounter(TIM10, 0);
//		TIM_Cmd(TIM10, DISABLE);
//	}
		
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif	
//��������USART1�����������ȡ��
//����ֵ������
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
 



