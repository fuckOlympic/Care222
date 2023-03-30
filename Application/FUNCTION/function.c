#include "function.h"
#include "lm75adp.h"
#include "string.h"
#include "delay.h"
#include "usart.h"
#include "usart1.h"
#include "uv_lamp.h"
#include "timer.h"
#include "24cxx.h"
#include "rtc.h"
#include "pir.h"
#include "pwm.h"
#include "key.h"
#include "adc.h"
#include "stmflash.h"
#include <stdlib.h>
#include <math.h>
//#include <time.h>

/***Added by pngao@20210701***/
#include "fan_motor.h"
#include "eeprom.h"
#include "led.h"
#include "Config.h"
#include "tca9555.h"
#include <stdio.h>
/***Added end***/
//for usb support
#include "usbd_cdc_core_loopback.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"

//要写入到STM32 FLASH的字符串数组
//const u8 TEXT_Buffer[]={"CA20_MCU_FW_V0.0.0.1"};
//#define TEXT_LENTH sizeof(TEXT_Buffer)	 		  	//数组长度	
#define OTA_CONF_SIZE sizeof(conf)/4+((sizeof(conf)%4)?1:0)
//Change FLASH_VERSION from 0x08028000 to 0x08030000 for partition change
#define FLASH_VERSION    0x08030000 //0x08028000   //APP版本存放起始地址 ,设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
																			//否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.

u8 Stop_send_flag=0;   //停止发送指令状态标志位
u16 SN_count = 0;  			// Send packet Sequence Number

CONFIG_UPGRADE_CONFIG conf;		//OTA升级版本号和标志位等参数结构体变量
PRODUCT_INFO 				pro_info; //Product information structure
DEVICE_CONFIG_DATA 	dev_data; //Device config data structure

//Status and config variables definition
u8 single_double_lamp_flag=0;	//单/双灯模式切换标志位
u8 BurnIn_mode = 0;		//BurnIn mode flag
u8 Device_status=0;		//设备状态变量
u8 Device_mode = MODE_SBNS;  //设备当前模式：CMNS:0x00,CMS:0x01,UMNS:0x02,UMS:0x03,SMNS:0x04,SMS:0x05,SBNS:0x06,SBS:0x07
u16 pilot_lamp_value=100;		//Pilot lamp定时器填充值
u16 Manual_time_count=15;		//手动物理按键控制持续时间变量
u8 Warranty_start_flag = 0;
//计划变量定义
u8 Plan_buffer[60]={0};   //存放时间变量的数组
u8 Plan_length=0;   //计划的数据长度
//u8 Plan_happen=0;   //计划产生变量标志位
//u8 BLE_PLAN_tab_flag=0;//BLE模组和计划模式在定时器9切换使用标志位
u8 schedule_flag=0;//计划模式是否打开标志位
u8 Plan_running = 0; //计划运行状态
//RTC Time Get Related Variables
RTC_TimeTypeDef RTC_TimeStruct;
//u8 STOP_TIME=0;
u8 Get_time_temp_flag=0,Get_time_UV_flag=0,Get_time_BLE_flag=0,Get_time_fan_flag=0;//用在事件上报LOG上的标记位
u8 Get_time_flag=0;//计划时间标志位
//u8 STANDBY_mode_togger=0;//作用在计划模式下，如果不在设定模式内，则执行STANDBY模式变量标志位
uint16_t UV1_Life_Hours, UV2_Life_Hours;

uint16_t log_event_status = 0;
int16_t time_zone = 0;
u8 pilot_lamp_status = 0;

EVENT_DATA log_event_data[16];
uint8_t current_event_cnt = 0;
uint8_t EEPROM_Log_Nums = 0;
uint8_t log_cur_pos = 0;

uint8_t pilot_lamp_mode = P1_BT_UNPAIR;
uint8_t uv_lamp_opened = 0;
uint8_t app_config_flag = 1;
uint8_t app_config_cnt = 0;

/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
extern uint8_t AT24CXX_exist_flag;
/***Added end***/
extern uint8_t ble_connection_status;
extern uint16_t fan1_speed, fan2_speed;
extern uint16_t uvlamp_fan_status;

extern uint8_t usb_rxbuf[64];
extern __IO uint32_t receive_count;
extern USB_OTG_CORE_HANDLE USB_OTG_dev;
extern uint8_t USART1_length_value;
extern uint32_t FAN1_Frequency ;
extern uint32_t FAN2_Frequency ;

void Update_RTC_Time(time_t utc_time);



/******************************************************
*
*Function name : Search_Info
*Description 	 : BLE Communication Command Query Productor Information(0x01) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Search_Info(void)
{
  u8 str_all_info[122]={0x55,0xAA,0x01,0x00,0x01,0x01,0x72};  //帧格式
  u8 str_version[115];
	char warranty_exp_date[9];
	u8 len;
	u8 i=0,temp=7;
	/***Added by pngao@20210701 for ProductID & MCUVersion config ***/
//	printf("exp=%d\r\n", pro_info.product_warranty_exp_date);
	if(pro_info.product_warranty_exp_date == 0)
	{
		strcpy(warranty_exp_date, "00000000\0");
	}
	else
	{
		sprintf(warranty_exp_date,"%d",pro_info.product_warranty_exp_date);
	}		
	len = sprintf((char *)str_version,"{\"p\":\"%s\",\"v\":\"%s\",\"sn\":\"%s\",\"pn\":\"%s\",\"usn1\":\"%s\",\"usn2\":\"%s\",\"wed\":\"%s\"}", ProductID, MCUFirm_Version,
		pro_info.product_sn,pro_info.product_pn, pro_info.product_usn1, pro_info.product_usn2, warranty_exp_date);
	printf("strlen = %d string : %s\r\n", len, str_version);
	/***Added end***/
	str_all_info[5] = 0x01;
	str_all_info[6] = len;
	SN_count++;
	str_all_info[3] = (u8)(SN_count >>8) & 0xff; //0x00;
	str_all_info[4] = (u8) SN_count & 0xff; //SN_count;
	for(i=0;i<len;i++)
	{
		str_all_info[temp++]=str_version[i];
	}
	str_all_info[temp]=Sum_Rem(str_all_info,temp+1);
	if(SendBuff[5]==0x01)
	{
		Usart1_Send(str_all_info, temp + 1);
		Stop_send_flag=1;    //停止发送指令状态标志位
	}	
}

#if 0
void Get_BLE_Status(void)
{
	u8 str_connect_status[9]={0x55,0xAA,0x01,0x03,0x02,0x02,0x01,0x00};  //帧格式
  delay_ms(10);
  SN_count++;
  str_connect_status[3]= (u8)(SN_count >>8) & 0xff; //0x00;
  str_connect_status[4]= (u8) SN_count & 0xff; //SN_count;
	str_connect_status[8]=Sum_Rem(str_connect_status,sizeof(str_connect_status));
	while(Stop_send_flag)
	{
		delay_ms(1);
	}
	Usart1_Send(str_connect_status,sizeof(str_connect_status));
	Stop_send_flag=1; 
	while(Stop_send_flag);
	printf("Received Command：%x ", SendBuff[7]);
}
#endif

/******************************************************
*
*Function name : Connect_Status
*Description 	 : BLE Communication Command BLE Network Status report(0x02) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Connect_Status(void)
{
  u8 str_connect_status[9]={0x55,0xAA,0x01,0x03,0x02,0x02,0x01,0x01};  //帧格式
  delay_ms(10);
  SN_count++;
  str_connect_status[3]= (u8)(SN_count >>8) & 0xff; //0x00;
  str_connect_status[4]= (u8) SN_count & 0xff; //SN_count;
//	ble_connection_status = SendBuff[7];
	switch(SendBuff[7])
	{
		case 0x0:  //设备从APP端删除，UV灯关闭，Standby模式		
		{	
			printf("device is deleted from app\r\n");
			hidden_open=1;
			PLAN_Clear();
			if(manual_schedule_flag == 0)
			{
				Manual_Schedule_Mode_Set(1);
			}
			default_set();
	//  TIM_Cmd(TIM4,DISABLE); //使能定时器4//该定时器同时控制schedule的启动与终止
	//	TIM_Cmd(TIM7,DISABLE); //失能定时器3
	//	UV1=0;
	//  UV2=0;
			Device_mode=MODE_SBNS;				//Standby模式
			Device_status=1;			//UV灯关闭状态标志位
			if(dev_data.First_Connect_Flag == 1)
			{
				dev_data.First_Connect_Flag = 0;
				/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
				if(AT24CXX_exist_flag == 0)
					EEPROM_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
				else
				/***Added end***/		
					AT24CXX_WriteBytes(FIRST_CONNECT_FLAG_ADDR,&dev_data.First_Connect_Flag,1);    //AT24LC64写入数据,首次开机标记位存储
			}
			pilot_lamp_mode = P1_BT_UNPAIR;
			ble_connection_status = 0;
			break;
		}
		case 0x01:
		{
			printf("device is paired \r\n");
			/***Added by pngao@20210701 for first_connect_flag update***/
			if(dev_data.First_Connect_Flag == 0)
			{
				dev_data.First_Connect_Flag = 1;
				if(AT24CXX_exist_flag == 0)
					EEPROM_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
				else
					AT24CXX_WriteBytes(FIRST_CONNECT_FLAG_ADDR,&dev_data.First_Connect_Flag,1);    //AT24LC64写入数据,首次开机标记位存储
			}
			/***Added end***/	
			if(dev_data.Stealth_In_Standby == 0)
			{
				Device_mode = MODE_SBNS;
				pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
				turnON_GREEN();
			}
			else
			{
				Device_mode = MODE_SBS;
				pilot_lamp_mode = STEALTH_MODE;
				turn_all_off();
			}
			ble_connection_status = 1;
			app_config_flag = 0;
			app_config_cnt = 0;
			break;
		}
		case 0x02:
		case 0x03:
			printf("ble net disconnect\r\n");
			if(ble_connection_status)
				ble_connection_status = 0;
			break;
		case 0x04:
			printf("ble net connected\r\n");
			if(ble_connection_status == 0)
				ble_connection_status = 1;
			break;
		default:
			break;
	}
	str_connect_status[7]=0x01;
	str_connect_status[8]=Sum_Rem(str_connect_status,sizeof(str_connect_status));
	if(SendBuff[5]==0x02)
	{
		Usart1_Send(str_connect_status,sizeof(str_connect_status));
		Stop_send_flag=1;     //停止发送指令状态标志位
  }
	printf("Connect_Status: %d, pilot_lamp_mode: %d\r\n", ble_connection_status, pilot_lamp_mode);
}

/******************************************************
*
*Function name : BLE_Mesh
*Description 	 : BLE Communication Reset BLE Mesh Module Command(0x03) Sending. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void BLE_Mesh(uint8_t data)
{
  u8 str_BLE_Mesh[9]={0x55,0xAA,0x01,0x00,0xff,0X03,0X01,0X01}; 
	SN_count++;
  str_BLE_Mesh[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_BLE_Mesh[4]=(u8)(SN_count&0xff); //SN_count;
	str_BLE_Mesh[7]=data;
  str_BLE_Mesh[8]=Sum_Rem(str_BLE_Mesh,sizeof(str_BLE_Mesh));
	printf("BLE_Mesh : data = %d\r\n", data);
	if(data == 1)
	{
		if(dev_data.First_Connect_Flag == 1)
		{
			printf("device unpaired\r\n");
			dev_data.First_Connect_Flag = 0;
			if(AT24CXX_exist_flag == 0)
					EEPROM_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
			else
					AT24CXX_WriteBytes(FIRST_CONNECT_FLAG_ADDR,&dev_data.First_Connect_Flag,1);    //AT24LC64写入数据,首次开机标记位存储
		}
		ble_connection_status = 0;	
		if(manual_schedule_flag == 0)
		{
			Manual_Schedule_Mode_Set(1);
		}	
	}
  Usart1_Send(str_BLE_Mesh,sizeof(str_BLE_Mesh));
	Stop_send_flag=1;
 // delay_ms(500);
}

void BLE_Mesh_Response(void)
{
	if((SendBuff[5] == 0x3) && (SendBuff[7] == 0x1))
	{
		printf("BLE_Mesh Config OK\r\n");
	}
}

/******************************************************
*
*Function name : Manual_Mode
*Description 	 : BLE Communication Get local UTC Time Command(0x04) Sending. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Get_Time(void)
{
 
  u8 str_Get_Time[9]={0x55,0xAA,0x01,0x00,0xff,0X04,0X01,0X01}; 
  SN_count++;
  str_Get_Time[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Get_Time[4]=(u8)(SN_count&0xff); //SN_count;	
  str_Get_Time[8]=Sum_Rem(str_Get_Time,sizeof(str_Get_Time));
  // if(SendBuff[5]==0x04)
  Usart1_Send(str_Get_Time,sizeof(str_Get_Time));
  Stop_send_flag=1;//停止发送指令状态标志位
}


/******************************************************
*
*Function name : OTA_Ask
*Description 	 : BLE Communication Command BLE soft version request(0x06) sending
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void OTA_Ask(void)
{
	u8 str_OTA_Ask[10]={0x55,0xAA,0x01,0x00,0xff,0X06,0X02,0X00,0X01,0X00}; 
  SN_count++;
  str_OTA_Ask[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_OTA_Ask[4]=(u8)(SN_count&0xff); //SN_count;
	/*Added by pngao@20210701*/
	str_OTA_Ask[7]= (BT_SOFT_Version>>8)&0xff;
	str_OTA_Ask[8]= BT_SOFT_Version&0xff;
  str_OTA_Ask[9]=Sum_Rem(str_OTA_Ask,sizeof(str_OTA_Ask));
  if(SendBuff[5]==0x06)
	{
		Usart1_Send(str_OTA_Ask,sizeof(str_OTA_Ask));
		Stop_send_flag=1;//停止发送指令状态标志位
	}
}

/******************************************************
*
*Function name : OTA_Update_Notice
*Description 	 : BLE Communication Command MCU OTA Upgrade Notification(0x07) processing
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void OTA_Update_Notice(void)
{
//	u8 datatemp[SIZE];//版本读出存放数组
//	u8 str_OTA_Update_Notice[9]={0x55,0xAA,0x01,0x00,0xff,0X07,0X01,0X00,0X01}; 
//  SN_count++;
//  str_OTA_Update_Notice[3]=(u8)((SN_count>>8)&0xff); //0x00;
//  str_OTA_Update_Notice[4]=(u8)(SN_count&0xff); //SN_count;
	
	conf.upgrade_flag=0x55AA;
	conf.block_len=(SendBuff[7]<<24)+(SendBuff[8]<<16)+(SendBuff[9]<<8)+SendBuff[10];//包长
	conf.Old_Vesion[0]= (MCUFirm_Version_U16>>8)&0xff;
	conf.Old_Vesion[1]= MCUFirm_Version_U16 & 0xff;
	conf.New_Vesion[0]=SendBuff[11];
	conf.New_Vesion[1]=SendBuff[12];//新版本
	conf.CRC32_value=(SendBuff[13]<<24)+(SendBuff[14]<<16)+(SendBuff[15]<<8)+SendBuff[16];//CRC32校验
	STMFLASH_Write(FLASH_VERSION,(u32*)&conf, OTA_CONF_SIZE);//写入数据
//  str_OTA_Update_Notice[8]=Sum_Rem(str_OTA_Update_Notice,sizeof(str_OTA_Update_Notice));
//	if(SendBuff[5]==0x07)
//  Usart1_Send(str_OTA_Update_Notice,sizeof(str_OTA_Update_Notice));
//  Stop_send_flag=1;//停止发送指令状态标志位
	NVIC_SystemReset();//设备重启指令
}

/******************************************************
*
*Function name : Get_BLE_MAC
*Description 	 : BLE Communication Command Get BLE Module Mac address(0xA)
*Parameter		 : 
*			@mac_addr   buffer used to save returned mac address   
*Return				 : NULL
*
******************************************************/

