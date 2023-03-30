#include "uv_lamp.h" 	 


//UV�� IO��ʼ��
void UV_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOCʱ��
  
 /***********************************************************/
  //��ʼ��PC0/PC2/PC4/PC6Ϊ�����.��ʹ����1���ڵ�ʱ��	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
  
  GPIO_ResetBits(GPIOC,GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);//GPIOC0���õ�  P3_UV1
	 
#if 0
 //��ʼ��PC4Ϊ�����.��ʹ����1���ڵ�ʱ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
  
  GPIO_ResetBits(GPIOC,GPIO_Pin_4);//GPIOC4���õ�  P4_UV2
#endif

/**********************************************************/	


/***********************************************************/
  //��ʼ��PB0,PB10,PB12,PC3Ϊ�����.��ʹ���������ڵ�ʱ��	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_10|GPIO_Pin_12; //PB0ΪLAMP_ON_A��Ӧ���ţ�PB10��PCB12Ϊ��˫���趨����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//����
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB0
	
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//LAMP_ON_B��Ӧ����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//����
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC3
/**********************************************************/	

	//��ʼ��PB3Ϊ�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //PB3ΪUV�Ƶ�Դʹ���ź�
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//����
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB0
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);

}






