#ifndef __24CXX_H
#define __24CXX_H
#include "myiic.h"   
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	  8191
#define AT24C128	16383
#define AT24C256	32767  
//����Ʒʹ�õ���24Lc64�����Զ���EE_TYPEΪAT24LC64
#define EE_TYPE AT24C64
#define EE_FIRST_USE_FLAG 0x3ff
#define EE_SLAVE_ADDR_W 0xA2
#define EE_SLAVE_ADDR_R 0xA3
					  
u8 AT24CXX_ReadOneByte(u16 ReadAddr);							//ָ����ַ��ȡһ���ֽ�
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		//ָ����ַд��һ���ֽ�
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);//ָ����ַ��ʼд��ָ�����ȵ�����
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);					//ָ����ַ��ʼ��ȡָ����������
void AT24CXX_Write(u16 WriteAddr,u16 *pBuffer,u16 NumToWrite);	//��ָ����ַ��ʼд��ָ�����ȵ�����
void AT24CXX_Read(u16 ReadAddr,u16 *pBuffer,u16 NumToRead);   	//��ָ����ַ��ʼ����ָ�����ȵ�����
void AT24CXX_ReadBytes(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);
void AT24CXX_WriteBytes(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);

u8 AT24CXX_Check(void);  //�������
void AT24CXX_Init(void); //��ʼ��IIC
#endif
















