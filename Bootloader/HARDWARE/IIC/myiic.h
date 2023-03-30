#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

//IO��������
 
#define SDA_IN()  {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=0<<9*2;}        //PB9����ģʽ
#define SDA_OUT() {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=1<<9*2;} //PB9���ģʽ
//#define SDA_IN()  {GPIOB->MODER&=0X0FFFFFFF;GPIOB->MODER|=(u32)8<<28;}
//#define SDA_OUT() {GPIOB->MODER&=0X0FFFFFFF;GPIOB->MODER|=(u32)3<<28;}

//IO��������	 
#define IIC_SCL    PBout(8) //SCL
#define IIC_SDA    PBout(9) //SDA	 
#define READ_SDA   PBin(9)  //����SDA 

#define I2C_TIMEOUT				0x00200000
#define I2C_TM_ERR				0x0000FFFF
#define I2C_TM_OK				0x00000000

//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  
#endif
