void Get_BLE_MAC(uint8_t *mac_addr)
{
  u8 i;
	u8 str_BLE_MAC[9]={0x55,0xAA,0x01,0x00,0xff,0x0A,0x01,0x00}; 
	u8 timeout = 0;
	
	SN_count++;
  str_BLE_MAC[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_BLE_MAC[4]=(u8)(SN_count&0xff); //SN_count;
  str_BLE_MAC[8]=Sum_Rem(str_BLE_MAC,sizeof(str_BLE_MAC));
  Usart1_Send(str_BLE_MAC,sizeof(str_BLE_MAC));
	Stop_send_flag=1;
	while(timeout < 200)
	{	
		if(Stop_send_flag == 0)
			break;
		if(VCP_CheckDataReceived() != 0 )
		{
			uint8_t usb_recvlen;
			VCP_ReceiveData(&USB_OTG_dev, usb_rxbuf, usb_recvlen);
//			printf("Received %d bytes: %s\r\n", receive_count, usb_rxbuf);
			receive_count = 0;
		}
		timeout++;
		delay_ms(10);
	}

	if((timeout < 200) && SendBuff[5] == 0x0A)
	{
		for(i = 0; i<6; i++)
		{
			mac_addr[i] = SendBuff[7+i];
		}
	}
	else
	{
		memset(mac_addr, 0, 6);
	}	
 // delay_ms(500);
}

/******************************************************
*
*Function name : Get_BLE_FWVER
*Description 	 : BLE Communication Command Get BLE Module Mac address(0xA)
*Parameter		 : NULL
*Return				 : 16bit ble module firmware version
*
******************************************************/

uint16_t Get_BLE_FWVER(void)
{
//  u8 i;
	u8 str_BLE_FWVER[9]={0x55,0xAA,0x01,0x00,0xff,0x0B,0x01,0x00}; 
	u8 timeout = 0;
	uint16_t ble_fwver = 0;
	SN_count++;
  str_BLE_FWVER[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_BLE_FWVER[4]=(u8)(SN_count&0xff); //SN_count;
  str_BLE_FWVER[8]=Sum_Rem(str_BLE_FWVER,sizeof(str_BLE_FWVER));
  Usart1_Send(str_BLE_FWVER,sizeof(str_BLE_FWVER));
	Stop_send_flag=1;
	while(timeout < 200)
	{	
		if(Stop_send_flag == 0)
			break;
		if(VCP_CheckDataReceived() != 0 )
		{
			uint8_t usb_recvlen;
			VCP_ReceiveData(&USB_OTG_dev, usb_rxbuf, usb_recvlen);
//			printf("Received %d bytes: %s\r\n", receive_count, usb_rxbuf);
			receive_count = 0;
		}
		timeout++;
		delay_ms(10);
	}

	if((timeout < 200) && SendBuff[5] == 0x0B)
	{
		ble_fwver = (SendBuff[7] << 8) | SendBuff[8];
	}
	else
	{
		printf("Get ble firmware version timeout!\r\n");
	}	
	return ble_fwver;
 // delay_ms(500);
}



/******************************************************
*
*Function name : GetAll_Status
*Description 	 : BLE Communication Command Get All Status of Device(0x20) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void GetAll_Status(void)
{
	float data;
	int16_t temp_data;
	u8 str_all_status[17]={0x55,0xAA,0x01,0x00,0x20,0x20,0x09,0x00,0x00};
	
	data=0.125*(I2C_LM75read());
	temp_data=data*10/10;
	str_all_status[7]=manual_schedule_flag;//设备的Manual_Schedule mode
	str_all_status[8]=Device_mode;//设备的工作模式
	str_all_status[9]=0x0;
	str_all_status[10]=(temp_data+20)&0xff;
	str_all_status[11]=(u8)UV2_Life_read_EEPROM_value[0]&0xff;
	str_all_status[12]=(u8)UV2_Life_read_EEPROM_value[1]&0xff;
	str_all_status[13]=(u8)UV1_Life_read_EEPROM_value[0]&0xff;
	str_all_status[14]=(u8)UV1_Life_read_EEPROM_value[1]&0xff;//读取UV灯寿命，第11到14个字节
	str_all_status[15]=single_double_lamp_flag;//是否是单灯模式
	SN_count++;
	str_all_status[3]= (u8)((SN_count >> 8)&0xff); //0x00;
	str_all_status[4]= (u8) SN_count & 0xff; //SN_count;
	//INTO_delay_count1=1;
	str_all_status[16]=Sum_Rem(str_all_status,sizeof(str_all_status));
	if(SendBuff[5]==0x20)
	{
		Usart1_Send(str_all_status,sizeof(str_all_status));
		Stop_send_flag=1;     //停止发送指令状态标志位
	}
	// turnON_GREEN();
//	DEVICE_INFO.Smart_DATA=15;
//	DEVICE_INFO.Unoccupied_DATA=15;
}


#if 0
//控制UV灯开关   功能暂未使用！！！
void Switch_UV(void)
{
	u8 str_Switch_UV[9]={0x55,0xAA,0x01,0x00,0x05,0x21,0x01};  //帧格式
	SN_count++;
	str_Switch_UV[3]= (u8)(SN_count >>8) & 0xff; //0x00;
	str_Switch_UV[4]= (u8) SN_count & 0xff; //SN_count;
	if(SendBuff[7]==0x00)
	{
		if(single_double_lamp_flag==1)
		{
			open_single_lamp();
		}
		else
		{
			open_double_lamp();
		}
		str_Switch_UV[7]=0X00;
		str_Switch_UV[8]=Sum_Rem(str_Switch_UV,sizeof(str_Switch_UV));
		Usart1_Send(str_Switch_UV,sizeof(str_Switch_UV));
//      //
//       Usart1_Send(my_str,10);
//      //
	}
	else
	{
		close_uv_lamp();

		str_Switch_UV[7]=0x06;
		str_Switch_UV[8]=Sum_Rem(str_Switch_UV,sizeof(str_Switch_UV));
		Usart1_Send(str_Switch_UV,sizeof(str_Switch_UV));
	}
	TIM_Cmd(TIM7,DISABLE);
	Stop_send_flag=1;    //停止发送指令状态标志位
}
#endif

/******************************************************
*
*Function name : Device_Mode
*Description 	 : BLE Communication Command Toggle Mode(0x22) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Device_Mode(void)
{
  u8 str_Device_Mode[9]={0x55,0xAA,0x01,0x00,0x05,0x22,0x01};  //帧格式
	u8 need_openlamp = 0;
  SN_count++;
  str_Device_Mode[3]= (u8)(SN_count >>8) & 0xff; //0x00;
  str_Device_Mode[4]= (u8) SN_count & 0xff; //SN_count;
	str_Device_Mode[7] = SendBuff[7] ;
	
	str_Device_Mode[8]=Sum_Rem(str_Device_Mode,sizeof(str_Device_Mode));
	Usart1_Send(str_Device_Mode,sizeof(str_Device_Mode));
//	Plan_happen=0;//当APP端执行开关UV灯时，计划模式标志位置0，不然会触发计划模式打开
	Stop_send_flag=1;//停止发送指令状态标志位
//	printf("Device_mode = %d\r\n", Device_mode);

	if((Device_mode == str_Device_Mode[7]) && manual_schedule_flag)
	{
		printf("Same mode to set, no need to change!\r\n");
	}
	else
	{
		if((Device_mode == MODE_CMNS) || (Device_mode == MODE_CMS))
		{
			if(((str_Device_Mode[7] == MODE_CMNS )||(str_Device_Mode[7] == MODE_CMS))&& manual_schedule_flag)
			{
				Manual_count_key = 0;
			}
			else
			{
				close_uv_lamp();
				Plan_running = 0;
				need_openlamp = 1;
			}
		}
		else
		{
			if(((str_Device_Mode[7] == MODE_CMNS )||(str_Device_Mode[7] == MODE_CMS)))
			{
				need_openlamp = 1;
			}
		}
		
		switch(str_Device_Mode[7])
		{
			case MODE_CMNS:
			case MODE_CMS:					
				execute_Plan_flag=0;
				key_schedule_flag = 2;
				if(manual_schedule_flag == 0){
					Manual_Schedule_Mode_Set(1);
				}
				duty_count=0;//TIM7变量计数置0
				togger_mode=1;
				UV12_status = 0;
				if((pilot_lamp_mode == P2_BT_DEV_FAULT) && 
					(((uvlamp_fan_status &(TEMP_ERROR|FAN1_SPEED_ERROR|FAN2_SPEED_ERROR)) != 0)||
					((uvlamp_fan_status &(UV1_TURNON_FAILURE|UV2_TURNON_FAILURE))== (UV1_TURNON_FAILURE|UV2_TURNON_FAILURE))))
				{
					if((Device_mode != MODE_SBNS) && (Device_mode != MODE_SBS))
						default_set();
				}
				else
				{
					Device_mode= str_Device_Mode[7];	
					pilot_lamp_mode = P5_UV_MANUAL_ON;
					LampFlash_GREEN(2);
					if(Device_mode == MODE_CMNS)
					{
						turn_all_off();
						pilot_lamp_mode = P9_Disinfection;
						turnON_BLUE();
					}
					else
					{
						pilot_lamp_mode = STEALTH_MODE;
						turn_all_off();
					}
					TIM_Cmd(TIM7,ENABLE);
	//			TIM_Cmd(TIM4,DISABLE); //使能定时器4

					Device_status=0;   //UV灯打开状态标志位
					if(single_double_lamp_flag==1)
					{
						if(need_openlamp)
							open_single_lamp();
					}
					else if(single_double_lamp_flag == 0)
					{
						if(need_openlamp)
							open_double_lamp();
					}
					else
					{
						printf("Wrong mode set and can not turn on lamp, Please check!\r\n");
						if(dev_data.Stealth_In_Standby == 1)
						{
							Device_mode = MODE_SBS;
							if(pilot_lamp_mode != P2_BT_DEV_FAULT)
							{
								pilot_lamp_mode = STEALTH_MODE;
								turn_all_off();
							}
						}
						else
						{
							Device_mode = MODE_SBNS;
							if(pilot_lamp_mode != P2_BT_DEV_FAULT)
							{
								pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
								turnON_GREEN();
							}
						}
						TIM_Cmd(TIM7,DISABLE);
					}
				}
				break;
			case MODE_SBNS:
			case MODE_SBS:	
				//close_uv_lamp();
				Device_mode= str_Device_Mode[7];
				execute_Plan_flag=0;
				if(manual_schedule_flag == 0)
				{
					Manual_Schedule_Mode_Set(1);
				}
				TIM_Cmd(TIM7,DISABLE);
				if((Device_mode == MODE_SBNS) && (dev_data.Stealth_In_Standby == 0) )
				{
					if(pilot_lamp_mode != P2_BT_DEV_FAULT)
					{
						pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
						turnON_GREEN();
					}
				}
				else
				{
					Device_mode = MODE_SBS;
					if(pilot_lamp_mode != P2_BT_DEV_FAULT)
					{
						pilot_lamp_mode = STEALTH_MODE;
						turn_all_off();
					}
				}
				Device_status=1; //UV灯关闭状态标志位
				togger_mode=0;
				key_schedule_flag=2;
				duty_count=0;//TIM7变量计数置0
				UV12_status = 0;
	//			TIM_Cmd(TIM4,DISABLE); //失能定时器4
	//			if(BurnIn_mode == 0)
	//				printf("into TIM4 disable\r\n");
				break;
		}
	}

}

#if 0
//按键切换模式 未使用？？？
/******************************************************
*
*Function name : Device_togger_Mode
*Description 	 : BLE Communication Command Toggle Mode(0x22) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Device_togger_Mode(void)
{
  u8 str_Device_togger_Mode[9]={0x55,0xAA,0x01,0x00,0x05,0x22,0x01};  //帧格式
  SN_count++;
  str_Device_togger_Mode[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Device_togger_Mode[4]=(u8)(SN_count&0xff); //SN_count;
  if(togger_mode==1)
	{
		if(single_double_lamp_flag==1)
		{
			open_single_lamp();
		}
		else
		{
			open_double_lamp();
		}
		str_Device_togger_Mode[7]=MODE_CMNS;
		Device_status=0;   //UV灯打开状态标志位
		Device_mode = MODE_CMNS;     //continuous-Mode&Non-Stealth模式
//		BLE_PLAN_tab_flag=0;//BLE模组和计划模式在定时器4切换使用标志位
//		Plan_happen=0;//当用户APP端打开计划功能或者设备端开机重启时，计划产生，标志位置1
		str_Device_togger_Mode[8]=Sum_Rem(str_Device_togger_Mode,sizeof(str_Device_togger_Mode));
		Usart1_Send(str_Device_togger_Mode,sizeof(str_Device_togger_Mode));
//		/***Added by pngao@20210701 for eeprom emulation instead of real eeprom***/
//		if(AT24CXX_exist_flag == 0)
//			EEPROM_Read(1, Duty_cycle_read_buff, 2);
//		else
//		/***Added end***/
//			AT24CXX_Read(1,Duty_cycle_read_buff,2);//从存储器中读出Duty_cycle值
		str_duty_value= dev_data.UV_DUTY_CYCLE * 1.5; //(Duty_cycle_read_buff[0]*256+Duty_cycle_read_buff[1])*1.5;//Duty-Cycle值换算到定时器执行
		duty_count=0;//TIM7变量计数置0
		execute_Plan_flag=0;//按键切换为schedule模式/待机模式
		//togger_mode=0;
		turnON_BLUE();
		TIM_Cmd(TIM7,DISABLE);
		if(BurnIn_mode == 0)
			printf("into TIM7 enable\r\n");
	}
	else if(togger_mode==0)
	{
		default_set();
	//      UV1=0;
	//      UV2=0;
	//			TIM_Cmd(TIM7,DISABLE);
		str_Device_togger_Mode[7]=MODE_SNS;
	//				turnON_GREEN();
		Device_status=1; //UV灯关闭状态标志位
	//			Device_mode=MODE_SBNS;   //Standby模式
		str_Device_togger_Mode[8]=Sum_Rem(str_Device_togger_Mode,sizeof(str_Device_togger_Mode));
		Usart1_Send(str_Device_togger_Mode,sizeof(str_Device_togger_Mode));
		duty_count=0;//TIM7变量计数置0
	//			execute_Plan_flag=0;//按键切换为schedule模式/待机模式
		//togger_mode=1;
		if(BurnIn_mode == 0)
			printf("into TIM7 disable\r\n");
//		TIM_Cmd(TIM4,DISABLE); //失能定时器4
	}
  Stop_send_flag=1;//停止发送指令状态标志位
}

#endif


