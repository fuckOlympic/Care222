#include "fan_motor.h" 	 

//��ʼ��PC9��PC13Ϊ�����.��ʹ���������ڵ�ʱ��		    
//FAN IO��ʼ��
void FAN_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��

	GPIO_ResetBits(GPIOC,GPIO_Pin_9| GPIO_Pin_13);//GPIOC9&GPIOC13���0
	
  //GPIOC9,C13��ʼ������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
	

//  GPIO_ResetBits(GPIOC,GPIO_Pin_9);//GPIOC9���ø���1
}






