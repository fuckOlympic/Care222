#ifndef __CONFIG_H
#define __CONFIG_H
#define ProductID "akeso"
#define MCUFirm_Version "1.0.1.2"
#define MCUFirm_Version_U16 0x1012
#define BT_SOFT_Version 0x0001
#define PRODUCT_SN_SIZE  9
#define PRODUCT_MDL_MIN_SIZE 5
#define PRODUCT_MDL_MAX_SIZE 8
#define PRODUCT_PN_SIZE 13
#define PRODUCT_CKEY_SIZE 10
#define PRODUCT_USN_SIZE 6
#define MAX_EEPROM_LOG_NUMBER 100
#define UV_LAMP_PERIOD 1500
#define UV_TURNON_CHECK_DELAY 15
#define UV_RESTART_DELAY 10
#define MAX_REPORT_NUM_ONCE 20
#define DEFAULT_WARS_DATE 20210101

#define UV_ON_DELAY 550
#define UV_OFF_DELAY 500

//Default parameter setting
#define DEFAULT_HIGH_TEMP_THRESH_WARNING 50
#define DEFAULT_HIGH_TEMP_THRESH_ERROR 60
#define DEFAULT_FAN_SPEED_THRESH_WARNING 3000
#define DEFAULT_UV_EOL_WARNING  2900
#define DEFAULT_UV_EOL_ERROR  3000
#define DEFAULT_PILOT_LAMP_BRIGHTNESS 30
#define DEFAULT_UV_DUTY_CYCLE 1000
#define PILOT_LAMP_OFF 500
#define PILOT_LAMP_BRIGHTNESS_MIN  450
#define PILOT_LAMP_BRIGHTNESS_MAX  50

//EEPROM DATA Address Definition
//Product Information
#define PRODUCT_MDL_ADDR  0x0
#define PRODUCT_SN_ADDR 0x10  //digital*9 
#define PRODUCT_PN_ADDR 0x1A		//char*13
#define PRODUCT_CKEY_ADDR 0x2A	//char*10
#define PRODUCT_USN1_ADDR 0x34  //digital*6
#define PRODUCT_USN2_ADDR 0x3A	//digital*6
#define PRODUCT_WARRANTY_EXP_DATE_ADDR 0x40 //uint32
#define PRODUCT_WARRANTY_START_UTC_ADDR 0x48 //uint32
#define PRODUCT_REGISTERED_FLAG_ADDR  0x50			//uint8
#define PRODUCT_WARRANTY_START_FLAG 0x51  //uint8

#define TIME_ZONE_ADDR  0x52
//Device Warning Threshhold Config 
#define HIGH_TEMP_WARNING_ADDR 		0x56
#define HIGH_TEMP_ERROR_ADDR 			0x58
#define FAN_SPEED_WARNING_ADDR   	0x5A
#define UV_EOL_MAXIMUM_ADDR				0x5C
#define UV_EOL_WARNING_ADDR				0x5E

//Device Config
#define UV_DUTY_CYCLE_ADDR					0x60	//uint16
#define UNOCCUPIED_PIR_PAUSE_TIME_ADDR	0x62   //uint16
#define UNOCCUPIED_SCALE_FACTOR_ADDR		0x64	//uint16
#define SMART_MAX_ON_TIME_ADDR					0x65	//uint16
#define SMART_SCALE_FACTOR_ADDR					0x67	//uint16
#define PIR_FUNCTION_FLAG_ADDR  				0x68
#define MANUAL_KEY_FLAG_ADDR						0x69
#define MAX_MANUAL_ONTIME_ADDR 					0x6A	//uint16
#define PILOT_LAMP_BRIGHTNESS_ADDR			0x6C	//uint16
#define STEALTH_MODE_FLAG_ADDR					0x6E
#define FIRST_CONNECT_FLAG_ADDR  				0x6F
#define UV1_LIFETIME_ADDR								0x70
#define UV2_LIFETIME_ADDR								0x72
#define SCHEDULE_MANUAL_MODE_ADDR       0x74
//PLAN Config
#define PLAN_SIZE_ADDR									0x7F
#define PLAN1_ADDR											0x80
//LOG
#define LOG_NUMBER_ADDR									0x180
#define LOG1_ADDR												0x181
#endif
