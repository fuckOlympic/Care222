#include "sys.h"
#include "usart1.h"	
#include "dma.h"
#include "delay.h"
#include "string.h"
#include "function.h"
#include "flash.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif

/* 
    This polynomial ( 0xEDB88320L) DOES generate the same CRC values as ZMODEM and PKZIP
 */
#include <stdint.h>
static const uint32_t crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL 
};
 
uint32_t crc32( const unsigned char *buf, uint32_t size)
{
     uint32_t i, crc;
     crc = 0xFFFFFFFF;
     for (i = 0; i < size; i++)
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
     return crc^0xFFFFFFFF;
}
//  char *str="sdfdsfdsfdsfdsfdssdfdsfdsfdsfdsssssssssssssfdsfdsfds";
//    printf("%lld",crc32(str,strlen(str)));
////////////////////////////////////////////////////////////////////////////////// 	  
 /*
 MCU OTA
�汾����	ģ�鷢��	0xAA55 0x01	0xXXXX	0x06	0x01	0x00	У���	0x01
	MCU����	0x55AA 0x01	0xXXXX	0x06	0x02	data[0]-data[1]:�̼��汾����ʽ0001.0000.0001(bit)->1.0.1(ʮ����);	У���	
MCU OTA
����֪ͨ	ģ�鷢��	0xAA55 0x01	0xXXXX	0x07	0x0A	data[0]-data[3]:�̼���С;
data[4]-data[5]:�̼��汾����ʽ0001.0000.0001(bit)->1.0.1(ʮ����);
data[6]-data[9]:�̼�CRC32У��	У���	0x01
	MCU����	0x55AA 0x01	0xXXXX	0x07	0x01	0x00:�ɹ�; 0x01:ʧ��;	У���	
MCU OTA
�̼����ݴ���	ģ�鷢��	0xAA55 0x01	0xXXXX	0x08	
0xXX+1	data[0]-data[3]:�̼�ƫ����;
data[4]-data[0xXX]:�̼����ݣ�<=128bytes��;	У���	0x01
	MCU����	0x55AA 0x01	0xXXXX	0x08	0x01	0x00:�ɹ�; 0x01:ʧ��;	У���	
MCU OTA
�̼���������ϱ�	MCU����	0x55AA 0x01	0xXXXX	0x09	0x03	data[0]: status, 0x00�ɹ���0x01ʧ�ܣ� 
data[1]-data[2]:�̼��汾����ʽ0001.0000.0001(bit)->1.0.1(ʮ����);	У���	0x01
	ģ�鷵��	0xAA55 0x01	0xXXXX	0x09	0x01	0x00:�ɹ�; 0x01:ʧ��;	У���	
ͨѶЭ��-��
 */

//#define SEND_BUF_SIZE 200	//�������ݳ���,��õ���sizeof(TEXT_TO_SEND)+2��������.

u8 SendBuff[SEND_BUF_SIZE]={0};	//�������ݻ����� 
u8 RecvBuff[SEND_BUF_SIZE]={0};
u8 USART1_length_value=0;
extern CONFIG_UPGRADE_CONFIG conf;

#if EN_USART1_RX   //���ʹ���˽���	

//u8 CheckSum(u8 * buf, int len)
//{
//	 u16 t,sum_buff=0;
//  if(buf[0]==0x55&&buf[1]==0xaa)
//  {
//    for(t=0;t<len-1;t++)
//    {
//      sum_buff+=buf[t];
//    }
//    
//  }
//  else return 0;
//  return sum_buff%256;
//#if 0	
//	int i  = 0,sum = 0;
//	for (i = 0; i < len; i ++)
//	{
//			sum += buf[i];
//	}
//	sum =~sum;
//	sum = (sum+1) &0XFF;
//	
//	return sum;
//#endif
//}

void TX_OTA_Version_Report(u16 FrameNum, u16 Version)
{
	u8 buffer[13]= {0,};
	//MCU����	0x55AA 0x01	0xXXXX	0x06	0x02	data[0]-data[1]:�̼��汾����ʽ0001.0000.0001(bit)->1.0.1(ʮ����);	У���	
	buffer[0] = 0x55;
	buffer[1] = 0xAA;
	buffer[3] = 0x01;
	//FrameNum
	buffer[4] = (FrameNum >>8) &0xFF;
	buffer[5] = FrameNum &0xFF;
	//
	buffer[6] = 0x06;
	buffer[7] = 0x02;
	//Version
	buffer[8] = (Version >>8) &0xFF;
	buffer[9] = Version &0xFF;
	//У���
	buffer[10] = Sum_Rem(buffer,10);
	//
	Usart1_Send(buffer,11);
}

