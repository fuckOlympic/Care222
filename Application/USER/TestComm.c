/**
 *    COPYRIGHT NOTICE
 *    Copyright (c) 2021, 
 *    All rights reserved.
 *@file  TestComm.c
 *		This file defines Serial Command function which does command parsing and processing for production test.
 *@Author  gaopeng
 *@Version 1.0
 *@Date    2021-07-14 
 *@Revision  First Version
 **/
 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "usart1.h"
#include "timer.h"
#include "function.h"
#include "24cxx.h"
#include "eeprom.h"
#include "uv_lamp.h"
#include "string.h"
#include "Config.h"
#include "TestComm.h"
#include "adc.h"
#include "fan_motor.h"
#include "key.h"
#include "lm75adp.h"

#include "usbd_cdc_core_loopback.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"

extern USB_OTG_CORE_HANDLE USB_OTG_dev;
#define USE_USB_COM 1
//extern uint8_t USART_RX_BUF[USART_REC_LEN]; 
//extern uint8_t uart5_frame_end;
//extern uint8_t uart5_recv_len;
extern uint8_t AT24CXX_exist_flag; 
//extern u8 UV1_Life_read_EEPROM_value[2],UV2_Life_read_EEPROM_value[2];
extern uint8_t ble_connection_status;
extern uint8_t dev_registered;
extern __IO uint32_t receive_count;
//extern u32 warranty_period;
//extern  u32 warranty_exp_date;
extern uint32_t FAN1_Frequency ;
extern uint32_t FAN2_Frequency ;
extern uint8_t usb_rxbuf[64];

/******************************************************
*
*Function name : TestCommand_Response
*Description 	 : FT Serial Command response
*Parameter		 : 
*			@buf   response buffer   
*     @len   response data length
*Return				 : NULL
*
******************************************************/

void TestCommand_Response(uint8_t *buf, uint8_t len)
{
//	printf("%s\r\n", buf);
#ifdef USE_USB_COM
	if(VCP_CheckDataReceived() != 0 )
	{
		VCP_ReceiveData(&USB_OTG_dev, usb_rxbuf, 64);
	}
	while (VCP_CheckDataSent() == 1);
	VCP_SendData(&USB_OTG_dev, buf, len);
	delay_ms(5);
//	receive_count = 0;
#else
	printf("%s\r\n", buf);
#endif
}