/******************************************************
*
*Function name : Unoccupied_Mode
*Description 	 : BLE Communication Command Unoccupied Mode(0x23) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Unoccupied_Mode(void)
{
	u8 str_Unoccupied_Mode[11]={0x55,0xAA,0x01,0x00,0x05,0x23,0x03};  //帧格式
	uint16_t temp_data;
	
  SN_count++;
  str_Unoccupied_Mode[3]= (u8)(SN_count >>8) & 0xff; //0x00;
  str_Unoccupied_Mode[4]= (u8) SN_count & 0xff; //SN_count;
//	DEVICE_INFO.Unoccupied_DATA= (SendBuff[7] << 8)+SendBuff[8];
	temp_data = (SendBuff[7] << 8)+SendBuff[8];
	if(dev_data.Unoccupied_PIR_Pause_Time!= temp_data)
	{
		dev_data.Unoccupied_PIR_Pause_Time = temp_data;
		//Update EEPROM data
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(UNOCCUPIED_PIR_PAUSE_TIME_ADDR, SendBuff+7, 2);
		else
			AT24CXX_WriteBytes(UNOCCUPIED_PIR_PAUSE_TIME_ADDR, SendBuff+7, 2);
	}
  str_Unoccupied_Mode[7]=SendBuff[7];
  str_Unoccupied_Mode[8]=SendBuff[8];
  str_Unoccupied_Mode[9]=SendBuff[9];
	if((dev_data.Unoccupied_Scale_Factor != SendBuff[9])&& (SendBuff[9] <= 10))
	{
		dev_data.Unoccupied_Scale_Factor = SendBuff[9];
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(UNOCCUPIED_SCALE_FACTOR_ADDR, &dev_data.Unoccupied_Scale_Factor, 1);
		else
			AT24CXX_WriteBytes(UNOCCUPIED_SCALE_FACTOR_ADDR, &dev_data.Unoccupied_Scale_Factor, 1);
	}
  str_Unoccupied_Mode[10]=Sum_Rem(str_Unoccupied_Mode,sizeof(str_Unoccupied_Mode));
  Usart1_Send(str_Unoccupied_Mode,sizeof(str_Unoccupied_Mode));
  Stop_send_flag=1;//停止发送指令状态标志位
	
}

/******************************************************
*
*Function name : Smart_Mode
*Description 	 : BLE Communication Command Smart Mode(0x24) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Smart_Mode(void)
{
  u8 str_Smart_Mode[11]={0x55,0xAA,0x01,0x00,0x05,0x24,0x03};  //帧格式
	uint8_t smart_max_on_buf[2];
	uint16_t temp_data;
	
  SN_count++;
  str_Smart_Mode[3]= (u8)(SN_count >>8) & 0xff; //0x00;
  str_Smart_Mode[4]= (u8) SN_count & 0xff; //SN_count;
//	DEVICE_INFO.Smart_DATA= (SendBuff[7] << 8)+SendBuff[8];
	temp_data = (SendBuff[7] << 8)+SendBuff[8];
	smart_max_on_buf[0] = SendBuff[7];
	smart_max_on_buf[1] = SendBuff[8];
	if(dev_data.Smart_Max_On_Time != temp_data)
	{
		dev_data.Smart_Max_On_Time = temp_data;
		//Update EEPROM data
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(SMART_MAX_ON_TIME_ADDR, smart_max_on_buf, 2);
		else
			AT24CXX_WriteBytes(SMART_MAX_ON_TIME_ADDR, smart_max_on_buf, 2);
	}
  str_Smart_Mode[7]=SendBuff[7];
  str_Smart_Mode[8]=SendBuff[8];
  str_Smart_Mode[9]=0x0a;
  
  str_Smart_Mode[9]=SendBuff[9];
	if((dev_data.Smart_Scale_Factor != SendBuff[9])&& (SendBuff[9] <= 10))
	{
		dev_data.Smart_Scale_Factor = SendBuff[9];
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(SMART_SCALE_FACTOR_ADDR, &dev_data.Smart_Scale_Factor, 1);
		else
			AT24CXX_WriteBytes(SMART_SCALE_FACTOR_ADDR, &dev_data.Smart_Scale_Factor, 1);
	}
//  str_smart_value=(str_Smart_Mode[9]%10);
  str_Smart_Mode[10]=Sum_Rem(str_Smart_Mode,sizeof(str_Smart_Mode));
  Usart1_Send(str_Smart_Mode,sizeof(str_Smart_Mode));
  Stop_send_flag=1;//停止发送指令状态标志位
}


/******************************************************
*
*Function name : Life_Time
*Description 	 : BLE Communication Command Get UV Lamp Lifetime(0x25) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Life_Time(void)
{
  u8 str_Life_Time[12]={0x55,0xAA,0x01,0x00,0xff,0X25,0X04,0X00,0X00,0X00,0X00}; 

  SN_count++;
  str_Life_Time[3]= (u8)(SN_count >>8) & 0xff; //0x00;
  str_Life_Time[4]= (u8) SN_count & 0xff; //SN_count;
	str_Life_Time[7]=UV2_Life_read_EEPROM_value[0];
	str_Life_Time[8]=UV2_Life_read_EEPROM_value[1];
	str_Life_Time[9]=UV1_Life_read_EEPROM_value[0];
	str_Life_Time[10]=UV1_Life_read_EEPROM_value[1];//读取UV灯寿命，第11到14个字节

	if(SendBuff[7]==0x01)//置UV1寿命归0
	{
		str_Life_Time[7]=0;
		str_Life_Time[8]=0;
		
		Set_UV_LAMP_Life(1, 0);
		//Set_UV_LAMP_Life(0, 0);
	}
	else if(SendBuff[7]==0x02)//置UV2寿命归0
	{
		str_Life_Time[9]=0;
	  str_Life_Time[10]=0;//
		
		Set_UV_LAMP_Life(0, 0);
		//Set_UV_LAMP_Life(1, 0);
	}
  str_Life_Time[11]=Sum_Rem(str_Life_Time,sizeof(str_Life_Time));
  if(SendBuff[5]==0x25)
	{
		Usart1_Send(str_Life_Time,sizeof(str_Life_Time));
		Stop_send_flag=1;
	}
}


/******************************************************
*
*Function name : event_reporting
*Description 	 : BLE Communication Event log report(0x26)
*Parameter		 : 
*  @event_buf     event data buffer
*  @event_number  event number
*Return				 : NULL
*
******************************************************/
uint8_t event_reporting(uint8_t *event_buf, uint8_t event_num)
{
	uint8_t *str_event_reporting;
	uint16_t  reporting_timeout;
	uint8_t i, j, upload_flag = 0;
	str_event_reporting = malloc(event_num*5+8);
	memset(str_event_reporting, 0 , event_num*5+8);
	str_event_reporting[0] = 0x55;
	str_event_reporting[1] = 0xAA;
	str_event_reporting[2] = 0x01;
	SN_count++;
	str_event_reporting[3] = (uint8_t)((SN_count>>8)&0xff);
	str_event_reporting[4] = (uint8_t)(SN_count & 0xff);
	str_event_reporting[5] = 0x26;
	str_event_reporting[6] = event_num*5;
	for(i = 0; i < event_num*5 ; i++)
	{
		str_event_reporting[7 + i] = event_buf[i];
	}
	str_event_reporting[event_num*5+7] =  Sum_Rem(str_event_reporting, event_num*5+8);
	printf("event_reporting data : ");
	for(i = 0; i< event_num*5+8; i++)
	{
		printf(" %x ", str_event_reporting[i]);
	}
	printf("\r\n");
	
	for(i = 0; i < 3; i++)
	{
		Usart1_Send(str_event_reporting, event_num*5+8);
		Stop_send_flag=1;
		reporting_timeout = 0;
		upload_flag = 0;
		while(reporting_timeout < 600)
		{
			reporting_timeout ++;
			if((Stop_send_flag == 0) && (USART1_length_value > 0))
			{
				if((SendBuff[0]==0xaa) && (SendBuff[1]==0x55) && (SendBuff[5] == 0x26))
				{
					printf("Event reporting response ok\r\n");
					upload_flag = 1;
					break;
				}
				else
				{
					printf("Received data , but not Event reporing response message\r\n");
					printf("Received %d bytes data: ", USART1_length_value);
					for(j = 0 ; j< USART1_length_value; j++)
						printf(" 0x%2x ", SendBuff[j]);
					printf("\r\n");
				}
				Stop_send_flag = 1;
			}			
			delay_ms(5);
		}
		if(upload_flag == 1)
			break;
		delay_ms(500);
	}
	free(str_event_reporting);
	if(upload_flag == 0)
	{
		printf("Event reporting response timeout\r\n");
		return 1;
	}
	else
		return 0;
	
}

/******************************************************
*
*Function name : Get_Temp
*Description 	 : BLE Communication Command Get Environment temperature(0x27) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Get_Temp(void)
{
	float data;
	u8 str_Temp[10] = {0x55,0xAA,0x01,0x00,0xff,0X27,0X02,0X00, 0x00, 0x00};
	printf("Get_Temp\r\n");
	SN_count++;
  str_Temp[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Temp[4]=(u8)(SN_count&0xff); //SN_count;
	data = 0.125*(I2C_LM75read());
	str_Temp[8] = (uint8_t)(data + 20)&0xff;
	str_Temp[9]=Sum_Rem(str_Temp,sizeof(str_Temp));
	if(SendBuff[5]==0x27)
	{
		Usart1_Send(str_Temp,sizeof(str_Temp));
	  Stop_send_flag=1;//停止发送指令状态标志位
	}	
}

/******************************************************
*
*Function name : Plan
*Description 	 : BLE Communication Command Plan(0x28) processing. 
*								 Write plan data to EEPROM and update status
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Plan(void)
{
  u16 i=0;
//	u32 time_value;
  u8 str_Plan[9]={0x55,0xAA,0x01,0x00,0xff,0X28,0X01,0X01}; 
  SN_count++;
  str_Plan[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Plan[4]=(u8)(SN_count&0xff); //SN_count;
	Plan_length=SendBuff[6];//schedule数据长度
  printf("schedule_value:");
	for(i=0; i<Plan_length; i++)
  {
		Plan_buffer[i]=SendBuff[7+i];       //保存计划数据
		printf(" 0x%02X ",Plan_buffer[i]);
  }
	printf("\r\n");

//	Plan_length_write_data[0]=Plan_length;
	/***Changed by pngao@20210701 for emulation eeprom instead of real eeprom***/
  if(AT24CXX_exist_flag == 0)
	{
		EEPROM_WriteBytes(PLAN_SIZE_ADDR , &Plan_length, 1);
		EEPROM_WriteBytes(PLAN1_ADDR, Plan_buffer, Plan_length);
//		EEPROM_Read(16,(u16*)Plan_read_buff,Plan_length);
//		EEPROM_Read(15,(u16*)Plan_length_read_data,1);
	}
	else
	{
		AT24CXX_WriteBytes(PLAN_SIZE_ADDR, &Plan_length, 1);    //AT24LC64写入数据
		delay_ms(5);
		AT24CXX_WriteBytes(PLAN1_ADDR, Plan_buffer, Plan_length);    //AT24LC64写入数据
//		delay_ms(5);
//		AT24CXX_Read(16,(u16*)Plan_read_buff,Plan_length);//从存储器中读出schedule值
//		AT24CXX_Read(15,(u16*)Plan_length_read_data,1);//从存储器中读出schedule值
	}
	/***Changed end***/
//	for(i=0;i<1;i++)
//	{
//		printf("schedule_length:%02X\r\n",Plan_length_read_data[i]);
//	}
//	for(i=0;i<Plan_length;i++)
//	{
//		printf(" schedule:%02X\r\n",Plan_read_buff[i]);
//	}
  str_Plan[8]=Sum_Rem(str_Plan,sizeof(str_Plan));
	if(SendBuff[5]==0x28)
	{
		Usart1_Send(str_Plan,sizeof(str_Plan));
	  Stop_send_flag=1;//停止发送指令状态标志位
	}
	delay_ms(100);

	str_duty_value= dev_data.UV_DUTY_CYCLE *1.5; //((Duty_cycle_read_buff[0]<<8)+Duty_cycle_read_buff[1])*1.5;//Duty-Cycle值换算到定时器执行
//	Plan_happen=1;//当用户APP端打开计划功能或者设备端开机重启时，计划产生，标志位置1
	execute_Plan_flag=1;//主函数执行计划函数开启标志位
//	BLE_PLAN_tab_flag=1;//BLE模组和计划模式在定时器4切换使用标志位
	Get_time_flag=0;//每次执行计划都是将该变量置0，以从APP端获取最新时间
//	STOP_TIME=0;
	key_schedule_flag=0;//打开计划模式

//	INTO_delay_count1=0;
// TIM_Cmd(TIM6,ENABLE); //使能定时器6，提供APP端间隔接收指令的时间
}

/******************************************************
*
*Function name : Get_Plan
*Description 	 : BLE Communication Command Get Plan(0x29) processing. 
*								 Get plan data from MCU
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/

void Get_Plan(void)
{
	u8 *str_Plan;
	u8 i, send_len;
	if(Plan_length > 0)
	{
		str_Plan = (u8 *)malloc(Plan_length+8);
		str_Plan[6] = Plan_length;
		send_len = Plan_length+8;
		for(i = 0; i< Plan_length; i++)
		{
			str_Plan[7+i] = Plan_buffer[i];
		}
	}
	else
	{
		str_Plan = (u8 *)malloc(9);
		str_Plan[6] = 1;
		str_Plan[7] = 0;
		send_len = 9;
	}
	str_Plan[0] = 0x55;
	str_Plan[1] = 0xAA;
	str_Plan[2] = 0x1;
	SN_count++;
  str_Plan[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Plan[4]=(u8)(SN_count&0xff); //SN_count;
	str_Plan[5]=0x29;
	str_Plan[send_len -1]=Sum_Rem(str_Plan, send_len);
	if(SendBuff[5]==0x29)
	{
		printf("get_Plan response ok\r\n");
		Usart1_Send(str_Plan, send_len);
	  Stop_send_flag=1;//停止发送指令状态标志位
	}
	delay_ms(5);
	free(str_Plan);
}


/******************************************************
*
*Function name : Manual_Mode
*Description 	 : BLE Communication Command Enable Manual_Mode(0x2B) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Manual_Mode(void)
{
	u8 str_Manual_Mode[9]={0x55,0xAA,0x01,0x00,0xff,0X2B,0X01,0X01}; 
  SN_count++;
  str_Manual_Mode[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Manual_Mode[4]=(u8)(SN_count&0xff); //SN_count;
	str_Manual_Mode[7]=SendBuff[7];
	if(str_Manual_Mode[7]==0x00)//关闭按键控制
	{
		schedule_flag=0;//schedule模式启用					
	}
	else if(str_Manual_Mode[7]==0x01)//打开按键控制
	{
		schedule_flag=1;//schedule模式终止
	}
	if(dev_data.Manual_Key_Enable_Flag != schedule_flag)
	{
		dev_data.Manual_Key_Enable_Flag = schedule_flag;
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(MANUAL_KEY_FLAG_ADDR, &SendBuff[7], 1);
		else
			AT24CXX_WriteBytes(MANUAL_KEY_FLAG_ADDR, &SendBuff[7],1); 
	}
//	sw3_count=0;
  str_Manual_Mode[8]=Sum_Rem(str_Manual_Mode,sizeof(str_Manual_Mode));
  if(SendBuff[5]==0x2B)
	{
		Usart1_Send(str_Manual_Mode,sizeof(str_Manual_Mode));
		Stop_send_flag=1;//停止发送指令状态标志位
	}
	printf("Manual_Mode : schedule_flag = %d\r\n", schedule_flag);
}

/******************************************************
*
*Function name : Manual_Control_delay
*Description 	 : BLE Communication Command Set Max UV on time for manual control(0x2C) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Manual_Control_delay(void)
{
	u8 str_Manual_Control_delay[10]={0x55,0xAA,0x01,0x00,0xff,0X2C,0X02,0X01}; 
  uint8_t manual_time_write[2];
	uint16_t max_manual_time;
	
	SN_count++;
  str_Manual_Control_delay[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Manual_Control_delay[4]=(u8)(SN_count&0xff); //SN_count;
	str_Manual_Control_delay[7]=SendBuff[7];
	str_Manual_Control_delay[8]=SendBuff[8];

	manual_time_write[0]=SendBuff[7];
	manual_time_write[1]=SendBuff[8];
	
	max_manual_time = (SendBuff[7] << 8) + SendBuff[8];
	if(max_manual_time != dev_data.Manual_On_Time)
	{
		/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(MAX_MANUAL_ONTIME_ADDR, manual_time_write, 2);
		else
		/***Added end***/
			AT24CXX_WriteBytes(MAX_MANUAL_ONTIME_ADDR, manual_time_write,2);    //AT24LC64写入数据,首次开机标记位存储
//		Manual_time_count = max_manual_time;
		dev_data.Manual_On_Time = max_manual_time;
	}
//	printf("Manual_time_count:%d\r\n",Manual_time_count);
  str_Manual_Control_delay[9]=Sum_Rem(str_Manual_Control_delay,sizeof(str_Manual_Control_delay));
  if(SendBuff[5]==0x2C)
	{
		Usart1_Send(str_Manual_Control_delay,sizeof(str_Manual_Control_delay));
		Stop_send_flag=1;
	}
}

/******************************************************
*
*Function name : Duty_Cycle
*Description 	 : BLE Communication Command Duty Cycle(0x2D) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Duty_Cycle(void)
{
  u8 str_Duty_Cycle[10]={0x55,0xAA,0x01,0x00,0x05,0x2D,0x02};  //帧格式
	uint16_t temp_data;
	
  SN_count++;
  str_Duty_Cycle[3]= (u8)(SN_count >>8) & 0xff; //0x00;
  str_Duty_Cycle[4]= (u8) SN_count & 0xff; //SN_count;
  str_Duty_Cycle[7]=SendBuff[7];
  str_Duty_Cycle[8]=SendBuff[8];
	
	temp_data = (str_Duty_Cycle[7] << 8)+str_Duty_Cycle[8];
	if(temp_data == 0x00)//请求Duty_cycle 值
	{
		str_Duty_Cycle[7]=(u8)((dev_data.UV_DUTY_CYCLE >> 8)&0xff);
		str_Duty_Cycle[8]=(u8)(dev_data.UV_DUTY_CYCLE &0xff);
	}
	else
	{		
		if((temp_data != dev_data.UV_DUTY_CYCLE) && (temp_data <= 0x3e8) && (temp_data >= 10))
		{
			/***Added by pngao@20210701 for emulation eeprom instead of real eeprom***/
			if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(UV_DUTY_CYCLE_ADDR, str_Duty_Cycle+7, 2);
			else
			/***Added end***/
				AT24CXX_WriteBytes(UV_DUTY_CYCLE_ADDR, str_Duty_Cycle+7,2);    //AT24LC64写入数据
			dev_data.UV_DUTY_CYCLE = temp_data;
			str_duty_value = temp_data * 1.5; //Duty-Cycle值换算
		}
		else
		{
			printf("No need to update duty_cycle, wrong value or repeated value\r\n");
		}		
//		duty_count=0;//TIM7变量计数置0
//		TIM_Cmd(TIM7,DISABLE);
	//  TIM_Cmd(TIM9,DISABLE);
	}

	str_Duty_Cycle[9]=Sum_Rem(str_Duty_Cycle,sizeof(str_Duty_Cycle));
	if(SendBuff[5]==0x2D)
		Usart1_Send(str_Duty_Cycle,sizeof(str_Duty_Cycle));
	Stop_send_flag=1;//停止发送指令状态标志位
}

