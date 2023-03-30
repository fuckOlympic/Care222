#ifndef __FUNCTION_H
#define __FUNCTION_H
#include "sys.h"
#include <time.h>
#include "Config.h"
//#define SCB_AIRCR    (*(volatite unsigned long *)0xE000ED0C)      //Reset control Address Register
//#define SCB_RESET_VALUE  0x05FA0004                //reset value ,write to SCB_AIRCR  can reset cpu

//FLASH版本结构体定义
typedef struct _CONFIG_UPGRADE_CONFIG{
	uint32_t upgrade_flag;
	uint32_t block_size;
	uint32_t block_len;
	uint32_t CRC32_value;
	uint32_t upgrade_status;
	uint32_t Old_Vesion[2];
	uint32_t New_Vesion[2];
} CONFIG_UPGRADE_CONFIG;

//typedef struct _DEVICE_DATA{
//	uint16_t Unoccupied_DATA;
//	uint8_t SCALE_FACTOR;
//	uint16_t Smart_DATA;
//	uint8_t SCALE_FACTOR1;
//	uint8_t PIR;
//	uint8_t Maunal_Control;
//	uint32_t Maunal_time;
//} DEVICE_DATA;

typedef struct PRODUCT_DATA{
	char product_mdl[PRODUCT_MDL_MAX_SIZE+1];
	char product_sn[PRODUCT_SN_SIZE+1];
	char product_pn[PRODUCT_PN_SIZE+1];
	char product_ckey[PRODUCT_CKEY_SIZE+1];
	char product_usn1[PRODUCT_USN_SIZE+1];
	char product_usn2[PRODUCT_USN_SIZE+1];
//	uint8_t product_warranty_exp_date[8];
	uint32_t product_warranty_exp_date;
	uint32_t product_warranty_start_date;
	uint8_t product_registration_flag;
}PRODUCT_INFO;

typedef struct _DEVICE_CONFIG_DATA{
	uint8_t High_Temp_Thresh_Warning;
	uint8_t High_Temp_Thresh_ERROR;
	uint16_t FAN_Speed_Thresh_Warning;
	uint16_t UV_EOL_WARNING;
	uint16_t UV_EOL_MAXIMUM;	
	uint16_t UV_DUTY_CYCLE;
	uint16_t Unoccupied_PIR_Pause_Time;
	uint8_t Unoccupied_Scale_Factor;
	uint16_t Smart_Max_On_Time;
	uint8_t Smart_Scale_Factor;
	uint8_t PIR_Function_Flag;
	uint8_t Manual_Key_Enable_Flag;
	uint16_t Manual_On_Time;
	uint8_t Pilot_Lamp_Brightness;
	uint8_t Stealth_In_Standby;
	uint8_t First_Connect_Flag;
}DEVICE_CONFIG_DATA;

typedef struct EVENT_LOG_DATA{
	uint8_t event_type;
	uint32_t event_time;
}EVENT_DATA;

enum Log_Event
{
	Ambient_Temperature_Warning=0x00,
	Ambient_temperature_Error =0x01,
	Fans_Speed_Warning = 0x02,
	Fans_Speed_Error = 0x03,
	BLE_Module_Reset = 0x04,
	Lamp_TurnOn_Failure = 0x05,
	BLE_Module_Connection_Lost = 0x06,
	BLE_Module_Reconnected =0x07,
	Manual_Turn_On_By_ResetButton = 0x08,
	Manual_Turn_Off_By_ResetButton = 0x09,
	Lamp_Lifetime_Warning = 0x0a,
	Lamp_Lifetime_Error = 0x0b,
	Lamp_Setting_Error = 0x0c
};

enum Working_Mode
{
	MODE_CMNS = 0x0, //Continuous Mode & Non-Stealth
	MODE_CMS = 0x1, //Continuous Mode & Stealth
	MODE_UMNS = 0x2, //Unoccupied Mode & Non-Stealth
	MODE_UMS = 0x3, //Unoccupied Mode & Stealth
	MODE_SMNS = 0x4, //Smart Mode & Non-Stealth
	MODE_SMS = 0x5, //Smart Mode & Stealth
	MODE_SBNS = 0x6, //Standby & Non-Stealth
	MODE_SBS = 0x7 //Standby & Stealth
};

enum Pilot_Lamp_Sts
{
	PILOT_OFF = 0x0,
	PILOT_GREEN_ON = 0x1,
	PILOT_BLUE_ON = 0x2,
	PILOT_RED_ON = 0x3
};

enum Pilot_Lamp_Mode
{
	STEALTH_MODE = 0x0,  //
	P1_BT_UNPAIR = 0x1,  //Red lamp fast flash
	P2_BT_DEV_FAULT = 0x2, //Red lamp on
	P3_RESET_ON_10S = 0x3, //Red lamp flash 3 times
	P5_UV_MANUAL_ON = 0x5, //Green lamp flash 2 times
	P6_DEVICE_IDENTIFY = 0x6, //Green lamp flash 3 times
	P7_RESET_PRESSED = 0x7, //Green lamp flash 3 times
	P8_STANDBY_NON_STEALTH = 0x8, //Green lamp on
	P9_Disinfection = 0x9, //Blue lamp on
};

