/**
 *    COPYRIGHT NOTICE
 *    Copyright (c) 2021, 
 *    All rights reserved.
 *@file  TestComm.h
 *		This file defines Serial Command and functions for production test.
 *@Author  gaopeng
 *@Version 1.0
 *@Date    2021-07-14 
 *@Revision  First Version
 **/

#ifndef __TESTCOMM_H
#define __TESTCOMM_H

#define CMD_QUERY_UV_STATUS "(PWR?)"
#define CMD_CLOSE_UV	"(PWR0)"
#define CMD_OPEN_UV	"(PWR1)"
#define CMD_CLOSE_PILOT_LAMP_RED "(LED=R0)"
#define CMD_OPEN_PILOT_LAMP_RED "(LED=R1)"
#define CMD_CLOSE_PILOT_LAMP_GREEN "(LED=G0)"
#define CMD_OPEN_PILOT_LAMP_GREEN "(LED=G1)"
#define CMD_CLOSE_PILOT_LAMP_BLUE "(LED=B0)"
#define CMD_OPEN_PILOT_LAMP_BLUE "(LED=B1)"
#define CMD_QUERY_UV1_LIFE "(LIFE1?)"
#define CMD_QUERY_UV2_LIFE "(LIFE2?)"
#define CMD_SET_UV1_LIFE "(LIFE1="
#define CMD_SET_UV2_LIFE "(LIFE2="
#define CMD_QUERY_FAN1_SPEED "(FAN1?)"
#define CMD_QUERY_FAN2_SPEED "(FAN2?)"
#define CMD_QUERY_SN "(SN?)"
#define CMD_WRITE_SN "(SN="
#define CMD_QUERY_MDL "(MDL?)"
#define CMD_WRITE_MDL "(MDL="
#define CMD_QUERY_BT "(BT?)"
#define CMD_BT_RST "(RST=BT)"
#define CMD_RST_FAC "(RST=FAC)"
#define CMD_RST_REG "(RST=REG)"
#define CMD_QUERY_PN "(PN?)"
#define CMD_WRITE_PN "(PN="
#define CMD_QUERY_CKEY "(CKEY?)"
#define CMD_WRITE_CKEY "(CKEY="
#define CMD_QUERY_USN1 "(USN1?)"
#define CMD_QUERY_USN2 "(USN2?)"
#define CMD_WRITE_USN1 "(USN1="
#define CMD_WRITE_USN2 "(USN2="
#define CMD_GET_ENVTEMP "(TEMP?)"
#define CMD_GET_SW3 "(SW3?)"
#define CMD_GET_SW4 "(SW4?)"
#define CMD_GET_SW5 "(SW5?)"
#define CMD_GET_SW6 "(SW6?)"
#define CMD_SET_WARS "(WARS="
#define CMD_SET_WARE "(WARE="
#define CMD_GET_BTMAC "(BTMAC?)"
#define CMD_GET_BTFWVER "(BTFWVER?)"
#define CMD_GET_WAR "(WAR?)"
#define CMD_SET_REG "(REG="
//Return Value of Command processing
enum CMD_PROCESS_RESULT
{
	CMD_OK=0,
	CMD_ERROR=1,
	PARAMETER_ERROR=2
};
uint8_t TestCommand_Parse(uint8_t *rxbuf, uint32_t len);

#endif