/******************************************************
*
*Function name : EOL_Config
*Description 	 : BLE Communication Command EOL Config(0x2E) processing. 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void EOL_Config(void)
{
  u8 str_EOL_Config[12]={0x55,0xAA,0x01,0x00,0x05,0x2E,0x04};  //帧格式
	//uint8_t EOL_data[2];
	uint16_t temp_eol_warning, temp_eol_max;
	
  SN_count++;
  str_EOL_Config[3]= (u8)(SN_count >>8) & 0xff; //0x00;
  str_EOL_Config[4]= (u8) SN_count & 0xff; //SN_count;
  str_EOL_Config[7]=SendBuff[7];
  str_EOL_Config[8]=SendBuff[8];	
	str_EOL_Config[9]=SendBuff[9];
  str_EOL_Config[10]=SendBuff[10];
	temp_eol_max = (str_EOL_Config[7]<<8)|str_EOL_Config[8];
	temp_eol_warning = (str_EOL_Config[9]<<8)|str_EOL_Config[10];
	if(dev_data.UV_EOL_WARNING != temp_eol_warning)
	{
		if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(UV_EOL_WARNING_ADDR, str_EOL_Config+9, 2);
		else
				AT24CXX_WriteBytes(UV_EOL_WARNING_ADDR, str_EOL_Config+9,2); 
		dev_data.UV_EOL_WARNING = temp_eol_warning;
	}
	if(dev_data.UV_EOL_MAXIMUM != temp_eol_max)
	{
		if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(UV_EOL_MAXIMUM_ADDR, str_EOL_Config+7, 2);
		else
				AT24CXX_WriteBytes(UV_EOL_MAXIMUM_ADDR, str_EOL_Config+7,2); 
		dev_data.UV_EOL_MAXIMUM = temp_eol_max;
	}
	str_EOL_Config[11]=Sum_Rem(str_EOL_Config,sizeof(str_EOL_Config));
	if(SendBuff[5]==0x2E)
	{
		Usart1_Send(str_EOL_Config,sizeof(str_EOL_Config));
		Stop_send_flag = 1;
	}
	printf("EOL_Config: UV_EOL_WARNING = %d UV_EOL_MAXIMUM = %d\r\n", dev_data.UV_EOL_WARNING, dev_data.UV_EOL_MAXIMUM );
}

/******************************************************
*
*Function name : Identify_Device
*Description 	 : BLE Communication Command Open Pilot Lamp(0x2F) processing
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Identify_Device(void)
{
	u8 str_Identify_Device[9]={0x55,0xAA,0x01,0x00,0xff,0X2F,0X00,0X00,0x00}; 
  SN_count++;
  str_Identify_Device[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Identify_Device[4]=(u8)(SN_count&0xff); //SN_count;
	
	str_Identify_Device[8]=Sum_Rem(str_Identify_Device, sizeof(str_Identify_Device));
  if(SendBuff[5]==0x2F)
	{
		if(BurnIn_mode == 0)
			printf("send identify_device response\r\n");
		Usart1_Send(str_Identify_Device,sizeof(str_Identify_Device));
		Stop_send_flag=1;//停止发送指令状态标志位
	}
	
//	Identify_Device_flag=1;
//	TIM_Cmd(TIM9,ENABLE); //使能定时器9
//	while(Identify_Device_flag==1)
	LampFlash_GREEN(3);
	
}

/******************************************************
*
*Function name : Device_Init
*Description 	 : BLE Communication Command Device config init(0x30) processing
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Device_Init(void)
{
	u16 i=0;
//	u32 time_value=0;
  u8 str_Device_Init[9]={0x55,0xAA,0x01,0x00,0xff,0X30,0X01,0X01}; 
	uint8_t Device_data[32];
	time_t tick;
	u8 data_len;
	int16_t time_zone_tmp;
	uint16_t temp_uv_duty_cycle = 0;
	
  SN_count++;
  str_Device_Init[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Device_Init[4]=(u8)(SN_count&0xff); //SN_count;
	printf("Device_Init data: ");
	data_len = SendBuff[6];
	for(i=0; i<SendBuff[6]; i++)
  {
    Device_data[i] = SendBuff[7+i];
		printf(" 0x%02X ", Device_data[i]);
  }
	printf("\r\n");
	//update config data to dev_data structure
	dev_data.Unoccupied_PIR_Pause_Time = (Device_data[0] << 8)+Device_data[1];
	dev_data.Unoccupied_Scale_Factor = Device_data[2];
	dev_data.Smart_Max_On_Time = (Device_data[3] << 8) +Device_data[4];
	dev_data.Smart_Scale_Factor = Device_data[5];
	dev_data.PIR_Function_Flag = Device_data[6];
	
	dev_data.Manual_Key_Enable_Flag = Device_data[7];
	if(Device_data[7]==0x00)//手控按钮使能设置
	{
		schedule_flag=0;//schedule模式启用
	}
	else
	{
		schedule_flag=1;//schedule模式终止
	}
	//Manual_time_count= (Device_data[8] << 8) + Device_data[9];
	dev_data.Manual_On_Time = (Device_data[8] << 8) + Device_data[9];
	dev_data.UV_EOL_MAXIMUM = (Device_data[10] << 8) + Device_data[11];
	dev_data.UV_EOL_WARNING = (Device_data[12] << 8) + Device_data[13];
		
	dev_data.Pilot_Lamp_Brightness = Device_data[18];
	pilot_lamp_value=450-Device_data[18]*4;//将APP端下发的指示灯亮度指令赋给变量保存
	dev_data.Stealth_In_Standby = Device_data[19];
	
	if(data_len == 24)
	{
		temp_uv_duty_cycle = (Device_data[22] << 8) | Device_data[23];
		printf("Set uv duty cycle : %d\r\n", temp_uv_duty_cycle);
	}
//	dev_data.First_Connect_Flag =1;//连接到APP成功，标记为1并存储
	//Update config data to EEPROM
	if(AT24CXX_exist_flag == 0)
	{
		EEPROM_WriteBytes(UNOCCUPIED_PIR_PAUSE_TIME_ADDR, Device_data, 10);
		EEPROM_WriteBytes(UV_EOL_MAXIMUM_ADDR, Device_data+10, 4);
		EEPROM_WriteBytes(PILOT_LAMP_BRIGHTNESS_ADDR, Device_data+18, 2);
		EEPROM_WriteBytes(STEALTH_MODE_FLAG_ADDR, &dev_data.Stealth_In_Standby, 1);
		if((temp_uv_duty_cycle >= 0xa) && (temp_uv_duty_cycle <= 0x3e8))
		{
			EEPROM_WriteBytes(UV_DUTY_CYCLE_ADDR, &Device_data[22], 2);
			dev_data.UV_DUTY_CYCLE = temp_uv_duty_cycle;
			str_duty_value = temp_uv_duty_cycle * 1.5;
		}
//		EEPROM_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
	}
	else
	{
		AT24CXX_WriteBytes(UNOCCUPIED_PIR_PAUSE_TIME_ADDR, Device_data, 10);
		AT24CXX_WriteBytes(UV_EOL_MAXIMUM_ADDR, Device_data+10, 2);	
		AT24CXX_WriteBytes(PILOT_LAMP_BRIGHTNESS_ADDR, Device_data+18, 2);	
		AT24CXX_WriteBytes(STEALTH_MODE_FLAG_ADDR, &dev_data.Stealth_In_Standby, 1);
		if((temp_uv_duty_cycle >= 0xa) && (temp_uv_duty_cycle <= 0x3e8))
		{
			AT24CXX_WriteBytes(UV_DUTY_CYCLE_ADDR, &Device_data[22],2);    //AT24LC64写入数据
			dev_data.UV_DUTY_CYCLE = temp_uv_duty_cycle;
			str_duty_value = temp_uv_duty_cycle * 1.5;
		}
//		AT24CXX_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);    //AT24LC64写入数据,首次开机标记位存储
	}
	/***Changed end***/
	//Update RTC Time
	tick = (Device_data[14]<<24) + (Device_data[15] << 16) + (Device_data[16] << 8) + Device_data[17];
	if(data_len >= 22)
	{
		if(Device_data[20] == 0x1)
			time_zone_tmp = -Device_data[21];
		else
			time_zone_tmp = Device_data[21];
		printf("Time Zone = %d\r\n", time_zone_tmp);
		if(time_zone_tmp != time_zone)
		{
			uint8_t time_zone_buf[2];
			time_zone = time_zone_tmp;
			time_zone_buf[0] = time_zone >> 8 & 0xff;
			time_zone_buf[1] = time_zone &0xff;
			printf("Update Time Zone to %d\r\n", time_zone);
			if(AT24CXX_exist_flag)
			{
				AT24CXX_WriteBytes(TIME_ZONE_ADDR, time_zone_buf, 2);
			}
			else
			{
				EEPROM_WriteBytes(TIME_ZONE_ADDR, time_zone_buf, 2);
			}
		}
	}
	Update_RTC_Time(tick);

  str_Device_Init[8]=Sum_Rem(str_Device_Init,sizeof(str_Device_Init));
  if(SendBuff[5]==0x30)
	{
		Usart1_Send(str_Device_Init,sizeof(str_Device_Init));
		Stop_send_flag=1;//停止发送指令状态标志位
	}
}