void TX_OTA_Replay_Packet(u16 FrameNum, u8 ret)
{
	u8 buffer[13]= {0,};
	//MCU����	0x55AA 0x01	0xXXXX	0x07	0x01	0x00:�ɹ�; 0x01:ʧ��;	У���
	buffer[0] = 0x55;
	buffer[1] = 0xAA;
	buffer[2] = 0x01;
	//FrameNum
	buffer[3] = (FrameNum >>8) &0xFF;
	buffer[4] = FrameNum &0xFF;
	//
	buffer[5] = 0x07;
	buffer[6] = 0x01;
	//Version
	buffer[7] = ret &0xFF;
	//У���
	buffer[8] = Sum_Rem(buffer,8);

	Usart1_Send(buffer,9);
//	printf("TX_OTA_Replay_Packet: %d\r\n", FrameNum);
}

void TX_OTA_Transfer_Replay_Packet(u16 FrameNum, u8 ret)
{
	u8 buffer[13]= {0,};
	//MCU����	0x55AA 0x01	0xXXXX	0x07	0x01	0x00:�ɹ�; 0x01:ʧ��;	У���
	buffer[0] = 0x55;
	buffer[1] = 0xAA;
	buffer[2] = 0x01;
	//FrameNum
	buffer[3] = (FrameNum >>8) &0xFF;
	buffer[4] = FrameNum &0xFF;
	//
	buffer[5] = 0x08;
	buffer[6] = 0x01;
	//Version
	buffer[7] = ret &0xFF;
	//У���
	buffer[8] = Sum_Rem(buffer,8);

	Usart1_Send(buffer,9);
//	printf("TX_OTA_Replay_Packet: %d\r\n", FrameNum);
}

void TX_OTA_Finish_Packet(u16 FrameNum, u8 status)
{
	u8 buffer[13]= {0,};
	//0x55AA 0x01	0xXXXX	0x09	0x03	data[0]: status, 0x00�ɹ���0x01ʧ�ܣ� 
	buffer[0] = 0x55;
	buffer[1] = 0xAA;
	buffer[2] = 0x01;
	//FrameNum
	buffer[3] = (FrameNum >>8) &0xFF;
	buffer[4] = FrameNum &0xFF;
	//
	buffer[5] = 0x09;
	buffer[6] = 0x03;
	//Status
	buffer[7] = status &0xFF;
	//version
	buffer[8] = (u8)(conf.New_Vesion[0]&0xff);
	buffer[9] = (u8)(conf.New_Vesion[1]&0xff);	
	//У���
	buffer[10] = Sum_Rem(buffer,10);

	Usart1_Send(buffer,11);
	//
}

void Usart1_Send(u8 *buf,u8 len)
{
	u8 t,i;
	printf("Usart1_Send: ");
	for(i = 0; i<len; i++)
		printf("%02x ", *(buf+i));
	printf("\r\n");
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
	u8 Usart1_Rec_Cnt;
  
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		  USART_ReceiveData(USART1);//��ȡ���� ע�⣺������Ҫ�������ܹ�����жϱ�־λ����Ҳ��֪��Ϊɶ��
		  Usart1_Rec_Cnt = SEND_BUF_SIZE-DMA_GetCurrDataCounter(DMA2_Stream5);	//����ӱ�֡���ݳ���
 		  USART1_length_value=Usart1_Rec_Cnt;
			//***********֡���ݴ�����************//
			//memcpy(RecvBuff, SendBuff, Usart1_Rec_Cnt);
//		Usart1_Send(SendBuff,Usart1_Rec_Cnt);
      TIM_Cmd(TIM3,ENABLE); 
			//*************************************//
			USART_ClearITPendingBit(USART1, USART_IT_IDLE);         //����жϱ�־
			MYDMA_Enable(DMA2_Stream5,SEND_BUF_SIZE);			//�ָ�DMAָ�룬�ȴ���һ�εĽ���
     } 
		
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif	
//��������USART1�����������ȡ��
//����ֵ������
u16 Sum_Rem(u8 *buff,u8 buff_length)
{
  u16 i,sum_buff=0;
  if(((buff[0]==0x55) && (buff[1]==0xaa))||((buff[1]==0x55) && (buff[0]==0xaa)))
  {
    for(i=0; i < buff_length; i++)
    {
      sum_buff = sum_buff + buff[i];
    }
  }
  else 
		return 0;
  return (sum_buff&0xff);
}
 