#define UV1_TURNON_FAILURE 0x1
#define UV2_TURNON_FAILURE 0x2
#define FAN1_SPEED_ERROR 0x4
#define FAN2_SPEED_ERROR 0x8
#define FAN1_SPEED_WARNING 0x10
#define FAN2_SPEED_WARNING 0x20
#define UV1_EOL_WARNING 0x40
#define UV2_EOL_WARNING 0x80
#define UV1_EOL_ERROR 0x100
#define UV2_EOL_ERROR 0x200
#define TEMP_WARNING 0x400
#define TEMP_ERROR 0x800


//全局定义
extern u8 Stop_send_flag; //停止发送指令状态标志位
extern u16 SN_count;//序列号自增变量
extern RTC_TimeTypeDef RTC_TimeStruct;
//extern u8 Plan_happen;
//extern u8 BLE_PLAN_tab_flag;//BLE模组和计划模式在定时器9切换使用标志位
extern u8 Device_status;//设备状态变量
extern u8 Device_mode;//设备模式：
//extern u8 Get_time_temp_flag,Get_time_UV_flag,Get_time_BLE_flag,Get_time_fan_flag;//用在事件上报LOG上的标记位
//extern u8 unoccupied_flag, smart_flag;
//extern u8 STANDBY_mode_togger;//作用在计划模式下，如果不在设定模式内，则执行STANDBY模式变量标志位
extern u8 schedule_flag;//计划模式是否打开标志位
//extern u16 first_connect_flag;//设备端首次连接APP标记--读
extern u16 Manual_time_count;
extern uint16_t UV1_Life_Hours, UV2_Life_Hours;
extern u8 BurnIn_mode;
extern u8 single_double_lamp_flag ;
extern PRODUCT_INFO pro_info;
extern DEVICE_CONFIG_DATA dev_data;
extern u8 Warranty_start_flag;
extern EVENT_DATA log_event_data[16];
extern uint8_t current_event_cnt;
extern uint8_t EEPROM_Log_Nums;
extern uint8_t log_cur_pos;
extern uint8_t pilot_lamp_mode;
extern uint8_t Plan_running;
extern uint8_t uv_lamp_opened;

//声明
//void Reset_Cpu(void);
void GetAll_Status(void);
void Search_Info(void);
void BLE_Mesh(uint8_t data);
void BLE_Mesh_Response(void);
void Connect_Status(void);
void Switch_UV(void);
void Smart_Mode(void);
void Duty_Cycle(void);
void Device_Mode(void);
void Device_togger_Mode(void);
void Get_Time(void);
void GET_time_update(void);
void Life_Time(void);
void Plan(void);
void execute_Plan(void);//执行计划
void Device_Init(void);
void Unoccupied_Mode(void);
void Identify_Device(void);
void OTA_Ask(void);
void OTA_Update_Notice(void);
void temp_Item_Update(void);
void fan_Item_Update(void);
void BLErst_Item_Update(void);
void UV_Item_Update(void);
void life_Item_update(void);
void turnON_GREEN(void);
void turnON_RED(void);
void turnON_BLUE(void);
void turn_all_off(void);
void Manual_Mode(void);
void Manual_Control_delay(void);
void Pilot_Lamp_brightness(void);
void AT24C64_READ_data(void);
void default_set(void);
void Resume_schedule(void);
void schedule_mode_run(void);
void open_single_lamp(void);
void open_double_lamp(void);
void single_lamp(void);
void Send_CurrentTime(void);
void Query_Registration_Tag(void);
void Query_Warranty_StartDate(void);
void Set_Warranty_Period(void);
void BurnIn_Mode(void);
void RTC_update(void);
void Set_UV_LAMP_Life(uint8_t channel, uint16_t uv_life);
void Read_Config_Data(void);
void Get_BLE_Status(void);
time_t RTC_To_UTC(void);
/****Added by pngao@20210701****/
void EEPROM_READ_data(void);
void Hardware_Test(void);
/***Added end***/
uint8_t event_reporting(uint8_t *event_buf, uint8_t event_num);
void close_uv_lamp(void);
void LampFlash_GREEN(uint8_t times);
void EOL_Config(void);
void Get_BLE_MAC(uint8_t *mac_addr);
uint16_t Get_BLE_FWVER(void);
void Get_Temp(void);
void Get_Plan(void);
void Stealth_Mode_In_Standby(void);
void Save_Logs(void);
void PLAN_Clear(void);
void Manual_Schedule_Mode_Set(uint8_t value);


#endif  