/******************************************************
*
*Function name : Pilot_Lamp_brightness
*Description 	 : BLE Communication Command Adjust Pilot Lamp Brightness(0x31) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Pilot_Lamp_brightness(void)
{
	u8 str_Pilot_Lamp_brightness[9]={0x55,0xAA,0x01,0x00,0xff,0X31,0X01,0X00,0X00}; 
	uint8_t led_brightness;
  SN_count++;
	str_Pilot_Lamp_brightness[7]=SendBuff[7];	
	led_brightness = SendBuff[7];
	if((led_brightness <= 100) && ((led_brightness != dev_data.Pilot_Lamp_Brightness)))
	{
		//Update related variables
		dev_data.Pilot_Lamp_Brightness = led_brightness;
		pilot_lamp_value = 450 - led_brightness*4;
		printf("Set Pilot_Lamp_brightness=%d, pilot_lamp_value=%d \r\n", dev_data.Pilot_Lamp_Brightness, pilot_lamp_value);
		//Save new Pilot lamp brightness Setting to EEPROM
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(PILOT_LAMP_BRIGHTNESS_ADDR, &led_brightness, 1);
		else
			AT24CXX_WriteBytes(PILOT_LAMP_BRIGHTNESS_ADDR, &led_brightness, 1);
	}
	else
	{
		printf("Pilot_Lamp_brightness: Wrong value or repeated value , No need to update!");
//		pilot_lamp_value=50;
		str_Pilot_Lamp_brightness[7] = dev_data.Pilot_Lamp_Brightness;
	}

  str_Pilot_Lamp_brightness[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Pilot_Lamp_brightness[4]=(u8)(SN_count&0xff); //SN_count;
  str_Pilot_Lamp_brightness[8]=Sum_Rem(str_Pilot_Lamp_brightness,sizeof(str_Pilot_Lamp_brightness));
	if(Device_mode == MODE_SBNS)
	{
		turn_all_off();
		turnON_GREEN();
	}
	else if(Device_mode == MODE_CMNS)
	{
		turn_all_off();
		turnON_BLUE();
	}
	
	
  if(SendBuff[5]==0x31)
	{
		Usart1_Send(str_Pilot_Lamp_brightness,sizeof(str_Pilot_Lamp_brightness));
		Stop_send_flag=1;
	}
}

/******************************************************
*
*Function name : Resume_schedule
*Description 	 : BLE Communication Command Resume Schedule(0x32) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Resume_schedule(void)
{
	u8 str_Resume_schedule[9]={0x55,0xAA,0x01,0x00,0xff,0X32,0X01,0X00,0X00}; 
  SN_count++;
  str_Resume_schedule[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Resume_schedule[4]=(u8)(SN_count&0xff); //SN_count;
  str_Resume_schedule[8]=Sum_Rem(str_Resume_schedule,sizeof(str_Resume_schedule));
	if(SendBuff[7]==0x00)
	{
		if(manual_schedule_flag == 1)
			schedule_mode_run();
		else
			printf("Device is already in schedule mode");
	}
  if(SendBuff[5]==0x32)
	{
		Usart1_Send(str_Resume_schedule,sizeof(str_Resume_schedule));
		Stop_send_flag=1;
	}
}

/******************************************************
*
*Function name : single_lamp
*Description 	 : BLE Communication Command Query Single Lamp Mode(0x33) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void single_lamp(void)
{
	u8 str_single_lamp[9]={0x55,0xAA,0x01,0x00,0xff,0X33,0X01,0X00,0X00}; 
  SN_count++;
  str_single_lamp[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_single_lamp[4]=(u8)(SN_count&0xff); //SN_count;
	str_single_lamp[7]=single_double_lamp_flag;
	str_single_lamp[8]=Sum_Rem(str_single_lamp,sizeof(str_single_lamp));
  if(SendBuff[5]==0x33)
	{
		Usart1_Send(str_single_lamp,sizeof(str_single_lamp));
		Stop_send_flag=1;
	}
}

/******************************************************
*
*Function name : Stealth_In_Standby
*Description 	 : BLE Communication Command Stealth mode in standby (0x34) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Stealth_Mode_In_Standby(void)
{
	u8 str_Stealth_mode[9] = {0x55,0xAA,0x01,0x00,0xff,0X34,0X01,0X00,0X00};
	SN_count++;
  str_Stealth_mode[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_Stealth_mode[4]=(u8)(SN_count&0xff); //SN_count;
	str_Stealth_mode[7]=SendBuff[7];
	if(dev_data.Stealth_In_Standby != str_Stealth_mode[7])
	{
		dev_data.Stealth_In_Standby = str_Stealth_mode[7];
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(STEALTH_MODE_FLAG_ADDR, &str_Stealth_mode[7], 1);
		else
			AT24CXX_WriteBytes(STEALTH_MODE_FLAG_ADDR, &str_Stealth_mode[7], 1);
		if((Device_mode == MODE_SBNS) && dev_data.Stealth_In_Standby)
		{
			Device_mode = MODE_SBS;
			if(pilot_lamp_mode != P2_BT_DEV_FAULT)
			{
				pilot_lamp_mode = STEALTH_MODE;
				turn_all_off();
			}
		}
		else if((Device_mode == MODE_SBS) && (dev_data.Stealth_In_Standby == 0))
		{
			Device_mode = MODE_SBNS;
			if(pilot_lamp_mode != P2_BT_DEV_FAULT)
			{
				pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
				turnON_GREEN();
			}
		}
	}
	if(SendBuff[5]==0x34)
	{
		Usart1_Send(str_Stealth_mode, sizeof(str_Stealth_mode));
		Stop_send_flag=1;
	}
}

/******************************************************
*
*Function name : Query_Registration_Tag
*Description 	 : BLE Communication Command Query Registration Flag(0xA0) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Query_Registration_Tag(void)
{
	u8 str_Query_Registration_Tag[9] = {0x55,0xAA,0x01,0x00,0x00,0xA0,0x01,0x00,0x00}; 
  SN_count++;
  str_Query_Registration_Tag[3] = (u8)((SN_count>>8)&0xff); //0x00;
  str_Query_Registration_Tag[4] = (u8)(SN_count&0xff); //SN_count;
	str_Query_Registration_Tag[7] = pro_info.product_registration_flag;
  str_Query_Registration_Tag[8] = Sum_Rem(str_Query_Registration_Tag, sizeof(str_Query_Registration_Tag));

  if(SendBuff[5]==0xA0)
		Usart1_Send(str_Query_Registration_Tag, sizeof(str_Query_Registration_Tag));
  Stop_send_flag=1;
}

/******************************************************
*
*Function name : Query_Warranty_StartDate
*Description 	 : BLE Communication Command Query Warranty Start Date(0xA1) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Query_Warranty_StartDate(void)
{
	u8 str_Query_Warranty_Period[12] = {0x55,0xAA,0x01,0x00,0x00,0xA1,0x04,0x00,0x00,0x00,0x00,0x00}; 
	struct tm stmT;
	uint32_t wars_date;
	uint32_t wars_utc;
	memset(&stmT, 0 , sizeof(stmT));
	if(pro_info.product_warranty_start_date == 0)
		wars_date = DEFAULT_WARS_DATE;
	else
		wars_date = pro_info.product_warranty_start_date;
	printf("Test: %d %d %d\r\n", pro_info.product_warranty_start_date/10000, pro_info.product_warranty_start_date%10000, pro_info.product_warranty_start_date%1000000);
	stmT.tm_year = (wars_date/10000 ) - 1900;
	stmT.tm_mon = (wars_date%10000)/100-1;  
	stmT.tm_mday=	(wars_date%100);  
	stmT.tm_hour=0;  
	stmT.tm_min=0;  
	stmT.tm_sec=0; 
	wars_utc = mktime(&stmT) - 3600*time_zone;
//	if(BurnIn_mode == 0)	
	printf("WARS Time is : %d Year=%d Mon=%d Day=%d\r\n", wars_date, stmT.tm_year, stmT.tm_mon, stmT.tm_mday);
  SN_count++;
  str_Query_Warranty_Period[3] = (u8)((SN_count>>8)&0xff); //0x00;
  str_Query_Warranty_Period[4] = (u8)(SN_count&0xff); //SN_count;
	//Calc warranty period utc time
	str_Query_Warranty_Period[7] = (u8)((wars_utc >> 24)&0xff);
	str_Query_Warranty_Period[8] = (u8)((wars_utc >> 16)&0xff);
	str_Query_Warranty_Period[9] = (u8)((wars_utc >> 8)&0xff);
	str_Query_Warranty_Period[10] = (u8)(wars_utc&0xff);
  str_Query_Warranty_Period[11] = Sum_Rem(str_Query_Warranty_Period, sizeof(str_Query_Warranty_Period));
	
  if(SendBuff[5]==0xA1)
		Usart1_Send(str_Query_Warranty_Period, sizeof(str_Query_Warranty_Period));
  Stop_send_flag=1;
}

/******************************************************
*
*Function name : Set_Warranty_Period
*Description 	 : BLE Communication Command Write Warranty EXP Date(0xA2) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Set_Warranty_Period(void)
{
	u8 str_Set_Warranty_Period[9] = {0x55,0xAA,0x01,0x00,0x00,0xA2,0x01,0x00,0x00}; 
	RTC_DateTypeDef sdate, warranty_date;
	uint8_t temp_buf[10] = {0}, exp_data[4];
	uint32_t wdate;
  SN_count++;
  str_Set_Warranty_Period[3] = (u8)((SN_count>>8)&0xff); //0x00;
  str_Set_Warranty_Period[4] = (u8)(SN_count&0xff); //SN_count;

	RTC_GetDate(RTC_Format_BIN, &sdate);
	memcpy(temp_buf, &SendBuff[7], 8);
	wdate = atoi((char *)temp_buf);
	warranty_date.RTC_Year = wdate/10000 - 2000;
	warranty_date.RTC_Month = (wdate%10000)/100;
	warranty_date.RTC_Date = wdate%100;
	if(BurnIn_mode == 0)
		printf("Set Warranty_date to Year:%d Mon:%d Day:%d\r\n",warranty_date.RTC_Year, warranty_date.RTC_Month, warranty_date.RTC_Date );
	if((warranty_date.RTC_Year > sdate.RTC_Year)||
		((warranty_date.RTC_Year == sdate.RTC_Year)&&(warranty_date.RTC_Month > sdate.RTC_Month))||
		((warranty_date.RTC_Year == sdate.RTC_Year)&&(warranty_date.RTC_Month > sdate.RTC_Month)&&(warranty_date.RTC_Date > sdate.RTC_Date)))
	{
		str_Set_Warranty_Period[7] = 0x00;
		pro_info.product_warranty_exp_date = wdate;
		exp_data[0] = (wdate >> 24) & 0xff;
		exp_data[1] = (wdate >> 16) & 0xff;
		exp_data[2] = (wdate >> 8) & 0xff;
		exp_data[3] =  wdate & 0xff;
		pro_info.product_registration_flag = 1;
		//Update EEPROM data
		if(AT24CXX_exist_flag == 0)
		{
			EEPROM_WriteBytes(PRODUCT_WARRANTY_EXP_DATE_ADDR, exp_data, 4);
			EEPROM_WriteBytes(PRODUCT_REGISTERED_FLAG_ADDR, &pro_info.product_registration_flag, 1);
		}
		else
		{
			AT24CXX_WriteBytes(PRODUCT_WARRANTY_EXP_DATE_ADDR, exp_data, 4);
			AT24CXX_WriteBytes(PRODUCT_REGISTERED_FLAG_ADDR, &pro_info.product_registration_flag, 1);
		}
	}
	else
	{
		if(BurnIn_mode == 0)
			printf("Wrong time setting\r\n");
		str_Set_Warranty_Period[7] = 0x01;
	}

  str_Set_Warranty_Period[8] = Sum_Rem(str_Set_Warranty_Period, sizeof(str_Set_Warranty_Period));
	
  if(SendBuff[5]==0xA2)
		Usart1_Send(str_Set_Warranty_Period, sizeof(str_Set_Warranty_Period));
  Stop_send_flag=1;
}

/******************************************************
*
*Function name : BurnIn_Mode
*Description 	 : BLE Communication Command Burn-IN(0xE0) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/

void BurnIn_Mode(void)
{
	u8 str_BurnIn_Mode[9] = {0x55,0xAA,0x01,0x00,0x00,0xE0,0x01,0x00,0x00}; 
  SN_count++;
  str_BurnIn_Mode[3] = (u8)((SN_count>>8)&0xff); //0x00;
  str_BurnIn_Mode[4] = (u8)(SN_count&0xff); //SN_count;
	if(SendBuff[7] == 0x0)
		BurnIn_mode = 0x0;
	else if(SendBuff[7] == 0x1)
		BurnIn_mode = 0x1;
	str_BurnIn_Mode[7] = BurnIn_mode;
	str_BurnIn_Mode[8] = Sum_Rem(str_BurnIn_Mode, sizeof(str_BurnIn_Mode));
	
  if(SendBuff[5]==0xE0)
		Usart1_Send(str_BurnIn_Mode, sizeof(str_BurnIn_Mode));
  Stop_send_flag=1;
}


/******************************************************
*
*Function name : execute_Plan
*Description 	 : execute Plan in schedule
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void execute_Plan(void)
{
	u8 i=0;
//	u8 tbuf[40];
//	u32 RTC_time_value=0;//RTC时钟数值变量
	u8 RTC_time[3]={0};
	u16 current_minutes;
	u8 time_in_plan = 0, plan_num;

	RTC_time[0] = RTC_TimeStruct.RTC_Hours;
	RTC_time[1] = RTC_TimeStruct.RTC_Minutes;
	RTC_time[2] = RTC_TimeStruct.RTC_Seconds;
	current_minutes = RTC_time[0] * 60 + RTC_time[1];
	printf("Enter execute_Plan @time-%d:%d:%d\r\n",RTC_time[0], RTC_time[1], RTC_time[2]);
	for(i=0; i<Plan_length/6; i++)
	{
		if(Plan_buffer[i*6]==0x00)
		{
			//if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2] <= current_minutes) && (Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5] > current_minutes))
			if((current_minutes >= Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]) && ( current_minutes < Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]))
			{
				plan_num = i;
				time_in_plan = 1;
				break;
			}
		}
	}
		
//		TIM_Cmd(TIM4,ENABLE); //使能定时器4
	printf("uvlamp_fan_status = %x log_event_status = %x\r\n",uvlamp_fan_status, log_event_status);
	if(time_in_plan  //&&((log_event_status & (1<<Ambient_temperature_Error)) == 0)
		&& ((uvlamp_fan_status & (UV1_TURNON_FAILURE|UV2_TURNON_FAILURE)) !=3)
		&& ((uvlamp_fan_status & (FAN1_SPEED_ERROR | FAN2_SPEED_ERROR | TEMP_ERROR)) == 0))
	{
		if(Plan_running == 0)
		{
			printf("New shedule start\r\n");
			Plan_running = 1;
			UV12_status = 0;
			duty_count = 0;
			TIM_Cmd(TIM7,ENABLE);   //打开定时器3，Duty-CYCLE功能启动
			Device_mode = Plan_buffer[plan_num*6+3];
//			STANDBY_mode_togger=0;
			if( Device_mode == MODE_CMNS)
			{
				if(pilot_lamp_mode != P2_BT_DEV_FAULT)
				{
					turnON_BLUE();
					pilot_lamp_mode = P9_Disinfection;
					hidden_open=0;					//隐身功能关闭
				}
			}
			else if(Device_mode == MODE_CMS)
			{
				if(pilot_lamp_mode != P2_BT_DEV_FAULT)
				{
					turn_all_off();
					pilot_lamp_mode = STEALTH_MODE;
					hidden_open=1;//隐身功能开启
				}
			}
			
			if((single_double_lamp_flag == 1))
			{
				open_single_lamp();
			}
			else
			{
				open_double_lamp();
			}
		}
		else
		{
			printf("Shedule is running :%d,%d\r\n",duty_count,str_duty_value);
		}				
	}	
	else
	{
		if(Plan_running == 1)
		{
			printf("Current shedule is over or some error occured\r\n");
			Plan_running = 0;
				//Close UV Lamp
//			if(UV12_status == 1)
			close_uv_lamp();
				
			if(dev_data.Stealth_In_Standby == 0)
			{
				//Change mode to SNS(Standby & Non-Stealth) 
				Device_mode= MODE_SBNS;
				if(pilot_lamp_mode != P2_BT_DEV_FAULT)
				{
					turnON_GREEN();
					pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
				}
			}
			else
			{
				Device_mode = MODE_SBS;
				if(pilot_lamp_mode != P2_BT_DEV_FAULT)
				{
					turn_all_off();
					pilot_lamp_mode = STEALTH_MODE;
				}
			}
			if(BurnIn_mode == 0)
				printf("UV12 is close\r\n");				
			TIM_Cmd(TIM7,DISABLE);   //关闭定时器3，Duty-CYCLE功能关闭
		}
	}
//	}//Plan_happen=0;//执行计划功能取消标志位
}
		

/******************************************************
*
*Function name : temp_Item_Update
*Description 	 : BLE Communication Ambient temperature warning(0x00) and Ambient temperature error(0x01) log report
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void temp_Item_Update(void)
{
	u8 str_Item_Update[13]={0x55,0xAA,0x01,0x00,0xff,0X26,0X05,0X00,0X00,0X00,0X00,0X00,0X00}; 
	time_t rtc_time;
//	u8 i;
	float data;//温度数据
  int16_t temp_data;//转换后的实际温度值
	data=0.125*(I2C_LM75read());
	temp_data=data*10/10;//实际温度值
	if(temp_data>= dev_data.High_Temp_Thresh_Warning)
	{
		if(temp_data > dev_data.High_Temp_Thresh_ERROR)
		{
			str_Item_Update[7] = Ambient_temperature_Error;
		}
		else
			str_Item_Update[7] = Ambient_Temperature_Warning;
		rtc_time = RTC_To_UTC();
		SN_count++;
		str_Item_Update[8]=(u8)((rtc_time >>24)&0xff);
		str_Item_Update[9]=(u8)((rtc_time >>16)&0xff);
		str_Item_Update[10]=(u8)((rtc_time >>8)&0xff);
		str_Item_Update[11]=(u8)(rtc_time&0xff);//标准unix时间戳赋值
		str_Item_Update[3]=(u8)((SN_count>>8)&0xff); //0x00;
		str_Item_Update[4]=(u8)(SN_count&0xff); //SN_count;
		str_Item_Update[12]=Sum_Rem(str_Item_Update, sizeof(str_Item_Update));
		Usart1_Send(str_Item_Update,sizeof(str_Item_Update));
		if(BurnIn_mode == 0)
			printf("temp:%d\r\n",temp_data);	
		Stop_send_flag=1;//停止发送指令状态标志位
	}
}

//风扇事件LOG上传
//0x02:Fans speed warning，风扇转速警告
//0x03:Fans speed  error ，风扇转速报错
/******************************************************
*
*Function name : fan_Item_Update
*Description 	 : BLE Communication Fans speed warning(0x02) and Fans speed  error(0x03) log report
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void fan_Item_Update(void)
{
//	u16 adcx,adcx1;
	u8 str_fan_Item_Update[13]={0x55,0xAA,0x01,0x00,0xff,0X26,0X05,0X02,0X00,0X00,0X00,0X00,0X00}; 
	time_t rtc_time;
	
	if((fan1_speed < dev_data.FAN_Speed_Thresh_Warning) || (fan2_speed < dev_data.FAN_Speed_Thresh_Warning))
	{		
		printf("fan1 speed = %d fan2 speed = %d\r\n", fan1_speed, fan2_speed);
		rtc_time = RTC_To_UTC();
		SN_count++;
		str_fan_Item_Update[8]=(u8)((rtc_time >>24)&0xff);
		str_fan_Item_Update[9]=(u8)((rtc_time >>16)&0xff);
		str_fan_Item_Update[10]=(u8)((rtc_time >>8)&0xff);
		str_fan_Item_Update[11]=(u8)(rtc_time&0xff);//标准unix时间戳赋值
		str_fan_Item_Update[3]=(u8)((SN_count>>8)&0xff); //0x00;
		str_fan_Item_Update[4]=(u8)(SN_count&0xff); //SN_count;
		str_fan_Item_Update[12]=Sum_Rem(str_fan_Item_Update,sizeof(str_fan_Item_Update));
		Usart1_Send(str_fan_Item_Update,sizeof(str_fan_Item_Update));
//		if(BurnIn_mode == 0)
//			printf("Get time successful\r\n");
		printf("FAN error Event\r\n");
		Stop_send_flag=1;//停止发送指令状态标志位
	}
}

//蓝牙复位事件LOG上传
//0x04:BLE module reset，蓝牙模块重置(复位)
//0x06:BLE module connection is lost (dead)，蓝牙模块连接丢失
//0x07:BLE module reconnected，蓝牙模块重连
/******************************************************
*
*Function name : BLErst_Item_Update
*Description 	 : BLE Communication BLE module reset(0x04), BLE module connection is lost (dead)(0x06) 
*								 and BLE module reconnected(0x07) log report
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void BLErst_Item_Update(void)
{
	u8 str_BLErst_Item_Update[13]={0x55,0xAA,0x01,0x00,0xff,0X26,0X05,0X04,0X00,0X00,0X00,0X00,0X00}; 
	time_t rtc_time;
  if(BLErst_flag==1)  //蓝牙复位事件
	{
		rtc_time = RTC_To_UTC();
		SN_count++;
		str_BLErst_Item_Update[3]=(u8)((SN_count>>8)&0xff); //0x00;
		str_BLErst_Item_Update[4]=(u8)(SN_count&0xff); //SN_count;
		str_BLErst_Item_Update[7]=BLE_Module_Reset;
		str_BLErst_Item_Update[8]=(u8)((rtc_time >>24)&0xff);
		str_BLErst_Item_Update[9]=(u8)((rtc_time >>16)&0xff);
		str_BLErst_Item_Update[10]=(u8)((rtc_time >>8)&0xff);
		str_BLErst_Item_Update[11]=(u8)(rtc_time & 0xff);//标准unix时间戳赋值
		str_BLErst_Item_Update[12]=Sum_Rem(str_BLErst_Item_Update,sizeof(str_BLErst_Item_Update));
//		if(SendBuff[5]!=0x04)//判断蓝牙是否在线？
//		{
//			str_BLErst_Item_Update[7]=0x06;
//			Usart1_Send(str_BLErst_Item_Update,sizeof(str_BLErst_Item_Update));
//		}			
//			delay_ms(100);
		Usart1_Send(str_BLErst_Item_Update,sizeof(str_BLErst_Item_Update));
		BLErst_flag=0;
		printf("BLE Reset Event\r\n");
		Stop_send_flag=1;//停止发送指令状态标志位
	}
}

//0x05:开灯失败事件LOG上传
//0x08:Manual Turn on by reset button，物理按键开灯
//0x09:Manual Turn off by reset button，物理按键关灯
/******************************************************
*
*Function name : UV_Item_Update
*Description 	 : BLE Communication Open Lamp Failure(0x05), Manual Turn on by reset button(0x08) 
*								 and Manual Turn off by reset button(0x09) log report
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void UV_Item_Update(void)
{
	u8 str_UV_Item_Update[13]={0x55,0xAA,0x01,0x00,0xff,0X26,0X05,0X00,0X00,0X00,0X00,0X00,0X00}; 
	time_t rtc_time;
	if((Device_mode==MODE_CMNS)||(Device_mode==MODE_CMS))//UV灯开启模式conuitunes mode
	{
		printf("single_double_lamp_flag=%d LAMP_ON_B=%d LAMP_ON_A=%d\r\n", single_double_lamp_flag, LAMP_ON_B, LAMP_ON_A);
		if((single_double_lamp_flag && LAMP_ON_B && LAMP_ON_A)||((single_double_lamp_flag==0)&&(LAMP_ON_A||LAMP_ON_B)))
		{
			rtc_time = RTC_To_UTC();	
			SN_count++;
			str_UV_Item_Update[7]=Lamp_TurnOn_Failure;
			str_UV_Item_Update[8]=(u8)((rtc_time >>24)&0xff);
			str_UV_Item_Update[9]=(u8)((rtc_time >>16)&0xff);
			str_UV_Item_Update[10]=(u8)((rtc_time >>8)&0xff);
			str_UV_Item_Update[11]=(u8)(rtc_time&0xff);
			str_UV_Item_Update[3]=(u8)((SN_count>>8)&0xff); //0x00;
			str_UV_Item_Update[4]=(u8)(SN_count&0xff); //SN_count;			
			str_UV_Item_Update[12]=Sum_Rem(str_UV_Item_Update,sizeof(str_UV_Item_Update));
			Usart1_Send(str_UV_Item_Update,sizeof(str_UV_Item_Update));
			if(BurnIn_mode == 0)
				printf("UV error\r\n");			
			log_event_status	|= 1 << Lamp_TurnOn_Failure;		
			Stop_send_flag=1;
		}
	}

}

/******************************************************
*
*Function name : life_Item_update
*Description 	 : BLE Communication Lamp lifetime warning(0x0a) and Lamp lifetime error(0x0b) log report
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void life_Item_update(void)
{
	u8 str_life_Item_Update[13]={0x55,0xAA,0x01,0x00,0xff,0X26,0X05,0X00,0X00,0X00,0X00,0X00,0X00}; 
	time_t	rtc_time;

	if((UV1_Life_Hours > dev_data.UV_EOL_WARNING) || (UV2_Life_Hours > dev_data.UV_EOL_WARNING) )
	{
		if((UV1_Life_Hours > dev_data.UV_EOL_MAXIMUM) || (UV2_Life_Hours > dev_data.UV_EOL_MAXIMUM))
		{
			str_life_Item_Update[7] = Lamp_Lifetime_Error;
		}
		else
		{
			str_life_Item_Update[7] = Lamp_Lifetime_Warning;
		}
		SN_count++;
		rtc_time = RTC_To_UTC();
		str_life_Item_Update[3]=(u8)((SN_count>>8)&0xff); //0x00;
		str_life_Item_Update[4]=(u8)(SN_count&0xff); //SN_count;
		str_life_Item_Update[8]=(u8)((rtc_time >>24)&0xff);
		str_life_Item_Update[9]=(u8)((rtc_time >>16)&0xff);
		str_life_Item_Update[10]=(u8)((rtc_time >>8)&0xff);
		str_life_Item_Update[11]=(u8)(rtc_time&0xff);
		str_life_Item_Update[12]=Sum_Rem(str_life_Item_Update,sizeof(str_life_Item_Update));
		Usart1_Send(str_life_Item_Update,sizeof(str_life_Item_Update));
		Stop_send_flag=1;
	}
}


/******************************************************
*
*Function name : GET_time_update
*Description 	 : Get utc time from app and update local RTC time
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void GET_time_update(void)
{
#if 0
	time_t tick;

//	u8 standby_time=0;
	if(INTO_delay_count1==1)
	{
		if(Get_time_flag==0)
		{
			Get_Time();//获取时间
//			INTO_delay_flag++;
			if(BurnIn_mode == 0)
				printf("get plan time success1\r\n");
			Get_time_flag=1;//获取时间函数只执行一次

			delay_ms(1000);
			delay_ms(1000);
			delay_ms(1000);
		}
		if(SendBuff[5]==0x04&&SendBuff[6]==0x06&&STOP_TIME==0)
		{
			if(SendBuff[11] == 0x1)
				time_zone = -SendBuff[12];
			else
				time_zone = SendBuff[12];
			tick= (SendBuff[7] << 24) + (SendBuff[8]<<16) + (SendBuff[9] << 8) + SendBuff[10];//将获取到的16进制时间戳转换成10进制数据
			Update_RTC_Time(tick);
			STOP_TIME=1;				
		}
		INTO_delay_count1=0;
	}
#endif
}

/***Added by pngao for RTC update***/
/******************************************************
*
*Function name : RTC_update
*Description 	 : Get utc time from app and update local RTC time
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void RTC_update(void)
{
	uint8_t i, j, update_flag;
	time_t tick;
	int16_t time_zone_tmp;
	
//	if(BurnIn_mode == 0)
	printf("Update RTC Time\r\n");
//	Get_time_flag=1;//获取时间函数只执行一次
	update_flag = 0;
	for(j = 0; j< 3; j++)
	{
		printf("get local time from app\r\n");
		Get_Time();//获取时间
		for(i=0; i< 200; i++)
		{
			if((Stop_send_flag == 0) && (USART1_length_value > 0) )
			{
				if(SendBuff[5]==0x04&&SendBuff[6]==0x06) //&&STOP_TIME==0)
				{
					printf("Received get_time command response\r\n");
					if(SendBuff[11] == 0x1)
						time_zone_tmp = -SendBuff[12];
					else
						time_zone_tmp = SendBuff[12];
					if(time_zone_tmp != time_zone)
					{
						uint8_t time_zone_buf[2];
						time_zone = time_zone_tmp;
						time_zone_buf[0] = time_zone >> 8 & 0xff;
						time_zone_buf[1] = time_zone &0xff;
						printf("Update Time Zone to %d\r\n", time_zone);
						if(AT24CXX_exist_flag)
						{
							AT24CXX_WriteBytes(TIME_ZONE_ADDR, time_zone_buf, 2);
						}
						else
						{
							EEPROM_WriteBytes(TIME_ZONE_ADDR, time_zone_buf, 2);
						}
					}
					tick= (SendBuff[7] << 24) + (SendBuff[8]<<16) + (SendBuff[9] << 8) + SendBuff[10];//将获取到的16进制时间戳转换成10进制数据
					if(BurnIn_mode == 0)
						printf("UTC Time = %d Time_Zone = %d\r\n",tick, time_zone );
					//tick = tick + 3600 * time_zone;
					Update_RTC_Time(tick);
	//				STOP_TIME = 1;
					update_flag = 1;
					if(ble_connection_status == 0)
						ble_connection_status = 1;
					break;	
				}
			}
			delay_ms(10);
		}
		if(update_flag == 1)
			break;
	}
}

/******************************************************
*
*Function name : Update_RTC_Time
*Description 	 : Update RTC time 
*Parameter		 : 
*			@utc_time  the utc time get from app
*Return				 : NULL
*
******************************************************/
void Update_RTC_Time(time_t utc_time)
{
	time_t local_utc;
	struct tm app_tm;
	//Get local UTC time
	local_utc = RTC_To_UTC();
	//Compare local_utc and utc_time, and update RTC if deviation is more than 5S 
	if(abs(utc_time - local_utc) > 5)
	{
		//Convert UTC time to struct tm 
		utc_time = utc_time + 3600 * time_zone;
		app_tm =*localtime(&utc_time);
//		if(BurnIn_mode == 0)
//			printf("RTC need update to: Year=%d Mon=%d Day=%d Hour=%d Min=%d Sec=%d\r\n", app_tm.tm_year-100, app_tm.tm_mon+1, app_tm.tm_mday, app_tm.tm_hour, app_tm.tm_min, app_tm.tm_sec);			
		//Write new time to RTC Register
		RTC_Set_Date(app_tm.tm_year - 100, app_tm.tm_mon + 1, app_tm.tm_mday, RTC_H12_AM);
		RTC_Set_Time(app_tm.tm_hour, app_tm.tm_min, app_tm.tm_sec, RTC_H12_AM);
	}
	else
	{
		if(BurnIn_mode == 0)
			printf("RTC time is right\r\n");
	}
}
/***added end***/