/******************************************************
*
*Function name : TestCommand_Parse
*Description 	 : FT Serial Command parsing and processing
*Parameter		 : NULL
*Return				 : cmd parsing and processing result  0 success, other fail
*
******************************************************/
uint8_t TestCommand_Parse(uint8_t *rxbuf, uint32_t len)
{
	char tx_buff[20] = {0};
	uint8_t tx_len;
	printf("Received %d bytes: %s\r\n", len, rxbuf);
//	uart5_frame_end = 0;
	
	//Query UV Lamp Status Command process
	if(strstr((char *)rxbuf, CMD_QUERY_UV_STATUS) != NULL)
	{
		tx_len = sprintf(tx_buff, "(PWR%d),(PWR%d)\r\n",LAMP_ON_A==1?0:1, LAMP_ON_B==1?0:1);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Open UV Lamp Command process
	if(strstr((char *)rxbuf,CMD_OPEN_UV) != NULL)
	{
		if(single_double_lamp_flag)
			open_single_lamp();
		else
			open_double_lamp();
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_OPEN_UV);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Close UV Lamp Command process
	if(strstr((char *)rxbuf,CMD_CLOSE_UV) != NULL)
	{
		close_uv_lamp();
		tx_len = sprintf(tx_buff, "%s\r\n",CMD_CLOSE_UV);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Close Red Pilot Lamp Command process	
	if(strstr((char *)rxbuf,CMD_CLOSE_PILOT_LAMP_RED) != NULL)
	{
		turn_all_off();
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_CLOSE_PILOT_LAMP_RED);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Open Red Pilot Lamp Command process	
	if(strstr((char *)rxbuf,CMD_OPEN_PILOT_LAMP_RED) != NULL)
	{
		//turnON_RED();
		TIM_SetCompare1(TIM12,PILOT_LAMP_OFF);
		TIM_SetCompare2(TIM12,PILOT_LAMP_OFF);
		TIM_SetCompare4(TIM3,PILOT_LAMP_BRIGHTNESS_MIN);
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_OPEN_PILOT_LAMP_RED);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Close Green Pilot Lamp Command process
	if(strstr((char *)rxbuf,CMD_CLOSE_PILOT_LAMP_GREEN) != NULL)
	{
		turn_all_off();
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_CLOSE_PILOT_LAMP_GREEN);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Open Green Pilot Lamp Command process
	if(strstr((char *)rxbuf,CMD_OPEN_PILOT_LAMP_GREEN) != NULL)
	{
//		turnON_GREEN();
		TIM_SetCompare1(TIM12,PILOT_LAMP_BRIGHTNESS_MIN);
		TIM_SetCompare2(TIM12,PILOT_LAMP_OFF);
		TIM_SetCompare4(TIM3,PILOT_LAMP_OFF);
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_OPEN_PILOT_LAMP_GREEN);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Close Blue Pilot Lamp Command process	
	if(strstr((char *)rxbuf,CMD_CLOSE_PILOT_LAMP_BLUE) != NULL)
	{
		turn_all_off();
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_CLOSE_PILOT_LAMP_BLUE);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Open Blue Pilot Lamp Command process
	if(strstr((char *)rxbuf,CMD_OPEN_PILOT_LAMP_BLUE) != NULL)
	{
//		turnON_BLUE();
		TIM_SetCompare1(TIM12,PILOT_LAMP_OFF);
		TIM_SetCompare2(TIM12,PILOT_LAMP_BRIGHTNESS_MIN);
		TIM_SetCompare4(TIM3,PILOT_LAMP_OFF);
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_OPEN_PILOT_LAMP_BLUE);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Query UV1 Lamp Life Command process
	if(strstr((char *)rxbuf,CMD_QUERY_UV1_LIFE) != NULL)
	{
		tx_len = sprintf(tx_buff, "%s%d)\r\n", CMD_SET_UV1_LIFE, UV1_Life_Hours);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Query UV1 Lamp Life Command process	
	if(strstr((char *)rxbuf,CMD_QUERY_UV2_LIFE) != NULL)
	{
		tx_len = sprintf(tx_buff, "%s%d)\r\n", CMD_SET_UV2_LIFE, UV2_Life_Hours);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Set UV1 Lamp Life Command process
	if(strstr((char *)rxbuf,CMD_SET_UV1_LIFE) != NULL)
	{
		char *pos1, *pos2, *tmp_buf;
		uint16_t uv_life;
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter, UV1_LIFE must be not null and  less than UV_EOL_MAXIMUM
		if((pos2 - pos1) > 1)
		{
			tmp_buf = (char *)malloc(pos2-pos1-1);
			strncpy(tmp_buf, pos1+1, pos2-pos1-1);
			uv_life = atoi(tmp_buf);
			free(tmp_buf);
			if(uv_life < dev_data.UV_EOL_MAXIMUM)
			{
				Set_UV_LAMP_Life(1, uv_life);
				//Set_UV_LAMP_Life(0, uv_life);
				tx_len = sprintf(tx_buff, "%s%d)\r\n", CMD_SET_UV1_LIFE, UV2_Life_Hours);
				TestCommand_Response((uint8_t *)tx_buff, tx_len);
				return CMD_OK;
			}
			else
				return PARAMETER_ERROR;
		}
		else
		{
			return PARAMETER_ERROR;
		}
	}
	//Set UV2 Lamp Life Command process
	if(strstr((char *)rxbuf,CMD_SET_UV2_LIFE) != NULL)
	{
		char *pos1,*pos2, *tmp_buf;
		uint16_t uv_life;
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter, UV2_LIFE must be not null and  less than UV_EOL_MAXIMUM
		if((pos2 - pos1) > 1)
		{
			tmp_buf = (char *)malloc(pos2-pos1-1);
			strncpy(tmp_buf, pos1+1, pos2-pos1-1);
			uv_life = atoi(tmp_buf);
			free(tmp_buf);
			if(uv_life < dev_data.UV_EOL_MAXIMUM)
			{
				Set_UV_LAMP_Life(0, uv_life);
				//Set_UV_LAMP_Life(1, uv_life);
				tx_len = sprintf(tx_buff, "%s%d)\r\n", CMD_SET_UV2_LIFE, UV1_Life_Hours);
				TestCommand_Response((uint8_t *)tx_buff, tx_len);
				return CMD_OK;
			}
			else
				return PARAMETER_ERROR;
		}
		else
		{
			return PARAMETER_ERROR;
		}
	}
	
	//Query FAN1 Speed Command process
	if(strstr((char *)rxbuf,CMD_QUERY_FAN1_SPEED) != NULL)
	{
		uint16_t adc_fan;
		adc_fan = FAN1_Frequency * 60;
		tx_len = sprintf(tx_buff, "(FAN1=%d)\r\n", adc_fan);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Query FAN2 Speed Command process
	if(strstr((char *)rxbuf,CMD_QUERY_FAN2_SPEED) != NULL)
	{
		uint16_t adc_fan;
		adc_fan = FAN2_Frequency * 60;
		tx_len = sprintf(tx_buff, "(FAN2=%d)\r\n", adc_fan);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Query Serial Number Command process
	if(strstr((char *)rxbuf,CMD_QUERY_SN) != NULL)
	{
		tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_SN, pro_info.product_sn);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Write Serial Number Command process
	if(strstr((char *)rxbuf, CMD_WRITE_SN) != NULL)
	{
		char *pos1, *pos2;
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter, SN string size confirm
		if((pos2 - pos1 -1) == PRODUCT_SN_SIZE)
		{
			strncpy((char *)pro_info.product_sn, pos1+1, PRODUCT_SN_SIZE);
			if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(PRODUCT_SN_ADDR, (uint8_t *)pro_info.product_sn, PRODUCT_SN_SIZE);
			else
				AT24CXX_WriteBytes(PRODUCT_SN_ADDR, (uint8_t *)pro_info.product_sn, PRODUCT_SN_SIZE);
			tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_SN, pro_info.product_sn);
			TestCommand_Response((uint8_t *)tx_buff, tx_len);
			return CMD_OK;
		}
		else
		{
			return PARAMETER_ERROR;
		}
	}
	
	//Query MDL Command process
	if(strstr((char *)rxbuf,CMD_QUERY_MDL) != NULL)
	{
		tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_MDL, pro_info.product_mdl);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Write Serial Number Command process
	if(strstr((char *)rxbuf, CMD_WRITE_MDL) != NULL)
	{
		char *pos1, *pos2;
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter, MDL string size confirm
		if(((pos2-pos1-1) <= PRODUCT_MDL_MAX_SIZE) && ((pos2-pos1-1) >= PRODUCT_MDL_MIN_SIZE))
		{
			memset(pro_info.product_mdl, 0, PRODUCT_MDL_MAX_SIZE);
			strncpy((char *)pro_info.product_mdl, pos1+1, pos2-pos1-1);
			if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(PRODUCT_MDL_ADDR, (uint8_t *)pro_info.product_mdl, PRODUCT_MDL_MAX_SIZE);
			else
				AT24CXX_WriteBytes(PRODUCT_MDL_ADDR, (uint8_t *)pro_info.product_mdl, PRODUCT_MDL_MAX_SIZE);
			tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_MDL, pro_info.product_mdl);
			TestCommand_Response((uint8_t *)tx_buff, tx_len);
			return CMD_OK;
		}
		else
			return PARAMETER_ERROR;
	}
	
	//Query BT Status Command process
	if(strstr((char *)rxbuf, CMD_QUERY_BT) != NULL)
	{
		char ble_status_str[16];
		switch(ble_connection_status)
		{
			case 0x0:
				strcpy(ble_status_str, "UNCONNECTED");
				break;
			case 0x1:
				strcpy(ble_status_str, "CONNECTED");
				break;
			case 0x2:
			case 0x3:
				strcpy(ble_status_str, "UNKNOWN");
				break;
		}
		tx_len = sprintf(tx_buff, "(BT=%s)\r\n", ble_status_str);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Reset BT Command process
	if(strstr((char *)rxbuf, CMD_BT_RST) != NULL)
	{
		//Reset BLE pair info
		BLE_Mesh(1);
		delay_ms(500);
		BLErst_flag=1;
		default_set();
		//Clear first_connect_flag
		dev_data.First_Connect_Flag = 0;
		if(AT24CXX_exist_flag == 0)
		{
			EEPROM_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
		}
		else
		{
			AT24CXX_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
		}
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_BT_RST);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
//		delay_ms(100);
//		NVIC_SystemReset();
		return CMD_OK;
	}
	
	//Reset FAC Command process
	if(strstr((char *)rxbuf, CMD_RST_FAC) != NULL)
	{
		//Clear BLE Pair info
		BLE_Mesh(1);
		delay_ms(500);
		BLErst_flag=1;
		default_set();
		dev_data.First_Connect_Flag = 0;
		//Clear First_connect_flag and UV Lifetime
		UV1_Life_Hours = 0;
		UV2_Life_Hours = 0;
		UV1_Life_read_EEPROM_value[0] = 0;
		UV1_Life_read_EEPROM_value[1] = 0;
		UV2_Life_read_EEPROM_value[0] = 0;
		UV2_Life_read_EEPROM_value[1] = 0;
		if(AT24CXX_exist_flag == 0)
		{
			EEPROM_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
			EEPROM_WriteBytes(UV1_LIFETIME_ADDR, UV1_Life_read_EEPROM_value, 2);
			EEPROM_WriteBytes(UV2_LIFETIME_ADDR, UV2_Life_read_EEPROM_value, 2);
		}
		else
		{
			AT24CXX_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
			AT24CXX_WriteBytes(UV1_LIFETIME_ADDR, UV1_Life_read_EEPROM_value, 2);
			AT24CXX_WriteBytes(UV2_LIFETIME_ADDR, UV2_Life_read_EEPROM_value, 2);
		}
		//Clear Log info
		if(EEPROM_Log_Nums > 0)
		{
			uint8_t i, buffer[5];
			memset(buffer, 0x0, 5);
			for(i = 0; i < EEPROM_Log_Nums; i++)
			{
				if(AT24CXX_exist_flag == 0)
				{
					EEPROM_WriteBytes(LOG1_ADDR+i*5, buffer, 5);
				}
				else
				{
					AT24CXX_WriteBytes(LOG1_ADDR+i*5, buffer, 5);
				}
			}
			EEPROM_Log_Nums = 0;
			if(AT24CXX_exist_flag == 0)
			{
				
				EEPROM_WriteBytes(LOG_NUMBER_ADDR, &EEPROM_Log_Nums, 1);
			}
			else
			{
				AT24CXX_WriteBytes(LOG_NUMBER_ADDR, &EEPROM_Log_Nums, 1);
			}
		}
		
		//T.B.D.
		tx_len = sprintf(tx_buff, "%s\r\n", CMD_RST_FAC);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		delay_ms(100);
		NVIC_SystemReset();
		return CMD_OK;
	}
	
	//Reset REG Command process
	if(strstr((char *)rxbuf, CMD_RST_REG) != NULL)
	{
		uint8_t zero_buf[4] = {0,0,0,0};
		uint8_t default_wars_date[4];
		pro_info.product_registration_flag = 0;
		pro_info.product_warranty_exp_date = 0;
		pro_info.product_warranty_start_date = DEFAULT_WARS_DATE;
		default_wars_date[0] = (pro_info.product_warranty_start_date >> 24) & 0xff;
		default_wars_date[1] = (pro_info.product_warranty_start_date >> 16) & 0xff;
		default_wars_date[2] = (pro_info.product_warranty_start_date >> 8) & 0xff;
		default_wars_date[3] =  pro_info.product_warranty_start_date  & 0xff;
		Warranty_start_flag = 0;
		//clear EEPROM Data of dev_registered_flag, warranty_exp_date and warranty_start_date
		if(AT24CXX_exist_flag == 0)
		{
			EEPROM_WriteBytes(PRODUCT_WARRANTY_EXP_DATE_ADDR, zero_buf, 4);
			EEPROM_WriteBytes(PRODUCT_WARRANTY_START_UTC_ADDR, default_wars_date, 4);
			EEPROM_WriteBytes(PRODUCT_REGISTERED_FLAG_ADDR, &pro_info.product_registration_flag, 1);
			EEPROM_WriteBytes(PRODUCT_WARRANTY_START_FLAG, &Warranty_start_flag, 1);
		}
		else
		{
			AT24CXX_WriteBytes(PRODUCT_WARRANTY_EXP_DATE_ADDR, zero_buf, 4);
			AT24CXX_WriteBytes(PRODUCT_WARRANTY_START_UTC_ADDR, default_wars_date, 4);
			AT24CXX_WriteBytes(PRODUCT_REGISTERED_FLAG_ADDR, &pro_info.product_registration_flag, 1);
			AT24CXX_WriteBytes(PRODUCT_WARRANTY_START_FLAG, &Warranty_start_flag, 1);
		}
		tx_len = sprintf(tx_buff, "(REG=0)\r\n(WARS=20210101)\r\n(WARE=00000000)\r\n");
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	//Query Part Number Command process
	if(strstr((char *)rxbuf,CMD_QUERY_PN) != NULL)
	{
		tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_PN, pro_info.product_pn);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Write Part Number Command process
	if(strstr((char *)rxbuf, CMD_WRITE_PN) != NULL)
	{
		char *pos1, *pos2;
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter, Part Number string size confirm
		if((pos2-pos1-1) == PRODUCT_PN_SIZE)
		{
			strncpy((char *)pro_info.product_pn, pos1+1, PRODUCT_PN_SIZE);
			if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(PRODUCT_PN_ADDR, (uint8_t *)pro_info.product_pn, PRODUCT_PN_SIZE);
			else
				AT24CXX_WriteBytes(PRODUCT_PN_ADDR, (uint8_t *)pro_info.product_pn, PRODUCT_PN_SIZE);
			tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_PN, pro_info.product_pn);
			TestCommand_Response((uint8_t *)tx_buff, tx_len);
			return CMD_OK;
		}
		else
			return PARAMETER_ERROR;
	}
	
	//Query Christie Key Command process
	if(strstr((char *)rxbuf,CMD_QUERY_CKEY) != NULL)
	{
		tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_CKEY, pro_info.product_ckey);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Write Christie Key Command process
	if(strstr((char *)rxbuf, CMD_WRITE_CKEY) != NULL)
	{
		char *pos1, *pos2;
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter, Christie KEY string size confirm
		if((pos2-pos1-1) == PRODUCT_CKEY_SIZE)
		{
			strncpy((char *)pro_info.product_ckey, pos1+1, PRODUCT_CKEY_SIZE);
			if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(PRODUCT_CKEY_ADDR, (uint8_t *)pro_info.product_ckey, PRODUCT_CKEY_SIZE);
			else
				AT24CXX_WriteBytes(PRODUCT_CKEY_ADDR, (uint8_t *)pro_info.product_ckey, PRODUCT_CKEY_SIZE);
			tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_CKEY, pro_info.product_ckey);
			TestCommand_Response((uint8_t *)tx_buff, tx_len);
			return CMD_OK;
		}
		else
			return PARAMETER_ERROR;
	}
	
	//Query USHIO lamp1 SN Command process
	if(strstr((char *)rxbuf,CMD_QUERY_USN1) != NULL)
	{
		tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_USN1, pro_info.product_usn1);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Write USHIO lamp1 SN Command process
	if(strstr((char *)rxbuf, CMD_WRITE_USN1) != NULL)
	{
		char *pos1, *pos2;
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter, USN1 string size confirm
		if((pos2-pos1-1) == PRODUCT_USN_SIZE)
		{
			strncpy((char *)pro_info.product_usn1, pos1+1, PRODUCT_USN_SIZE);
			if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(PRODUCT_USN1_ADDR, (uint8_t *)pro_info.product_usn1, PRODUCT_USN_SIZE);
			else
				AT24CXX_WriteBytes(PRODUCT_USN1_ADDR, (uint8_t *)pro_info.product_usn1, PRODUCT_USN_SIZE);
			tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_USN1, pro_info.product_usn1);
			TestCommand_Response((uint8_t *)tx_buff, tx_len);
			return CMD_OK;
		}
		else
			return PARAMETER_ERROR;
	}	

	//Query USHIO lamp2 SN Command process
	if(strstr((char *)rxbuf,CMD_QUERY_USN2) != NULL)
	{
		tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_USN2, pro_info.product_usn2);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Write USHIO lamp2 SN Command process
	if(strstr((char *)rxbuf, CMD_WRITE_USN2) != NULL)
	{
		char *pos1, *pos2;
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter, USN2 string size confirm
		if((pos2-pos1-1) == PRODUCT_USN_SIZE)
		{
			strncpy((char *)pro_info.product_usn2, pos1+1, PRODUCT_USN_SIZE);
			if(AT24CXX_exist_flag == 0)
				EEPROM_WriteBytes(PRODUCT_USN2_ADDR, (uint8_t *)pro_info.product_usn2, PRODUCT_USN_SIZE);
			else
				AT24CXX_WriteBytes(PRODUCT_USN2_ADDR, (uint8_t *)pro_info.product_usn2, PRODUCT_USN_SIZE);
			tx_len = sprintf(tx_buff, "%s%s)\r\n", CMD_WRITE_USN2, pro_info.product_usn2);
			TestCommand_Response((uint8_t *)tx_buff, tx_len);
			return CMD_OK;
		}
		else
			return PARAMETER_ERROR;
	}		
	//Get Env Temperature Command process
	if(strstr((char *)rxbuf, CMD_GET_ENVTEMP) != NULL)
	{
		int16_t env_temp;
		env_temp = 0.125*I2C_LM75read();
		tx_len = sprintf(tx_buff, "(TEMP=%d)\r\n", env_temp);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Get BTE Reconnect Key Command process
	if(strstr((char *)rxbuf, CMD_GET_SW3) != NULL)
	{
		tx_len = sprintf(tx_buff, "(SW3=%d)\r\n", SW3);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	//Get Test Key Command process
	if(strstr((char *)rxbuf, CMD_GET_SW4) != NULL)
	{
		tx_len = sprintf(tx_buff, "(SW4=%d)\r\n", SW4);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;			
	}	
	//Get SW5 Command process
	if(strstr((char *)rxbuf, CMD_GET_SW5) != NULL)
	{
		tx_len = sprintf(tx_buff, "(SW5=%d)\r\n", SW5);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}		
	//Get SW6 Command process
	if(strstr((char *)rxbuf, CMD_GET_SW6) != NULL)
	{
		tx_len = sprintf(tx_buff, "(SW6=%d)\r\n", SW6);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}		

	//Set Warranty Exp Date Start Command process
	if(strstr((char *)rxbuf, CMD_SET_WARS) != NULL)
	{
		char *pos1, *pos2;
		uint8_t temp_buf[8];
		uint32_t wdate;
		uint8_t exp_data[4];
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter
		if((pos2-pos1-1) == 8)
		{			
			strncpy((char *)temp_buf, pos1+1, 8);
			wdate = atoi((char *)temp_buf);
			pro_info.product_warranty_start_date = wdate;
			exp_data[0] = (wdate >> 24) & 0xff;
			exp_data[1] = (wdate >> 16) & 0xff;
			exp_data[2] = (wdate >> 8) & 0xff;
			exp_data[3] =  wdate & 0xff;
			Warranty_start_flag = 0;
				//Update EEPROM data
			if(AT24CXX_exist_flag == 0)
			{
				EEPROM_WriteBytes(PRODUCT_WARRANTY_START_UTC_ADDR, exp_data, 4);
				EEPROM_WriteBytes(PRODUCT_WARRANTY_START_FLAG, &Warranty_start_flag, 1);
			}
			else
			{
				AT24CXX_WriteBytes(PRODUCT_WARRANTY_START_UTC_ADDR, exp_data, 4);
				AT24CXX_WriteBytes(PRODUCT_WARRANTY_START_FLAG, &Warranty_start_flag, 1);
			}
			tx_len = sprintf(tx_buff, "%s%d)\r\n", CMD_SET_WARS, pro_info.product_warranty_start_date);
			TestCommand_Response((uint8_t *)tx_buff, tx_len);
			return CMD_OK;
		}
		else
			return PARAMETER_ERROR;
	}
	
	//Set Warranty Exp Date End Command process
	if(strstr((char *)rxbuf, CMD_SET_WARE) != NULL)
	{
		char *pos1, *pos2;
		uint8_t temp_buf[8];
		uint32_t wdate;
		uint8_t exp_data[4];
		pos1 = strstr((char *)rxbuf, "=");
		pos2 = strstr((char *)rxbuf, ")");
		//Check the validity of the parameter
		if((pos2-pos1-1) == 8)
		{			
			strncpy((char *)temp_buf, pos1+1, 8);
			wdate = atoi((char *)temp_buf);
			pro_info.product_warranty_exp_date = wdate;
			exp_data[0] = (wdate >> 24) & 0xff;
			exp_data[1] = (wdate >> 16) & 0xff;
			exp_data[2] = (wdate >> 8) & 0xff;
			exp_data[3] =  wdate & 0xff;
				//Update EEPROM data
			if(AT24CXX_exist_flag == 0)
			{
				EEPROM_WriteBytes(PRODUCT_WARRANTY_EXP_DATE_ADDR, exp_data, 4);
			}
			else
			{
				AT24CXX_WriteBytes(PRODUCT_WARRANTY_EXP_DATE_ADDR, exp_data, 4);
			}
			tx_len = sprintf(tx_buff, "%s%d)\r\n", CMD_SET_WARE, pro_info.product_warranty_exp_date);
			TestCommand_Response((uint8_t *)tx_buff, tx_len);
			return CMD_OK;
		}
		else
			return PARAMETER_ERROR;
	}
	if(strstr((char *)rxbuf, CMD_GET_BTMAC) != NULL)
	{
		uint8_t macaddr[6];
		Get_BLE_MAC(macaddr);
		tx_len = sprintf(tx_buff, "(BTMAC=%02x:%02x:%02x:%02x:%02x:%02x)\r\n",macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	if(strstr((char*)rxbuf, CMD_GET_BTFWVER) != NULL)
	{
		uint16_t fwver;
		fwver = Get_BLE_FWVER();
		tx_len = sprintf(tx_buff, "(BTFWVER=V%d.%d.%d.%d)\r\n", (fwver>>12)&0xf, (fwver>>8)&0xf, (fwver>>4)&0xf, fwver&0xf);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	if(strstr((char*)rxbuf, CMD_GET_WAR) != NULL)
	{
		tx_len = sprintf(tx_buff, "(REG=%d)\r\n(WARS=%08d)\r\n(WARE=%08d)\r\n", pro_info.product_registration_flag, pro_info.product_warranty_start_date, pro_info.product_warranty_exp_date);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);
		return CMD_OK;
	}
	
	if(strstr((char*)rxbuf, CMD_SET_REG) != NULL)
	{
		char *pos1;
		uint8_t reg_flag;
		pos1 = strstr((char *)rxbuf, "=");
		if(*(pos1+1) == '1')
			reg_flag = 1;
		else
			reg_flag = 0;
		if(reg_flag != pro_info.product_registration_flag)
		{
			pro_info.product_registration_flag = reg_flag;
			if(AT24CXX_exist_flag == 0)
			{
				EEPROM_WriteBytes(PRODUCT_REGISTERED_FLAG_ADDR, &reg_flag, 1);
			}
			else
			{
				AT24CXX_WriteBytes(PRODUCT_REGISTERED_FLAG_ADDR, &reg_flag, 1);
			}
		}
		tx_len = sprintf(tx_buff, "(REG=%d)\r\n", reg_flag);
		TestCommand_Response((uint8_t *)tx_buff, tx_len);	
		return CMD_OK;		
	}
	
	return CMD_ERROR;
}



