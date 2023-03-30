#include "led.h" 	 


//LED IO��ʼ��
void LED_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOBʱ��
  
 /***********************************************************/
  //��ʼ��PB1,PB14��PB15Ϊ�����.��ʹ���������ڵ�ʱ��	RGB�ƴ�  PB1:R  PB14:G  PB15:B
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
  
  GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
  GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
  GPIO_SetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
 /***********************************************************/
  
/************************************************************/  
 //��ʼ��PC5��PC10Ϊ�����.��ʹ���������ڵ�ʱ��	D5,D6ָʾ�� 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
  
  GPIO_SetBits(GPIOC,GPIO_Pin_5);//GPIOC5���ø�
  GPIO_SetBits(GPIOC,GPIO_Pin_10);//GPIOC10���ø�
/**********************************************************/	
	

}