/******************************************************
*
*Function name : Read_Config_Data
*Description 	 : Read product info,device info and configuration from EEPROM
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Read_Config_Data(void)
{
	uint8_t buf[1024];
	u8 i= 0, j=0;
	int16_t temp_data;
	
	if(AT24CXX_exist_flag)
		AT24CXX_ReadBytes(0, buf, sizeof(buf));
	else
		EEPROM_ReadBytes(0, buf, sizeof(buf));
	printf("Read EEPROM Data: \r\n");
	for(i = 0; i<64; i++)
	{
		for(j = 0; j< 16; j++)
			printf(" 0x%02X ", buf[i*16+j]);
		printf("\r\n");
	}
	//product information init
	//get product MDL
	if(buf[PRODUCT_MDL_ADDR] == 0xff)
	{
		strcpy((char *)pro_info.product_mdl, ProductID);
	}
	else
	{
		memcpy(pro_info.product_mdl, buf, PRODUCT_MDL_MAX_SIZE);
	}
	//get product SN
	if(buf[PRODUCT_SN_ADDR] == 0xff)
	{
		strcpy(pro_info.product_sn, "000000000\0");
	}
	else
	{
		memcpy(pro_info.product_sn, buf + PRODUCT_SN_ADDR, PRODUCT_SN_SIZE);
	}
	//get product PN
	if(buf[PRODUCT_PN_ADDR] == 0xff)
	{
		strcpy(pro_info.product_pn, "000-000000-00\0");
	}
	else
	{
		memcpy(pro_info.product_pn, buf + PRODUCT_PN_ADDR, PRODUCT_PN_SIZE);
	}
	//get product CKEY
	if(buf[PRODUCT_CKEY_ADDR] == 0xff)
	{
		strcpy(pro_info.product_ckey, "abcdefghij\0");
	}
	else
	{
		memcpy(pro_info.product_ckey, buf + PRODUCT_CKEY_ADDR, PRODUCT_CKEY_SIZE);
	}
	//get product USN1
	if(buf[PRODUCT_USN1_ADDR] == 0xff)
	{
		strcpy(pro_info.product_usn1, "000000\0");
	}
	else
	{
		memcpy(pro_info.product_usn1, buf + PRODUCT_USN1_ADDR, PRODUCT_USN_SIZE);
	}
	//get product USN2
	if(buf[PRODUCT_USN2_ADDR] == 0xff)
	{
		strcpy(pro_info.product_usn2, "000000\0");
	}
	else
	{
		memcpy(pro_info.product_usn2, buf + PRODUCT_USN2_ADDR, PRODUCT_USN_SIZE);
	}	
	//get product warranty exp date
	if(buf[PRODUCT_WARRANTY_EXP_DATE_ADDR] == 0xff)
	{
		pro_info.product_warranty_exp_date = 0;
	}
	else
	{
		pro_info.product_warranty_exp_date = (buf[PRODUCT_WARRANTY_EXP_DATE_ADDR] << 24) + (buf[PRODUCT_WARRANTY_EXP_DATE_ADDR+1] << 16)
			+(buf[PRODUCT_WARRANTY_EXP_DATE_ADDR+2] << 8) + buf[PRODUCT_WARRANTY_EXP_DATE_ADDR+3];
	}
	if(buf[PRODUCT_WARRANTY_START_UTC_ADDR] == 0xff)
	{
		//pro_info.product_warranty_start_date = 0;
		pro_info.product_warranty_start_date = DEFAULT_WARS_DATE;
	}
	else
	{
		pro_info.product_warranty_start_date = (buf[PRODUCT_WARRANTY_START_UTC_ADDR] << 24) + (buf[PRODUCT_WARRANTY_START_UTC_ADDR+1] << 16)
			+(buf[PRODUCT_WARRANTY_START_UTC_ADDR+2] << 8) + buf[PRODUCT_WARRANTY_START_UTC_ADDR+3];
	}
	//get product registered flag
	if(buf[PRODUCT_REGISTERED_FLAG_ADDR] == 0x1)
	{
		pro_info.product_registration_flag = 1;
	}
	else
	{
		pro_info.product_registration_flag = 0;	
	}	
	
#if 0	
	if(pro_info.product_registration_flag && (pro_info.product_warranty_start_date!= 0))
	{
		Warranty_start_flag = 1;
	}
#endif
	if(buf[PRODUCT_WARRANTY_START_FLAG] == 0x1)
	{
		Warranty_start_flag = 1;
	}
	else
	{
		Warranty_start_flag = 0;
	}
	
	//get time zone info	
	temp_data = buf[TIME_ZONE_ADDR] << 8 | buf[TIME_ZONE_ADDR + 1];
	if((temp_data >= -12 )&&(temp_data <= 12))
	{
		time_zone = temp_data;
	}
	else
		time_zone = 0;
	printf("time_zone = %d\r\n", time_zone);
	
	//device config data init
	if(buf[HIGH_TEMP_WARNING_ADDR] == 0xff)
	{
		dev_data.High_Temp_Thresh_Warning = DEFAULT_HIGH_TEMP_THRESH_WARNING;
	}
	else
	{
		dev_data.High_Temp_Thresh_Warning = buf[HIGH_TEMP_WARNING_ADDR];
	}
	
	if(buf[HIGH_TEMP_ERROR_ADDR] == 0xff)
	{
		dev_data.High_Temp_Thresh_ERROR = DEFAULT_HIGH_TEMP_THRESH_ERROR;
	}
	else
	{
		dev_data.High_Temp_Thresh_ERROR = buf[HIGH_TEMP_ERROR_ADDR];
	}
	printf("Temp_Thresh_Warning = %d Temp_Thresh_ERROR = %d\r\n", dev_data.High_Temp_Thresh_Warning, dev_data.High_Temp_Thresh_ERROR);
	
	if(buf[FAN_SPEED_WARNING_ADDR] == 0xff)
	{
		dev_data.FAN_Speed_Thresh_Warning = DEFAULT_FAN_SPEED_THRESH_WARNING;
	}
	else
	{
		dev_data.FAN_Speed_Thresh_Warning = (buf[FAN_SPEED_WARNING_ADDR] << 8) | buf[FAN_SPEED_WARNING_ADDR + 1];
	}
	printf("FAN_Speed_Thresh = %d\r\n", dev_data.FAN_Speed_Thresh_Warning);
	
	if(buf[UV_EOL_WARNING_ADDR] == 0xff)
	{
		dev_data.UV_EOL_WARNING = DEFAULT_UV_EOL_WARNING;
	}
	else
	{
		dev_data.UV_EOL_WARNING = (buf[UV_EOL_WARNING_ADDR] << 8) | buf[UV_EOL_WARNING_ADDR + 1];
	}
	
	if(buf[UV_EOL_MAXIMUM_ADDR] == 0xff)
	{
		dev_data.UV_EOL_MAXIMUM = DEFAULT_UV_EOL_ERROR;
	}
	else
	{
		dev_data.UV_EOL_MAXIMUM = (buf[UV_EOL_MAXIMUM_ADDR] << 8) | buf[UV_EOL_MAXIMUM_ADDR + 1];
	}
	printf("UV_EOL_WARNING = %d, UV_EOL_MAXIMUM = %d \r\n", dev_data.UV_EOL_WARNING, dev_data.UV_EOL_MAXIMUM);
	
	if(buf[UV_DUTY_CYCLE_ADDR] == 0xff)
	{
		dev_data.UV_DUTY_CYCLE = DEFAULT_UV_DUTY_CYCLE;
	}
	else
	{
		dev_data.UV_DUTY_CYCLE = (buf[UV_DUTY_CYCLE_ADDR] << 8) | buf[UV_DUTY_CYCLE_ADDR + 1];
	}
	printf("duty_cycle = %.1f%\r\n", (float)(dev_data.UV_DUTY_CYCLE/10.0));
	str_duty_value = dev_data.UV_DUTY_CYCLE *1.5;
	//Not supported , default value init
	dev_data.Unoccupied_PIR_Pause_Time = 15;
	dev_data.Unoccupied_Scale_Factor = 10;
	dev_data.Smart_Max_On_Time = 15;
	dev_data.Smart_Scale_Factor = 10;
	dev_data.PIR_Function_Flag = 0;
	
	if(buf[MANUAL_KEY_FLAG_ADDR] == 0x1)
	{
		dev_data.Manual_Key_Enable_Flag = 1;
	}
	else
	{
		dev_data.Manual_Key_Enable_Flag = 0;
	}
	printf("Manual_Key_Enable_Flag = %d\r\n", dev_data.Manual_Key_Enable_Flag);
	
	if(buf[MAX_MANUAL_ONTIME_ADDR] == 0xff)
	{
		dev_data.Manual_On_Time = 15;
	}
	else
	{
		dev_data.Manual_On_Time = (buf[MAX_MANUAL_ONTIME_ADDR] << 8) | buf[MAX_MANUAL_ONTIME_ADDR + 1] ;
	}
	Manual_time_count=  dev_data.Manual_On_Time;
	printf("Manual_On_Time = %d\r\n", Manual_time_count);
	
	if(buf[PILOT_LAMP_BRIGHTNESS_ADDR] == 0xff)
	{
		dev_data.Pilot_Lamp_Brightness = DEFAULT_PILOT_LAMP_BRIGHTNESS;
	}
	else
	{
		dev_data.Pilot_Lamp_Brightness = buf[PILOT_LAMP_BRIGHTNESS_ADDR];
	}
	pilot_lamp_value = 450 - dev_data.Pilot_Lamp_Brightness * 4;
	printf("Pilot_Lamp_Brightness = %d, pilot_lamp_value= %d \r\n", dev_data.Pilot_Lamp_Brightness, pilot_lamp_value);
	
	if(buf[STEALTH_MODE_FLAG_ADDR] == 0x1)
	{
		dev_data.Stealth_In_Standby = 1;
	}
	else
	{
		dev_data.Stealth_In_Standby = 0;
	}
	printf("Stealth_In_Standby = %d \r\n", dev_data.Stealth_In_Standby);
	
	if(buf[FIRST_CONNECT_FLAG_ADDR] == 0x1)
	{
		dev_data.First_Connect_Flag = 1;
	}
	else
	{
		dev_data.First_Connect_Flag = 0;
	}
//	first_connect_flag = dev_data.First_Connect_Flag;
	printf("FIRST_CONNECT_FLAG = %d\r\n", buf[FIRST_CONNECT_FLAG_ADDR]);
	
	if((buf[UV1_LIFETIME_ADDR] == 0xff )&& (buf[UV1_LIFETIME_ADDR+1] == 0xff ))
	{
		UV1_Life_read_EEPROM_value[0] =  0;
		UV1_Life_read_EEPROM_value[1] =  0;
		UV1_Life_Hours = 0;
	}
	else
	{
		UV1_Life_read_EEPROM_value[0] = buf[UV1_LIFETIME_ADDR];
		UV1_Life_read_EEPROM_value[1] = buf[UV1_LIFETIME_ADDR + 1];
		UV1_Life_Hours = (UV1_Life_read_EEPROM_value[0] << 8) + UV1_Life_read_EEPROM_value[1];
	}

	if((buf[UV2_LIFETIME_ADDR] == 0xff )&& (buf[UV2_LIFETIME_ADDR+1] == 0xff ))
	{
		UV2_Life_read_EEPROM_value[0] =  0;
		UV2_Life_read_EEPROM_value[1] =  0;
		UV2_Life_Hours = 0;
	}
	else
	{
		UV2_Life_read_EEPROM_value[0] = buf[UV2_LIFETIME_ADDR];
		UV2_Life_read_EEPROM_value[1] = buf[UV2_LIFETIME_ADDR + 1];
		UV2_Life_Hours = (UV2_Life_read_EEPROM_value[0] << 8) + UV2_Life_read_EEPROM_value[1];
	}

	//Get schedule manual mode setting
	if(buf[SCHEDULE_MANUAL_MODE_ADDR] == 0)
	{
		manual_schedule_flag = 0;
	}
	else
	{
		manual_schedule_flag = 1;
	}
	
	/******/
	//PLAN Process
	if((buf[PLAN_SIZE_ADDR] == 0xff)||(buf[PLAN_SIZE_ADDR] == 0))
	{
		Plan_length = 0;
	}
	else
	{
		Plan_length = buf[PLAN_SIZE_ADDR];
	}
	
	printf("%d bytes schedule_data:\r\n", Plan_length);
	for(i=0;i<Plan_length;i++)
  {
    Plan_buffer[i] = buf[PLAN1_ADDR + i];       //保存计划数据
		printf(" 0x%02X ",Plan_buffer[i]);
		if(i%16 == 0)
			printf("\r\n");
  }
	printf("\r\n");
	execute_Plan_flag=1;
