#ifndef __USART1_H
#define __USART1_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

////////////////////////////////////////////////////////////////////////////////// 	
#define SEND_BUF_SIZE 255	      //发送数据长度,最好等于sizeof(TEXT_TO_SEND)+2的整数倍.	  	
#define EN_USART1_RX 1
extern u8 SendBuff[SEND_BUF_SIZE];	
extern u8 RecvBuff[SEND_BUF_SIZE];
extern u8 USART1_length_value;
void uart1_init(u32 bound);
void Usart1_Send(u8 *buf,u8 len);
u16 Sum_Rem(u8 *usart1_buff,u8 buff_length);
void TX_OTA_Version_Report(u16 FrameNum, u16 Version);
void TX_OTA_Replay_Packet(u16 FrameNum, u8 ret);
void TX_OTA_Transfer_Replay_Packet(u16 FrameNum, u8 ret);
void TX_OTA_Finish_Packet(u16 FrameNum, u8 status);
#endif


