#include "pir.h"
#include "delay.h" 	 

//PIR��ʼ������
void PIR_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //PIR��Ӧ����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//���գ���ΪӲ����������������
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC1
 
} 





