//	BLE_PLAN_tab_flag=1;//BLE模组和计划模式在定时器4切换使用标志位
//	Plan_happen=1;//当用户APP端打开计划功能或者设备端开机重启时，计划产生，标志位置1
	
	if(buf[LOG_NUMBER_ADDR] > MAX_EEPROM_LOG_NUMBER)
	{
		EEPROM_Log_Nums = 0;
		printf("No eeprom log is available\r\n");
	}
	else
	{		
		EEPROM_Log_Nums = buf[LOG_NUMBER_ADDR];
		printf("%d EEPROM Logs:\r\n", EEPROM_Log_Nums);
		for(i = 0 ; i < EEPROM_Log_Nums; i++)
		{
			printf("Logs %d : event = %x  utc = 0x%x%x%x%x\r\n", i, buf[LOG1_ADDR + 5*i], buf[LOG1_ADDR + 5*i + 1], buf[LOG1_ADDR + 5*i + 2], buf[LOG1_ADDR + 5*i + 3],buf[LOG1_ADDR + 5*i + 4]);
		}
		if(EEPROM_Log_Nums < 100)
		{
			log_cur_pos = EEPROM_Log_Nums;
		}
		else
		{
			uint32_t event1_time, event2_time;
			for(i = 1; i < EEPROM_Log_Nums; i++)
			{
				j = i-1;
				event1_time = (buf[LOG1_ADDR + 5*j + 1] << 24) + (buf[LOG1_ADDR + 5*j + 2] << 16) + (buf[LOG1_ADDR + 5*j + 3] << 8) + buf[LOG1_ADDR + 5*j +4];   
				event2_time = (buf[LOG1_ADDR + 5*i + 1] << 24) + (buf[LOG1_ADDR + 5*i + 2] << 16) + (buf[LOG1_ADDR + 5*i + 3] << 8) + buf[LOG1_ADDR + 5*i +4];
				printf("event1_utc=%d, evnet2_utc=%d\r\n", event1_time, event2_time);
				if(event1_time > event2_time)
				{
					log_cur_pos = i;
					break;
				}
			}
			printf("log_cur_pos = %d\r\n", log_cur_pos);
		}
	}
}

/******************************************************
*
*Function name : default_set
*Description 	 : enter standby mode
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void default_set(void)
{
	printf("Enter default_set\r\n");
	
	close_uv_lamp();
	execute_Plan_flag=0;
//	BLE_PLAN_tab_flag=0;		//BLE模组和计划模式在定时器4切换使用标志位
//	Plan_happen=0;					//当用户APP端打开计划功能或者设备端开机重启时，计划产生，标志位置1
	if(dev_data.First_Connect_Flag == 1)
	{
		if(dev_data.Stealth_In_Standby == 0)
		{
			if(pilot_lamp_mode != P2_BT_DEV_FAULT)
			{
				pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
				turnON_GREEN();
			}
			Device_mode= MODE_SBNS;
		}
		else
		{
			if(pilot_lamp_mode != P2_BT_DEV_FAULT)
			{
				pilot_lamp_mode = STEALTH_MODE;
				turn_all_off();
			}
			Device_mode= MODE_SBS;
		}
	}
	//Close Timer4&Timer7
	TIM_Cmd(TIM7,DISABLE); 
	duty_count = 0;
//	TIM_Cmd(TIM4,DISABLE);  
	if(Plan_running == 1)
	{
		Plan_running = 0;
	}
}

/******************************************************
*
*Function name : schedule_mode_run
*Description 	 : prepare to enter schedule mode
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void schedule_mode_run(void)
{
	execute_Plan_flag=1;
	key_schedule_flag=0;
	Manual_Schedule_Mode_Set(0);
//	Plan_happen=1;
//	BLE_PLAN_tab_flag=1;
	//Open Timer4&Timer7
//	TIM_Cmd(TIM7,ENABLE);   
//	TIM_Cmd(TIM4,ENABLE);  
	if(UV1_open_flag || UV2_open_flag)
	{
		close_uv_lamp();
		if(dev_data.Stealth_In_Standby == 0)
		{
			if(pilot_lamp_mode != P2_BT_DEV_FAULT)
			{
				pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
				turnON_GREEN();
			}
			Device_mode= MODE_SBNS;
		}
		else
		{
			if(pilot_lamp_mode != P2_BT_DEV_FAULT)
			{
				pilot_lamp_mode = STEALTH_MODE;
				turn_all_off();
			}
			Device_mode= MODE_SBS;
		}
	}
}

/***Added by pngao@20210705***/

/******************************************************
*
*Function name : RTC_To_UTC
*Description 	 : Convert local RTC time to UTC time
*Parameter		 : NULL
*Return				 : Converted UTC time
*
******************************************************/
time_t RTC_To_UTC()
{
//	char buf[50];
	struct tm stmT;
	RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;
	//Get RTC Time
	RTC_GetTime(RTC_Format_BIN, &stime);
	RTC_GetDate(RTC_Format_BIN, &sdate);
	stmT.tm_year=sdate.RTC_Year+100;
	stmT.tm_mon=sdate.RTC_Month-1;  
	stmT.tm_mday=sdate.RTC_Date;  
	stmT.tm_hour=stime.RTC_Hours;  
	stmT.tm_min=stime.RTC_Minutes;  
	stmT.tm_sec=stime.RTC_Seconds; 
//	if(BurnIn_mode == 0)	
//		printf("RTC Time is : Year=%d Mon=%d Day=%d Hour=%d Min=%d Sec=%d\r\n", sdate.RTC_Year,sdate.RTC_Month,sdate.RTC_Date,stime.RTC_Hours,stime.RTC_Minutes,stime.RTC_Seconds);
	return (mktime(&stmT) - 3600*time_zone);
}

/******************************************************
*
*Function name : Send_CurrentTime
*Description 	 : BLE Communication Command Query Device Current Time(0x35) processing 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Send_CurrentTime(void)
{
//	char buf[50];
	time_t now;
	u8 str_send_curtime[12]={0x55,0xAA,0x01,0x00,0xff,0X35,0X04,0X00,0X00,0X00,0X00,0X00}; 
  SN_count++;
  str_send_curtime[3]=(u8)((SN_count>>8)&0xff); //0x00;
  str_send_curtime[4]=(u8)(SN_count&0xff); //SN_count;
	//get UTC time
	now = RTC_To_UTC();	
	str_send_curtime[7] =(u8)((now >> 24) & 0xff);
	str_send_curtime[8] =(u8)((now >> 16) & 0xff);
	str_send_curtime[9] =(u8)((now >> 8) & 0xff);
	str_send_curtime[10] =(u8)(now & 0xff);
	str_send_curtime[11]=Sum_Rem(str_send_curtime,sizeof(str_send_curtime));
	if(BurnIn_mode == 0)
		printf("utc time is %d\r\n",now);
  if(SendBuff[5]==0x35)
	{
		Usart1_Send(str_send_curtime,sizeof(str_send_curtime));
		Stop_send_flag=1;
	}
}
/***Added end***/

/******************************************************
*
*Function name : turnON_GREEN
*Description 	 : Open green pilot lamp 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void turnON_GREEN(void)
{
	pilot_lamp_status = PILOT_GREEN_ON;
	TIM_SetCompare1(TIM12,pilot_lamp_value);	//修改比较值，修改占空比  灯G
	TIM_SetCompare2(TIM12,PILOT_LAMP_OFF);	//修改比较值，修改占空比 灯B
	TIM_SetCompare4(TIM3,PILOT_LAMP_OFF);	//修改比较值，修改占空比 灯R
}

/******************************************************
*
*Function name : turnON_RED
*Description 	 : Open red pilot lamp 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void turnON_RED(void)
{
	pilot_lamp_status = PILOT_RED_ON;
	TIM_SetCompare1(TIM12, PILOT_LAMP_OFF);	//修改比较值，修改占空比  灯G
	TIM_SetCompare2(TIM12, PILOT_LAMP_OFF);	//修改比较值，修改占空比 灯B
	TIM_SetCompare4(TIM3, pilot_lamp_value);	//修改比较值，修改占空比 灯R
}

/******************************************************
*
*Function name : turnON_BLUE
*Description 	 : Open blue pilot lamp 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void turnON_BLUE(void)
{
	pilot_lamp_status = PILOT_BLUE_ON;
	TIM_SetCompare1(TIM12, PILOT_LAMP_OFF);	//修改比较值，修改占空比  灯G
	TIM_SetCompare2(TIM12, pilot_lamp_value);	//修改比较值，修改占空比 灯B
	TIM_SetCompare4(TIM3, PILOT_LAMP_OFF);	//修改比较值，修改占空比 灯R
}

/******************************************************
*
*Function name : turn_all_off
*Description 	 : turn off all pilot lamps
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void turn_all_off(void)
{
	pilot_lamp_status = PILOT_OFF;
	TIM_SetCompare1(TIM12, PILOT_LAMP_OFF);	//修改比较值，修改占空比  灯G
	TIM_SetCompare2(TIM12, PILOT_LAMP_OFF);	//修改比较值，修改占空比 灯B
	TIM_SetCompare4(TIM3, PILOT_LAMP_OFF);	//修改比较值，修改占空比 灯R
}

/******************************************************
*
*Function name : LampFlash_GREEN
*Description 	 : green pilot lamp flash 
*Parameter		 : 
*	@times       : flash times variable
*Return				 : NULL
*
******************************************************/
void LampFlash_GREEN(uint8_t times)
{
	uint8_t i;
	uint8_t old_pilot_lamp_mode;
	old_pilot_lamp_mode = pilot_lamp_mode;
	pilot_lamp_mode = P6_DEVICE_IDENTIFY;
	for(i = 0; i< times; i++)
	{
		turn_all_off();
		delay_ms(500);
		turnON_GREEN();
		delay_ms(500);
	}
	turn_all_off();
	delay_ms(500);
	pilot_lamp_mode = old_pilot_lamp_mode;
	if(pilot_lamp_mode == P8_STANDBY_NON_STEALTH)
	{
		turnON_GREEN();
	}else if(pilot_lamp_mode == P9_Disinfection)
	{
		turnON_BLUE();
	}	
}

/******************************************************
*
*Function name : open_single_lamp
*Description 	 : open single uv lamp mode
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void open_single_lamp(void)
{
	//Compare lifetime of two UV lamp and First Open the lamp that has fewer lifetime
	printf("open_single_lamp\r\n");
	
	if(UV1_Life_Hours >= UV2_Life_Hours)
	{
		if(((uvlamp_fan_status & UV2_TURNON_FAILURE) == 0) && (UV2_Life_Hours < dev_data.UV_EOL_MAXIMUM))
		{
			printf("UV2 start turn on\r\n");
			//Open LAMP B
			UV_PWR_EN = 0;
			INVERTER_DRV_A_CMD = 0;
			LAMP_A_ENABLE = 0;
			LAMP_B_ENABLE = 1;
			delay_ms(UV_ON_DELAY);
			INVERTER_DRV_B_CMD = 1;
	//		UV1_life_hour_count=0;
			//Enable FAN2
			FAN1_ENABLE = 0;
			FAN2_ENABLE = 1;
			uv_lamp_opened = 2;
		}
		else if(((uvlamp_fan_status & UV1_TURNON_FAILURE) == 0) && (UV1_Life_Hours < dev_data.UV_EOL_MAXIMUM))
		{
			printf("UV2 is error, UV1 start turn on\r\n");
			UV_PWR_EN = 0;
			INVERTER_DRV_B_CMD = 0;
			LAMP_B_ENABLE = 0;
			LAMP_A_ENABLE = 1;
			delay_ms(UV_ON_DELAY);
			INVERTER_DRV_A_CMD = 1;	
//			UV2_life_hour_count=0;
			FAN1_ENABLE = 1;
			FAN2_ENABLE = 0;
			uv_lamp_opened = 1;
		}
		else
		{
			printf("UV1 & UV2 both turn on failure happened , Please check hardware\r\n");
		}
	}
	else
	{
		if(((uvlamp_fan_status & UV1_TURNON_FAILURE) == 0)  && (UV1_Life_Hours < dev_data.UV_EOL_MAXIMUM))
		{
			printf("UV1 start turn on\r\n");
			//Open LAMP A
			UV_PWR_EN = 0;
			INVERTER_DRV_B_CMD = 0;
			LAMP_B_ENABLE = 0;
			LAMP_A_ENABLE = 1;
			delay_ms(UV_ON_DELAY);
			INVERTER_DRV_A_CMD = 1;	
	//		UV2_life_hour_count=0;
			//Open FAN1
			FAN1_ENABLE = 1;
			FAN2_ENABLE = 0;
			uv_lamp_opened = 1;
		}
		else if(((uvlamp_fan_status & UV2_TURNON_FAILURE) == 0) && (UV2_Life_Hours < dev_data.UV_EOL_MAXIMUM))
		{
			printf("UV1 is error, UV2 start turn on\r\n");
			//Open LAMP B
			UV_PWR_EN = 0;
			INVERTER_DRV_A_CMD = 0;
			LAMP_A_ENABLE = 0;
			LAMP_B_ENABLE = 1;
			delay_ms(UV_ON_DELAY);
			INVERTER_DRV_B_CMD = 1;
	//		UV1_life_hour_count=0;
			//Enable FAN2
			FAN1_ENABLE = 0;
			FAN2_ENABLE = 1;
			uv_lamp_opened = 2;
		}
		else
		{
			printf("UV1 & UV2 both turn on failure happened , Please check hardware\r\n");
		}

	}
	delay_ms(50);
}

/******************************************************
*
*Function name : open_double_lamp
*Description 	 : open double uv lamp mode
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void open_double_lamp(void)
{
	printf("open_double_lamp\r\n");

	if((uvlamp_fan_status & (UV1_TURNON_FAILURE | UV2_TURNON_FAILURE)) != 0x3)
	{
		//Open lamp
		UV_PWR_EN = 1;
		delay_ms(UV_ON_DELAY);
		if(((uvlamp_fan_status & UV1_TURNON_FAILURE) == 0 ) && (UV1_Life_Hours < dev_data.UV_EOL_MAXIMUM))
		{
			LAMP_A_ENABLE =1;
			INVERTER_DRV_A_CMD = 1;
			FAN1_ENABLE = 1;
		}
		if(((uvlamp_fan_status & UV2_TURNON_FAILURE) == 0 ) && (UV2_Life_Hours < dev_data.UV_EOL_MAXIMUM))
		{
			LAMP_B_ENABLE =1;
			INVERTER_DRV_B_CMD = 1;
			FAN2_ENABLE = 1;
		}
		
		uv_lamp_opened = 3;
	}
	else
	{
		printf("UV lamp turnon error happened, please check hardware");
	}
}

/******************************************************
*
*Function name : close_uv_lamp
*Description 	 : close uv lamp
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void close_uv_lamp()
{
	printf("close_uv_lamp\r\n");

	if(single_double_lamp_flag == 1)
	{
		INVERTER_DRV_A_CMD = 0;
		INVERTER_DRV_B_CMD = 0;
		delay_ms(UV_OFF_DELAY);
		LAMP_A_ENABLE = 0;		
		LAMP_B_ENABLE = 0;
		UV_PWR_EN = 0;	
		FAN1_ENABLE = 0;		
		FAN2_ENABLE = 0;
	}
	else if(single_double_lamp_flag == 0)
	{
		INVERTER_DRV_A_CMD = 0;
		INVERTER_DRV_B_CMD = 0;
		LAMP_A_ENABLE = 0;		
		LAMP_B_ENABLE = 0;
		delay_ms(UV_OFF_DELAY);
		UV_PWR_EN = 0;	
		FAN1_ENABLE = 0;		
		FAN2_ENABLE = 0;
	}
	uv_lamp_opened = 0;
}


/******************************************************
*
*Function name : Set_UV_LAMP_Life
*Description 	 : Set UV Lamp Life
*Parameter		 : 
			@channel  UV Lamp Channel
			@uv_life  UV Lamp lifetime to be set
*Return				 : NULL
*
******************************************************/
void Set_UV_LAMP_Life(uint8_t channel, uint16_t uv_life)
{
	if(channel == 0)
	{
		UV1_Life_Hours = uv_life;
		UV1_Life_read_EEPROM_value[0] = (UV1_Life_Hours >> 8) & 0xff;
		UV1_Life_read_EEPROM_value[1] = UV1_Life_Hours & 0xff;
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(UV1_LIFETIME_ADDR, UV1_Life_read_EEPROM_value, 2); 
		else
			AT24CXX_WriteBytes(UV1_LIFETIME_ADDR,UV1_Life_read_EEPROM_value, 2);    //AT24LC64写入数据
	}
	else if(channel == 1)
	{
		UV2_Life_Hours = uv_life;
		UV2_Life_read_EEPROM_value[0] = (UV2_Life_Hours >> 8) & 0xff;
		UV2_Life_read_EEPROM_value[1] = UV2_Life_Hours & 0xff;
		if(AT24CXX_exist_flag == 0)
			EEPROM_WriteBytes(UV2_LIFETIME_ADDR, UV2_Life_read_EEPROM_value, 2); 
		else
			AT24CXX_WriteBytes(UV2_LIFETIME_ADDR, UV2_Life_read_EEPROM_value, 2);    //AT24LC64写入数据
	}
}

/******************************************************
*
*Function name : Save_Logs
*Description 	 : Function for saving logs to eeprom 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Save_Logs(void)
{
	uint8_t i;
	if(current_event_cnt > 0)
	{
		uint8_t *log_buf;
		printf("EEPROM has saved %d pieces of logs , Save %d pieces of new logs to EEPROM address %d\r\n", EEPROM_Log_Nums, current_event_cnt, log_cur_pos);
		log_buf = (uint8_t *)malloc(current_event_cnt*5);
		for(i = 0 ; i< current_event_cnt; i++)
		{
			log_buf[5*i] = log_event_data[i].event_type;
			log_buf[5*i + 1] = (log_event_data[i].event_time >> 24) & 0xff;
			log_buf[5*i + 2] = (log_event_data[i].event_time >> 16) & 0xff;
			log_buf[5*i + 3] = (log_event_data[i].event_time >> 8) & 0xff;
			log_buf[5*i + 4] = log_event_data[i].event_time & 0xff;
		}
		//event log in EEPROM
		if(current_event_cnt + EEPROM_Log_Nums <= MAX_EEPROM_LOG_NUMBER)
		{
			uint8_t log_num;
			log_num = current_event_cnt + EEPROM_Log_Nums;
			if(AT24CXX_exist_flag == 0)
			{
				EEPROM_WriteBytes(LOG_NUMBER_ADDR, &log_num, 1);
				EEPROM_WriteBytes(LOG1_ADDR + 5*EEPROM_Log_Nums, log_buf, current_event_cnt *5);
			}
			else
			{
				AT24CXX_WriteBytes(LOG_NUMBER_ADDR, &log_num, 1);
				AT24CXX_WriteBytes(LOG1_ADDR + 5*EEPROM_Log_Nums, log_buf, current_event_cnt *5);
			}
			EEPROM_Log_Nums = log_num;
			log_cur_pos = EEPROM_Log_Nums;
			if(log_cur_pos == MAX_EEPROM_LOG_NUMBER)
			{
				log_cur_pos = 0;
			}
		}
		else //if(current_event_cnt + EEPROM_Log_Nums > MAX_EEPROM_LOG_NUMBER)
		{
			if((current_event_cnt + log_cur_pos) <= MAX_EEPROM_LOG_NUMBER)
			{
				
				if(AT24CXX_exist_flag == 0)
				{
					EEPROM_WriteBytes(LOG1_ADDR + 5*log_cur_pos, log_buf, current_event_cnt *5);
				}
				else
				{
					AT24CXX_WriteBytes(LOG1_ADDR + 5*log_cur_pos, log_buf, current_event_cnt *5);
				}
				log_cur_pos +=  current_event_cnt;
				if(log_cur_pos == MAX_EEPROM_LOG_NUMBER)
					log_cur_pos = 0;
				EEPROM_Log_Nums = MAX_EEPROM_LOG_NUMBER;
			}
			else 
			{
				if(current_event_cnt <= MAX_EEPROM_LOG_NUMBER)
				{
					if(AT24CXX_exist_flag == 0)
					{
						if(EEPROM_Log_Nums < MAX_EEPROM_LOG_NUMBER)
						{
							EEPROM_Log_Nums = MAX_EEPROM_LOG_NUMBER;
							EEPROM_WriteBytes(LOG_NUMBER_ADDR, &EEPROM_Log_Nums, 1);
						}
						EEPROM_WriteBytes(LOG1_ADDR + 5*log_cur_pos, log_buf, 5*(MAX_EEPROM_LOG_NUMBER - log_cur_pos));
						EEPROM_WriteBytes(LOG1_ADDR, log_buf+5*(MAX_EEPROM_LOG_NUMBER - log_cur_pos), 5*(log_cur_pos + current_event_cnt - MAX_EEPROM_LOG_NUMBER));
					}
					else
					{
						if(EEPROM_Log_Nums < MAX_EEPROM_LOG_NUMBER)
						{
							EEPROM_Log_Nums = MAX_EEPROM_LOG_NUMBER;
							AT24CXX_WriteBytes(LOG_NUMBER_ADDR, &EEPROM_Log_Nums, 1);
						}
						AT24CXX_WriteBytes(LOG1_ADDR + 5*log_cur_pos, log_buf, 5*(MAX_EEPROM_LOG_NUMBER - log_cur_pos));
						AT24CXX_WriteBytes(LOG1_ADDR, log_buf+5*(MAX_EEPROM_LOG_NUMBER - log_cur_pos), 5*(log_cur_pos + current_event_cnt - MAX_EEPROM_LOG_NUMBER));
					}
					log_cur_pos = log_cur_pos + current_event_cnt - MAX_EEPROM_LOG_NUMBER;
					if(log_cur_pos >= MAX_EEPROM_LOG_NUMBER)
						log_cur_pos = log_cur_pos - MAX_EEPROM_LOG_NUMBER;
					EEPROM_Log_Nums = MAX_EEPROM_LOG_NUMBER;
				}
				else  //current_event_cnt > MAX_EEPROM_LOG_NUMBER
				{
					uint8_t log_remainder;
					log_remainder = current_event_cnt% MAX_EEPROM_LOG_NUMBER;
					if(log_remainder+log_cur_pos >=MAX_EEPROM_LOG_NUMBER)
						log_cur_pos = log_cur_pos + log_remainder - MAX_EEPROM_LOG_NUMBER;
					else
						log_cur_pos = log_cur_pos + log_remainder;
								
					if(AT24CXX_exist_flag == 0)
					{
						if(EEPROM_Log_Nums < MAX_EEPROM_LOG_NUMBER)
						{
							EEPROM_Log_Nums = MAX_EEPROM_LOG_NUMBER;
							EEPROM_WriteBytes(LOG_NUMBER_ADDR, &EEPROM_Log_Nums, 1);
						}
						if(log_cur_pos == 0)
						{
							EEPROM_WriteBytes(LOG1_ADDR, log_buf+5*(current_event_cnt - MAX_EEPROM_LOG_NUMBER), 5*MAX_EEPROM_LOG_NUMBER);
						}
						else
						{
							EEPROM_WriteBytes(LOG1_ADDR + 5*log_cur_pos, log_buf+5*(current_event_cnt - MAX_EEPROM_LOG_NUMBER), 5*(MAX_EEPROM_LOG_NUMBER - log_cur_pos));
							EEPROM_WriteBytes(LOG1_ADDR, log_buf+5*(current_event_cnt - log_cur_pos), 5*log_cur_pos);
						}
					}
					else
					{
						if(EEPROM_Log_Nums < MAX_EEPROM_LOG_NUMBER)
						{
							EEPROM_Log_Nums = MAX_EEPROM_LOG_NUMBER;
							AT24CXX_WriteBytes(LOG_NUMBER_ADDR, &EEPROM_Log_Nums, 1);
						}
						if(log_cur_pos == 0)
						{
							AT24CXX_WriteBytes(LOG1_ADDR, log_buf+5*(current_event_cnt - MAX_EEPROM_LOG_NUMBER), 5*MAX_EEPROM_LOG_NUMBER);
						}
						else
						{
							AT24CXX_WriteBytes(LOG1_ADDR + 5*log_cur_pos, log_buf+5*(current_event_cnt - MAX_EEPROM_LOG_NUMBER), 5*(MAX_EEPROM_LOG_NUMBER - log_cur_pos));
							AT24CXX_WriteBytes(LOG1_ADDR, log_buf+5*(current_event_cnt - log_cur_pos), 5*log_cur_pos);
						}
					}
				}
			}
		}
		free(log_buf);
		current_event_cnt = 0;
		memset(log_event_data, 0 , sizeof(log_event_data));
	}		 
}



/******************************************************
*
*Function name : Hardware_Test
*Description 	 : Function for hardware test only 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Hardware_Test(void)
{
	u16 adc_fan1,adc_fan2, i;
	u8 uv_state, uv_mode, sw4_pressed;

	if(BurnIn_mode == 0)
		printf("GPIO Test Begin\r\n");
	//for test only
	//LED Test
	turnON_RED();
	delay_ms(1000);
	turn_all_off();
	turnON_GREEN();
	delay_ms(1000);
	turn_all_off();
	turnON_BLUE();
	delay_ms(1000);
	turn_all_off();
	D8 = 0;
	D9 = 0;
	delay_ms(1000);
	D8 = 1;
	D9 = 1;
	//FAN Test
	FAN1_ENABLE = 1;
	FAN2_ENABLE = 1;
	for(i = 0; i< 5; i++)
	{
		if(BurnIn_mode == 0)
			printf("SW3 = %d, SW4 = %d, SW5 = %d, SW6 = %d PIR = %d\r\n", SW3, SW4, SW5, SW6, PIR_input);
		delay_ms(1000);
		adc_fan1=Get_Adc_Average(ADC_Channel_6,20);
		adc_fan2=Get_Adc_Average(ADC_Channel_7,20);
		if(BurnIn_mode == 0)
			printf("FAN adc value %d %d\r\n", adc_fan1,adc_fan2);
	}
	FAN1_ENABLE = 0;
	FAN2_ENABLE = 0;
	//UV Control test	
	if((SW5 == 0) && (SW6 == 0))
	{
		uv_mode = 2;
	}
	else
	{
		uv_mode = 1 ;
	}
	uv_state = 0;
	for(i = 0; i< 10000; i++)
	{
		if(SW4)
		{
			if(sw4_pressed == 0)
			{
				sw4_pressed = 1;
				if(BurnIn_mode == 0)
					printf("SW4 is pressed\r\n");
			}
		}
		else
		{
			if(sw4_pressed)
			{
				if(BurnIn_mode == 0)
				{
					printf("SW4 is released\r\n");
					printf("uv_mode = %d, uv_state = %d \r\n", uv_mode, uv_state);
				}
				if(uv_mode == 1)
				{
					switch(uv_state)
					{
						case 0:
							FAN1_ENABLE = 1;
							LAMP_B_ENABLE = 0;
							INVERTER_DRV_A_CMD = 0;
							LAMP_A_ENABLE = 1;
							delay_ms(10);
							INVERTER_DRV_A_CMD = 1;
							delay_ms(1000);
							if(LAMP_ON_A == 0)
							{
								uv_state = 1;
							}
							else
							{
								LAMP_A_ENABLE = 0;
								INVERTER_DRV_A_CMD = 0;
								FAN1_ENABLE = 0;
							}
							break;
						case 1:
							LAMP_A_ENABLE = 0;
							INVERTER_DRV_A_CMD = 0;
							FAN1_ENABLE = 0;
							delay_ms(10);
							FAN2_ENABLE = 1;
							LAMP_B_ENABLE = 1;
							delay_ms(10);	
							INVERTER_DRV_B_CMD = 1;
							delay_ms(1000);
							if(LAMP_ON_B == 0)
							{
								uv_state = 2;
							}
							else
							{
								LAMP_B_ENABLE = 0;
								INVERTER_DRV_B_CMD = 0;
								FAN2_ENABLE = 0;
							}
							break;
						case 2:
							LAMP_B_ENABLE = 0;
							INVERTER_DRV_B_CMD = 0;
							FAN2_ENABLE = 0;
							uv_state = 0;
							break;
					}
				}
				else if(uv_mode == 2)
				{
					if(uv_state == 0)
					{
						FAN1_ENABLE = 1;
						FAN2_ENABLE = 1;
						UV_PWR_EN = 1;
						delay_ms(10);
						LAMP_A_ENABLE =1;
						INVERTER_DRV_A_CMD = 1;
						LAMP_B_ENABLE = 1;
						INVERTER_DRV_B_CMD = 1;
						delay_ms(1000);
						if((LAMP_ON_B == 0) && (LAMP_ON_A) == 0)
						{
							if(BurnIn_mode == 0)
								printf("LAMP ON OK\r\n");
							uv_state = 1;
						}
						else
						{
							if(BurnIn_mode == 0)
								printf("LAMP Status %d %d\r\n", LAMP_ON_A, LAMP_ON_B);
							close_uv_lamp();
						}
					}
					else
					{
						close_uv_lamp();
						uv_state  = 0;
					}
				}
				sw4_pressed = 0;
			}
		}
		delay_ms(10);
	}
	//I2C device test
	if(BurnIn_mode == 0)
	{
		printf("Temperature is %f \r\n", 0.125*I2C_LM75read());
		printf("GPIO Expander Value : %x\r\n", I2C_TCA9555read());	
	}
}

/******************************************************
*
*Function name : PLAN_Clear
*Description 	 : Function for Clearing Plan 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void PLAN_Clear(void)
{
	uint8_t old_length;
	if(Plan_length > 0)
	{
		old_length = Plan_length;
		Plan_length = 0;
		memset(Plan_buffer, 0, sizeof(Plan_buffer));
		if(AT24CXX_exist_flag == 0)
		{
			EEPROM_WriteBytes(PLAN_SIZE_ADDR , &Plan_length, 1);
			EEPROM_WriteBytes(PLAN1_ADDR, Plan_buffer, old_length);			
		}
		else
		{
			AT24CXX_WriteBytes(PLAN_SIZE_ADDR, &Plan_length, 1);    //AT24LC64写入数据
			delay_ms(5);
			AT24CXX_WriteBytes(PLAN1_ADDR, Plan_buffer, old_length);    //AT24LC64写入数据
		}
		Plan_length = 0;
	}
}

/******************************************************
*
*Function name : Manual_Schedule_Mode_Set
*Description 	 : Function for Saving Manual_Schedule_Mode_Flag to EEPROM 
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void Manual_Schedule_Mode_Set(uint8_t value)
{
	manual_schedule_flag = value;
	if(AT24CXX_exist_flag == 0)
	{
		EEPROM_WriteBytes(SCHEDULE_MANUAL_MODE_ADDR , &manual_schedule_flag, 1);		
	}
	else
	{
		AT24CXX_WriteBytes(SCHEDULE_MANUAL_MODE_ADDR, &manual_schedule_flag, 1);
	}
}
