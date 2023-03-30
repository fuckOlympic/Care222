#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "usart1.h"	
#include "led.h"
#include "beep.h"
#include "key.h"
#include "timer.h"
#include "myiic.h"
#include "24cxx.h"
#include "fan_motor.h"
#include "adc.h"
#include "dma.h"
#include "pwm.h"
#include "lm75adp.h"
#include "pir.h"
#include "uv_lamp.h"
#include "string.h"
#include "function.h"
#include "rtc.h"
#include "stmflash.h" 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "eeprom.h"
#include "tca9555.h"
#include "Config.h"
#include "TestComm.h"
//for usb support
#include "usbd_cdc_core_loopback.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "iwdg.h"

//描述：消毒灯源代码
//作者：jerry
//日期：2020-12
//版本：v1.0
//#define DEBUG 1
__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev __ALIGN_END;
__IO uint32_t receive_count = 0;
uint16_t VirtAddVarTab[NB_OF_VAR];
uint8_t AT24CXX_exist_flag = 0;
uint8_t ble_connection_status = 0;
uint16_t fan1_speed, fan2_speed;
uint8_t fan_onoff_retry = 0;
uint8_t usb_rxbuf[64];
uint16_t uvlamp_fan_status = 0;
uint8_t uv_restart_delay_flag = 0;
uint8_t uv_restart_delay_count = 0;

extern uint16_t log_event_status;
extern uint8_t USART_RX_BUF[USART_REC_LEN]; 
extern uint8_t uart5_frame_end;
extern uint8_t uart5_recv_len;
extern u8 USART1_length_value;
extern uint32_t FAN1_Frequency ;
extern uint32_t FAN2_Frequency ;
extern uint8_t app_config_flag;
extern uint8_t ble_connection_status;
extern uint8_t app_config_cnt;

//主函数
int main(void)
{ 
	u8 tab_main=0;
	u16 i;
	uint8_t usb_txbuf[128];
	uint8_t usb_sendlen, usb_recvlen = 64;
	time_t event_time;
//	float data;//温度数据
	int16_t temp_data;//转换后的实际温度值
	uint8_t uv_retry_times = 0;
	EVENT_DATA lamp_setting_err;
	uint8_t lamp_setting_err_report_flag = 0;
	uint16_t btfwver;
	uint8_t bt_ver[9];
	uint16_t last_temperature_data=0;
	uint8_t retry_times = 0;
//	uint8_t event_buf[5];
	
//	uint16_t data_write[2], data_read[2];
//	RTC_DateTypeDef RTC_DateStruct;
//	u8 temp=0,time_stop_flag=0;
	
	SCB->VTOR = FLASH_BASE | 0x10000;//设置偏移量
	__enable_irq();//使能中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);		//延时初始化 
	uart_init(115200);	//串口初始化波特率为115200
  uart1_init(230400);
  IIC_Init();         //IIC初始化
  PIR_Init();         //PIR初始化
	LED_Init();		  		//初始化与LED连接的硬件接口 
  UV_Init();          //UV灯初始化
	
  TIM7_Int_Init(10000-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数10000次为1000ms  
  TIM9_Int_Init(10000-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数10000次为1000ms  
	TIM10_Int_Init(10000-1,84-1);  
	TIM4_Int_Init(10000-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数10000次为1000ms  
//	TIM5_Int_Init(5000-1,8400-1); 	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms  
	TIM5_Int_Init(1000-1,8400-1); 	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数1000次为100ms 
	TIM6_Int_Init(10000-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数10000次为1000ms  
	//Initialize three PWM for triple-color Pilot Lamp control
  TIM3_PWM_Init(500-1,84-1);	//84M/84=1Mhz的计数频率,重装载值500，所以PWM频率为 1M/500=2Khz.
	TIM12_PWM_Init(500-1,84-1);	//84M/84=1Mhz的计数频率,重装载值500，所以PWM频率为 1M/500=2Khz.
	TIM12_CH2_PWM_Init(500-1,84-1);	//84M/84=1Mhz的计数频率,重装载值500，所以PWM频率为 1M/500=2Khz.
  FAN_Init(); 
	delay_ms(10);
	TIM13_Init();
	TIM14_Init();
	KEY_Init();	
	My_RTC_Init();		 		//初始化RTC
	RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,0);		//配置WAKE UP中断,1秒钟中断一次
	IWDG_Init(6, 4000);
  MYDMA_Config(DMA2_Stream5,DMA_Channel_4,(u32)&USART1->DR,(u32)SendBuff,SEND_BUF_SIZE);//DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
  MYDMA_Enable(DMA2_Stream5,SEND_BUF_SIZE);	//开启一次DMA传输
	//USB Virtual COM init
	USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);

	printf("Enter MCU APP!\r\n");
	printf("<------Firm Version: V%s ------>\r\n",MCUFirm_Version);

/***single double mode sw detect***/
	if((SW5 == 0) && (SW6 == 0)){
		single_double_lamp_flag = 0;
	}	
	else if((SW5 == 1) && (SW6 == 1)){
		single_double_lamp_flag = 1;
	}
	else {
		single_double_lamp_flag = 2;
		//log report ?? 
		printf("Warning : SW5 and SW6 should be set to the same position!!!");
	}
	printf("single_double_lamp_flag = %d\r\n", single_double_lamp_flag);

/***Check if AT24C64 EEPROM is exist***/	
	for(i = 0; i < 10; i++)
	{
		if(AT24CXX_Check()== 0)
		{
			AT24CXX_exist_flag = 1;
			break;
		}
		delay_ms(100);
	}
/***Use internal Flash instead of EEPROM if EEPROM Check failed***/	
	if(AT24CXX_exist_flag == 0)
	{
		printf("Can not find AT24CXX eeprom!!\r\n");
		printf("Emulation EEPROM will be used\r\n");
//		for(i = 0; i< NB_OF_VAR; i++)
//		{
//			VirtAddVarTab[i] = i;
//		}
		if(EEPROM_Check() == 1)
			printf("EEPROM error!\r\n");
	}
/***Read Config data from EEPROM or Flash***/	
	Read_Config_Data();
	btfwver = Get_BLE_FWVER();
	sprintf((char*)bt_ver, "%d.%d.%d.%d", (btfwver>>12)&0xf, (btfwver>>8)&0xf, (btfwver>>4)&0xf, btfwver&0xf);
/***Output firm version info through usb serial port***/	
	usb_sendlen = sprintf((char *)usb_txbuf, "<------MCU Firm Version: V%s ------>\r\n<------BLE Firm Version: V%s ------>\r\nSystem boot ok!\r\n",MCUFirm_Version, bt_ver);
	while (VCP_CheckDataSent() == 1);
	VCP_SendData(&USB_OTG_dev, usb_txbuf, usb_sendlen);
		
/***Usb receive buffer init***/	
	memset(usb_rxbuf, 0, sizeof(usb_rxbuf));
	VCP_ReceiveData(&USB_OTG_dev, usb_rxbuf, usb_recvlen);
	
	while((SendBuff[5]!=0x20)&&(tab_main==0))   //开启设备首先扫描是否与APP配对成功
	{
		if((FAN1_EN_STS == 0) && (FAN1_Frequency >0))
		{
			FAN1_Frequency = 0;
		}
		if((FAN2_EN_STS == 0) && (FAN2_Frequency >0))
		{
			FAN2_Frequency = 0;
		}
		KEY_Scan();    //按键扫描
		if(SendBuff[0]==0xaa&&SendBuff[1]==0x55&&Stop_send_flag==0)
		{
			if(USART1_length_value > 0)
			{
				printf("Received data: ");
				for(i = 0 ; i< USART1_length_value; i++)
					printf(" 0x%2x ", SendBuff[i]);
				printf("\r\n");
				USART1_length_value = 0;
			}
			 switch(SendBuff[5])//获取命令字
			 {
				 case 0x30:
					 if(BurnIn_mode == 0)
						 printf("Device init CMD is received\r\n");
					 Device_Init();
					 tab_main=1;
					 break;           //设备初始化配置
				 case 0x20:
					 if(BurnIn_mode == 0)
						 printf("Get all Status CMD is received\r\n");
					 break;
				 case 0x02:
					 if(BurnIn_mode == 0)
						  printf("Report Netlink CMD is received\r\n");
					 Connect_Status();
					 break; 
			 }			 
		}
		if(dev_data.First_Connect_Flag)
			break;
#if 1
/***Receive USB command and process***/
		if(VCP_CheckDataReceived() != 0 )
		{
			uint8_t ret;
			
			VCP_ReceiveData(&USB_OTG_dev, usb_rxbuf, usb_recvlen);
			ret = TestCommand_Parse(usb_rxbuf, receive_count);
			receive_count = 0;
			memset(usb_rxbuf, 0,  sizeof(usb_rxbuf));
			if( ret == 0)
			{
				if(BurnIn_mode == 0)
				{
					BurnIn_mode = 1;
					turn_all_off();
				}
			}
			if( ret == 1)
			{
				//printf("CMD Error\r\n");
				while (VCP_CheckDataSent() == 1);
				VCP_SendData(&USB_OTG_dev, (uint8_t *)"CMD Error\r\n", sizeof("CMD Error\r\n"));
			}
			else if(ret == 2)
			{
				//printf("Parameter Error\r\n");
				while (VCP_CheckDataSent() == 1);
				VCP_SendData(&USB_OTG_dev, (uint8_t *)"Parameter Error\r\n", sizeof("Parameter Error\r\n"));
			}
//			}
//			else
//			{
//				receive_count = 0;
//				printf("received wrong data\r\n");
//			}
		}
#endif
//		if(dev_data.First_Connect_Flag == 0)
//		{
//			turnON_RED();
//			delay_ms(300);
//			turn_all_off();
//			delay_ms(300);
//		}
//		else
//		{
//			if(BurnIn_mode == 0)
//				printf("first_connect_flag == 1\r\n");
//			break;
//		}
//		if(BurnIn_mode == 1)
//		{
//			if(uart5_frame_end == 1)
//				TestCommand_Parse();
//		}
		IWDG_Feed();
	}
	
	if(dev_data.First_Connect_Flag == 0)
	{
		dev_data.First_Connect_Flag = 1;
		if(AT24CXX_exist_flag == 0)
		{
			EEPROM_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
		}
		else
		{
			AT24CXX_WriteBytes(FIRST_CONNECT_FLAG_ADDR, &dev_data.First_Connect_Flag, 1);
		}
	}
	if(BurnIn_mode)
		BurnIn_mode = 0;
	
	if(dev_data.Stealth_In_Standby == 1)
	{
		turn_all_off();
		pilot_lamp_mode = STEALTH_MODE;
	}
	else
	{
		turnON_GREEN();
		pilot_lamp_mode = P8_STANDBY_NON_STEALTH;
	}
	
	//RTC clock config and update When device bootup
	if(app_config_flag == 1)
	{
		RTC_update();
	}
	if(single_double_lamp_flag == 2 )
	{
		lamp_setting_err_report_flag = 1;
		lamp_setting_err.event_type = Lamp_Setting_Error;
		lamp_setting_err.event_time = RTC_To_UTC();
		if(ble_connection_status == 1)
		{
			uint8_t event_buf[5];
			event_buf[0] = lamp_setting_err.event_type;
			event_buf[1] = (lamp_setting_err.event_time >> 24) & 0xff;
			event_buf[2] = (lamp_setting_err.event_time >> 16) & 0xff;
			event_buf[3] = (lamp_setting_err.event_time >> 8) & 0xff;
			event_buf[4] = lamp_setting_err.event_time & 0xff;
			event_reporting(event_buf, 1);
			lamp_setting_err_report_flag = 0;
		}
	}
	
	for(i = 0; i< 10; i++)
	{
		last_temperature_data += 0.125*(I2C_LM75read());
	}
	last_temperature_data =  last_temperature_data/10;
	
//	Get_BLE_Status();
	while(1)
	{	
		KEY_Scan();    //按键扫描
//		GET_time_update();//每隔3600秒获取一次APP时间
		
    if((Stop_send_flag == 0) && (USART1_length_value > 0) )
    {
			printf("Received %d bytes data: ", USART1_length_value);
			for(i = 0 ; i< USART1_length_value; i++)
				printf(" 0x%2x ", SendBuff[i]);
			printf("\r\n");
			
			if(SendBuff[0]==0xaa && SendBuff[1]==0x55)
			{
#if 0
				if((SendBuff[5] != 0x02) && (SendBuff[5] != 0x03) && (SendBuff[5] != 0x0a) && (SendBuff[5] != 0x0b) &&(ble_connection_status == 0) )
				{
						ble_connection_status = 1;
//					dev_data.First_Connect_Flag = 1;
				}
#endif
				switch(SendBuff[5])//获取命令字
				{   
					case 0x01:Search_Info();break;           //查询产品信息
					case 0x02:Connect_Status();break;        //报告模块网络状态
          case 0x03:BLE_Mesh_Response();break;     //配置BLE_Mesh Response
  //        case 0x04:Get_Time();break;              //获取本地时间
					case 0x06:OTA_Ask();break;             	 //MCU OTA版本请求
					case 0x07:OTA_Update_Notice();break;   	 //MCU OTA升级通知
	//          case 0x08:OTA_Ask_Content();break;     //MCU OTA请求内容
	//          case 0x09:OTA_Update_Result();break;   //OTA升级结果
					case 0x20:GetAll_Status();break;		   	 //读取MCU全状态数据
	//				case 0x21:Switch_UV();break;	           //控制UV灯开关
					case 0x22:Device_Mode();break;           //设备模式 
					case 0x23:Unoccupied_Mode();break;       //空闲模式
					case 0x24:Smart_Mode();break;            //智能模式 
					case 0x25:Life_Time();break;             //灯的寿命
					case 0x27:Get_Temp(); break;
					case 0x28:Plan(); break;                  //计划
					case 0x29:Get_Plan();break;
					case 0x2B:Manual_Mode();break;					 //手动控制按键模式
					case 0x2C:Manual_Control_delay();break;	 //手动开启后的持续时间
					case 0x2D:Duty_Cycle();break;            //Duty_Cycle
					case 0x2E:EOL_Config();break; 					//EOL config
					case 0x2F:Identify_Device();break;       //指示灯打开
					case 0x30:
						Device_Init();
						if(app_config_flag == 0)
						{
							app_config_flag = 1;
						}
						break;           //设备初始化配置
					case 0x31:Pilot_Lamp_brightness();break; //指示灯亮度调节
					case 0x32:Resume_schedule();break;//恢复schedule执行
					case 0x33:single_lamp();break;//单灯模式
					case 0x34:Stealth_Mode_In_Standby();break; //隐身模式
					case 0x35:Send_CurrentTime();break;
					case 0xA0:Query_Registration_Tag();break;
					case 0xA1:Query_Warranty_StartDate();break;
					case 0xA2:Set_Warranty_Period();break;
					case 0xE0:BurnIn_Mode();break;
				}				
			}
			USART1_length_value = 0;
    }
		
		if(dev_data.First_Connect_Flag){
			//if(key_schedule_flag==0)//该判断是因为如果schedule在执行过程中，如果APP设置了开启/关闭按键是否终止schedule功能所做的判断语句
			if((manual_schedule_flag == 0) && (single_double_lamp_flag < 2))
			{
				if((execute_Plan_flag==1) && (uv_restart_delay_flag == 0) && (manual_key_off_flag == 0))
				{	
					execute_Plan();//执行计划
					execute_Plan_flag=0;//标志位置0，等待下一次执行
				}
			}
			else//终止schedule
			{
				if(Plan_running)
				{
					default_set();
					Plan_running = 0;
				}
			}	 
		}
#if 0		
		if(log_flag2==1)
		{			 
			temp_Item_Update();
			BLErst_Item_Update();
			if((Device_mode==MODE_CMNS)||(Device_mode==MODE_CMS))
			{
				if((log_event_status & (1<<Lamp_TurnOn_Failure)) == 0)
				{
					UV_Item_Update();
				}
				if(UV1_open_flag || UV2_open_flag)
					fan_Item_Update();
			}
			log_flag2=0;
			//printf("temp:%d,temp1:%d \r\n",adcx,adcx1);
		}
		
		if(log_flag==1)
		{
			//读取UV1是否处于打开状态  如果UV1打开，则定时器9使能，UV1_open_flag置1
			if(LAMP_ON_A==0)
			{
//				TIM_Cmd(TIM9,ENABLE); //使能定时器9
				UV1_open_flag=1;
				if(BurnIn_mode == 0)
					printf("UV1 is open\r\n");
			}
			else
			{
				UV1_open_flag=0;
				FAN1_Frequency = 0;		
				if(BurnIn_mode == 0)
					printf("UV1 is close\r\n");
			}
			//读取UV2是否处于打开状态  如果UV2打开，则定时器9使能，UV2_open_flag置1
			if(LAMP_ON_B==0)
			{
//				TIM_Cmd(TIM9,ENABLE); //使能定时器9
				UV2_open_flag=1;
				if(BurnIn_mode == 0)
					printf("UV2 is open\r\n");
			}
			else
			{
//			TIM_Cmd(TIM9,DISABLE); //失能定时器9
				UV2_open_flag=0;
				FAN2_Frequency = 0;
				if(BurnIn_mode == 0)
					printf("UV2 is close\r\n");
			}
			temp_Item_Update();
			if(UV1_open_flag || UV2_open_flag)
				fan_Item_Update();
			BLErst_Item_Update();
			UV_Item_Update();
//			Get_time_temp_flag=0;
//			Get_time_UV_flag=0;
//			Get_time_BLE_flag=0;
//			Get_time_fan_flag=0;
			log_flag=0;
		}
#else
		/*****Added by pngao for event check, report and log *****/
		if(log_flag==1)
		{
			//printf("Device_mode = %d\r\n", Device_mode);
			//读取UV1是否处于打开状态  如果UV1打开，则定时器9使能，UV1_open_flag置1
			if(LAMP_ON_A==0)
			{
				if(UV1_open_flag == 0)
				{
//					TIM_Cmd(TIM9,ENABLE); //使能定时器9
					UV1_open_flag=1;
					printf("UV1 is open\r\n");
				}
			}
			else
			{
				if(UV1_open_flag == 1)
				{
					UV1_open_flag=0;
					FAN1_Frequency = 0;		
					printf("UV1 is close\r\n");
				}
			}
			//读取UV2是否处于打开状态  如果UV2打开，则定时器9使能，UV2_open_flag置1
			if(LAMP_ON_B==0)
			{
				if(UV2_open_flag == 0)
				{
//					TIM_Cmd(TIM9,ENABLE); //使能定时器9
					UV2_open_flag=1;
					printf("UV2 is open\r\n");
				}
			}
			else
			{
				if(UV2_open_flag == 1)
				{
//			TIM_Cmd(TIM9,DISABLE); //失能定时器9
					UV2_open_flag=0;
					FAN2_Frequency = 0;
					printf("UV2 is close\r\n");
				}
			}
			
			//end test
			//UV lamp turn on failure check and retry 3 times turnon
//		if((uv_lamp_opened != 0) && uv_turnon_check_flag  &&((uvlamp_fan_status &(UV1_TURNON_FAILURE|UV2_TURNON_FAILURE)) != 3 ))
//			if(Plan_running && uv_turnon_check_flag)
			if(((Device_mode == MODE_CMNS)||(Device_mode == MODE_CMS)) && uv_turnon_check_flag)
			{
				uv_turnon_check_flag = 0;
				switch(uv_lamp_opened){
					case 1:
						if(UV1_open_flag == 0)
						{
							printf("single lamp mode UV1 open failed %d times\r\n", uv_retry_times);
							uv_retry_times ++;
							duty_count = 0;
							UV12_status = 0;
							Manual_count_key = 0;
							close_uv_lamp();
							if(uv_retry_times == 3)
							{
								uv_retry_times = 0;
								printf("UV1 can not turn on\r\n");
								uvlamp_fan_status |= UV1_TURNON_FAILURE; 
								//turnon failure log recorder								
								log_event_status |= 1 << Lamp_TurnOn_Failure;
								log_event_data[current_event_cnt].event_type = Lamp_TurnOn_Failure;
								log_event_data[current_event_cnt].event_time = RTC_To_UTC();
								current_event_cnt++;
								if((uvlamp_fan_status & UV2_TURNON_FAILURE) == 0)
								{
									printf("Satrt to turn on UV2");
									uv_restart_delay_flag = 1;
									uv_restart_delay_count = 0;
									//open_single_lamp();
								}
								else
								{
									//clear status
									//Plan_running = 0;
									//execute_Plan_flag = 0;
									if(manual_schedule_flag == 1)
									{
										//TIM_Cmd(TIM7,DISABLE);
										Manual_count_key = 0;
										default_set();
									}
									else
									{
										TIM_Cmd(TIM7,DISABLE);
										duty_count = 0;
										UV12_status = 0;
									}
									printf("UV1 and UV2 both turnon failed\r\n");
								}
							}
							else
							{
								uv_restart_delay_flag = 1;
								uv_restart_delay_count = 0;
								UV12_status = 0;
								duty_count = 0;
								Manual_count_key = 0;
								//open_single_lamp();
							}
						}
						else
						{
							printf("single lamp mode UV1 turn on successfully\r\n");
							uv_retry_times = 0;
						}
						break;
					case 2:
						if(UV2_open_flag == 0)
						{
							printf("single lamp mode UV2 open failed %d times\r\n", uv_retry_times);
							uv_retry_times ++;
							duty_count = 0;
							Manual_count_key = 0;
							UV12_status = 0;
							close_uv_lamp();
							if(uv_retry_times == 3)
							{
								uv_retry_times = 0;
								printf("UV2 can not turn on\r\n");
								uvlamp_fan_status |=  UV2_TURNON_FAILURE; 
								//turnon failure log recorder
								log_event_status |= 1 << Lamp_TurnOn_Failure;
								log_event_data[current_event_cnt].event_type = Lamp_TurnOn_Failure;
								log_event_data[current_event_cnt].event_time = RTC_To_UTC();
								current_event_cnt++;
								if((uvlamp_fan_status & UV1_TURNON_FAILURE) == 0)
								{
									printf("Satrt to turn on UV1");
									uv_restart_delay_flag = 1;
									uv_restart_delay_count = 0;
									UV12_status = 0;
									duty_count = 0;
									Manual_count_key = 0;
									//open_single_lamp();
								}
								else
								{
									//clear status
									//Plan_running = 0;
									//execute_Plan_flag = 0;
									if(manual_schedule_flag == 1)
									{
										//TIM_Cmd(TIM7,DISABLE);
										Manual_count_key = 0;
										default_set();
									}
									else
									{
										TIM_Cmd(TIM7,DISABLE);
										duty_count = 0;
										UV12_status = 0;
									}
									printf("UV1 and UV2 both turnon failed\r\n");
								}
							}
							else
							{
								uv_restart_delay_flag = 1;
								uv_restart_delay_count = 0;
								UV12_status = 0;
								duty_count = 0;
								Manual_count_key = 0;
								//open_single_lamp();
							}
						}
						else
						{
							printf("single lamp mode UV2 turn on successfully\r\n");
							uv_retry_times = 0;
						}
						break;
					case 3:
						if(((UV1_open_flag == 0) && ((uvlamp_fan_status & UV1_TURNON_FAILURE) == 0) && (UV1_Life_Hours < dev_data.UV_EOL_MAXIMUM))
							||(( UV2_open_flag == 0) &&((uvlamp_fan_status & UV2_TURNON_FAILURE) == 0) && (UV2_Life_Hours < dev_data.UV_EOL_MAXIMUM)) )
						{
							printf("double lamp mode uv open failed %d times\r\n", uv_retry_times);
							uv_retry_times ++;
							duty_count = 0;
							Manual_count_key = 0;
							UV12_status = 0;
							close_uv_lamp();
							if(uv_retry_times == 3)
							{
								//clear status
								if(UV1_open_flag == 0)
								{
									printf("UV1 can not turn on\r\n");
									uvlamp_fan_status |= UV1_TURNON_FAILURE; 
								}
								if(UV2_open_flag == 0)
								{
									printf("UV2 can not turn on\r\n");
									uvlamp_fan_status |= UV2_TURNON_FAILURE; 
								}
								if((uvlamp_fan_status &(UV1_TURNON_FAILURE|UV2_TURNON_FAILURE)) != 3)
								{
									uv_restart_delay_flag = 1;
									uv_restart_delay_count = 0;
									UV12_status = 0;
									duty_count = 0;
									Manual_count_key = 0;
									//open_double_lamp();
								}
								else
								{
									if(manual_schedule_flag == 1)
									{
										TIM_Cmd(TIM7,DISABLE);
										default_set();
									}
									else
									{
										TIM_Cmd(TIM7,DISABLE);
										duty_count = 0;
										UV12_status = 0;
									}
								}
//								else
//								{
//									Plan_running = 0;
//									execute_Plan_flag = 0;
//								}
								//turnon failure log recorder
								log_event_status |= 1 << Lamp_TurnOn_Failure;
								log_event_data[current_event_cnt].event_type = Lamp_TurnOn_Failure;
								log_event_data[current_event_cnt].event_time = RTC_To_UTC();
								current_event_cnt++;
								uv_retry_times = 0;
							}
							else
							{
								uv_restart_delay_flag = 1;
								uv_restart_delay_count = 0;
								UV12_status = 0;
								duty_count = 0;
								Manual_count_key = 0;
								//open_double_lamp();
							}
						}
						break;
				}
			}
			if(uv_restart_delay_flag == 2)
			{
				printf("uv lamp restart\r\n");
				uv_restart_delay_flag = 0;
				if(single_double_lamp_flag)
				{
					open_single_lamp();
				}
				else
				{
					open_double_lamp();
				}
			}
			//BLE Reset check
			if(BLErst_flag==1) 
			{
				printf("BLErst log recorder\r\n");
				log_event_data[current_event_cnt].event_type = BLE_Module_Reset;
				event_time = RTC_To_UTC();
				log_event_data[current_event_cnt].event_time = event_time;
				current_event_cnt++;
				BLErst_flag = 0;
			}
			//Temp check
			
			temp_data = 0.125*(I2C_LM75read());
			retry_times = 0;
			while((temp_data > dev_data.High_Temp_Thresh_ERROR) && (last_temperature_data < dev_data.High_Temp_Thresh_Warning))
			{
				temp_data = 0.125*(I2C_LM75read());
				retry_times++;
				if(retry_times == 5)
					break;
				delay_ms(10);
			}
			
			last_temperature_data = temp_data;
			//printf("temp = %d\r\n", temp_data);
			//temp_data = data*10/10;//实际温度值

			if(temp_data>= dev_data.High_Temp_Thresh_Warning)
			{
				if(temp_data > dev_data.High_Temp_Thresh_ERROR)
				{
					printf("High_Temp_Thresh_ERROR : %d\r\n",temp_data);
					if((log_event_status & (1 << Ambient_temperature_Error)) == 0)
					{
						log_event_status |= 1 << Ambient_temperature_Error;
						log_event_data[current_event_cnt].event_type = Ambient_temperature_Error;
						event_time = RTC_To_UTC();
						log_event_data[current_event_cnt].event_time = event_time;
						current_event_cnt++;
					}	
					uvlamp_fan_status |= TEMP_ERROR;
					if(manual_schedule_flag == 1)
					{
						//TIM_Cmd(TIM7,DISABLE);
						Manual_count_key = 0;
						default_set();
					}
				}
				else
				{
					printf("High_Temp_Thresh_Warning : %d\r\n",temp_data);
					if((log_event_status & (1 << Ambient_Temperature_Warning)) == 0)
					{
						if(log_event_status & (1 << Ambient_temperature_Error))
						{
							log_event_status &= ~(1 << Ambient_temperature_Error);				
						}
						log_event_status |= 1 << Ambient_Temperature_Warning;
						log_event_data[current_event_cnt].event_type = Ambient_Temperature_Warning;
						event_time = RTC_To_UTC();
						log_event_data[current_event_cnt].event_time = event_time;
						current_event_cnt++;
					}
					log_event_status &= ~(1<<Ambient_temperature_Error);
				}				
			}
			else
			{
				if((log_event_status &(1<<Ambient_Temperature_Warning | 1<<Ambient_temperature_Error )) != 0 )
				{
					log_event_status &= ~(1<<Ambient_Temperature_Warning | 1<<Ambient_temperature_Error);
				}
				if((uvlamp_fan_status & TEMP_ERROR) && (temp_data < dev_data.High_Temp_Thresh_Warning))
				{
					uvlamp_fan_status &= ~TEMP_ERROR;
				}
			}
			
			//转速检测
			if((UV1_open_flag || UV2_open_flag) && (uv_restart_delay_flag == 0))
			{
				if(FAN1_EN_STS)
				{
					fan1_speed = FAN1_Frequency * 60;
				}
//				else
//				{
//					FAN1_Frequency = 0;
//					fan1_speed = 0;
//				}
				if(FAN2_EN_STS)
				{
					fan2_speed = FAN2_Frequency * 60;
				}
//				else
//				{
//					FAN2_Frequency = 0;
//					fan2_speed = 0;
//				}	
				printf("fan1_speed = %d fan2_speed = %d\r\n", fan1_speed, fan2_speed);
				if(((UV1_open_flag && (fan1_speed ==0))||(UV2_open_flag && (fan2_speed ==0)))&& ((uvlamp_fan_status & (FAN1_SPEED_ERROR|FAN2_SPEED_ERROR)) == 0))
				{
					if((log_event_status &(1<<Fans_Speed_Error)) == 0)
					{
						log_event_status |= 1 << Fans_Speed_Error;					
						fan_onoff_retry++;
						if(UV1_open_flag && fan1_speed ==0)
						{
							FAN1_ENABLE = 0;
							delay_ms(50);
							FAN1_ENABLE = 1;
							delay_ms(50);
						}
						if(UV2_open_flag && fan2_speed ==0)
						{
							FAN2_ENABLE = 0;
							delay_ms(50);
							FAN2_ENABLE = 1;
							delay_ms(50);
						}
					}
					else
					{
						fan_onoff_retry ++;
						printf("Fans_Speed_Error retry = %d\r\n", fan_onoff_retry);
						if(fan_onoff_retry < 5)
						{
							if(UV1_open_flag && fan1_speed ==0)
							{
								FAN1_ENABLE = 0;
								delay_ms(50);
								FAN1_ENABLE = 1;
								delay_ms(50);
							}
							if(UV2_open_flag && fan2_speed ==0)
							{
								FAN2_ENABLE = 0;
								delay_ms(50);
								FAN2_ENABLE = 1;
								delay_ms(50);
							}
						}
						else
						{
							if(UV1_open_flag && (fan1_speed ==0))
							{
								uvlamp_fan_status |= FAN1_SPEED_ERROR;
							}
							if(UV2_open_flag && (fan2_speed ==0))
							{
								uvlamp_fan_status |= FAN2_SPEED_ERROR;
							}
							fan_onoff_retry  = 0;
							//close UV Lamp
							//close_uv_lamp();
							//Plan_running = 0;
							if((uvlamp_fan_status &(FAN1_SPEED_ERROR|FAN2_SPEED_ERROR)) != 0)
							{
								if(manual_schedule_flag == 1)
								{
									default_set();
								}
								else
								{
									TIM_Cmd(TIM7,DISABLE);
									duty_count = 0;
									UV12_status = 0;
								}							
							}
							//event record
							log_event_data[current_event_cnt].event_type = Fans_Speed_Error;
							event_time = RTC_To_UTC();
							log_event_data[current_event_cnt].event_time = event_time;
							current_event_cnt++;
						}
					}
				}			
				else if((UV1_open_flag && (fan1_speed < dev_data.FAN_Speed_Thresh_Warning))|| (UV2_open_flag && (fan2_speed < dev_data.FAN_Speed_Thresh_Warning)))
				{
					if((log_event_status &(1<<Fans_Speed_Warning)) == 0)
					{
						log_event_status |= 1 << Fans_Speed_Warning;
						log_event_data[current_event_cnt].event_type = Fans_Speed_Warning;
						event_time = RTC_To_UTC();
						log_event_data[current_event_cnt].event_time = event_time;
						current_event_cnt++;
						fan_onoff_retry =0;
					}
				}
				else
				{
					fan_onoff_retry = 0;
					if(log_event_status &(1<<Fans_Speed_Error))
						log_event_status &=  ~(1<<Fans_Speed_Error);
					if(log_event_status &(1<<Fans_Speed_Warning))
						log_event_status &=  ~(1<<Fans_Speed_Warning);
				}
			}	
			else
			{
//				FAN1_Frequency = 0;
//				FAN2_Frequency = 0;
				fan_onoff_retry = 0;
				if(log_event_status &(1<<Fans_Speed_Error))
					log_event_status &=  ~(1<<Fans_Speed_Error);
				if(log_event_status &(1<<Fans_Speed_Warning))
					log_event_status &=  ~(1<<Fans_Speed_Warning);
			}
			
			//UV Lifetime check
			if((UV1_Life_Hours > dev_data.UV_EOL_MAXIMUM) || (UV2_Life_Hours > dev_data.UV_EOL_MAXIMUM) )
			{
				if((log_event_status & (1 << Lamp_Lifetime_Error)) == 0)
				{
					log_event_status |= 1 << Lamp_Lifetime_Error;
					log_event_data[current_event_cnt].event_type = Lamp_Lifetime_Error;
					event_time = RTC_To_UTC();
					log_event_data[current_event_cnt].event_time = event_time;
					current_event_cnt++;
				}
			}
			else if((UV1_Life_Hours > dev_data.UV_EOL_WARNING) || (UV2_Life_Hours > dev_data.UV_EOL_WARNING) )
			{
				printf("Lamp_Lifetime_Warning report\r\n");
				if((log_event_status & (1 << Lamp_Lifetime_Warning)) == 0)
				{
					log_event_status |= 1 << Lamp_Lifetime_Warning;
					log_event_data[current_event_cnt].event_type = Lamp_Lifetime_Warning;
					event_time = RTC_To_UTC();
					log_event_data[current_event_cnt].event_time = event_time;
					current_event_cnt++;
				}
			}
			else
			{
				if(log_event_status & (1 << Lamp_Lifetime_Warning) )
					log_event_status &= ~(1 << Lamp_Lifetime_Warning);
				if(log_event_status & (1 << Lamp_Lifetime_Error) )
					log_event_status &= ~(1 << Lamp_Lifetime_Error);
			}
			
			if((ble_connection_status == 1)&&(app_config_flag == 1))
			{
					//Send Event Log Command to APP
				if(EEPROM_Log_Nums > 0)
				{
					uint8_t logs_left, logs_upload;
					uint8_t *clear_buf;
					logs_left = EEPROM_Log_Nums;
					logs_upload = 0;					
					printf("upload EEPROM log to app\r\n");
					while(logs_left > 0)
					{
						uint8_t *log_buf;	
						if(logs_left > MAX_REPORT_NUM_ONCE)
						{
							log_buf = (uint8_t *)malloc(5*MAX_REPORT_NUM_ONCE+1);
							if(AT24CXX_exist_flag == 0)
							{
								EEPROM_ReadBytes(LOG1_ADDR + logs_upload*5, log_buf, 5*MAX_REPORT_NUM_ONCE);
							}
							else
							{
								AT24CXX_ReadBytes(LOG1_ADDR + logs_upload*5, log_buf, 5*MAX_REPORT_NUM_ONCE);
							}
							//Send event reporter to APP
							if(event_reporting(log_buf, MAX_REPORT_NUM_ONCE) == 0)
							{
								printf("event_reporting succeed\r\n");
							}
							else
							{
								printf("event_reporting failed\r\n");
							}
							logs_left = logs_left - MAX_REPORT_NUM_ONCE;
							logs_upload += MAX_REPORT_NUM_ONCE;
						}
						else
						{
							log_buf = (uint8_t *)malloc(5*logs_left+1);
							if(AT24CXX_exist_flag == 0)
							{
								EEPROM_ReadBytes(LOG1_ADDR + logs_upload*5, log_buf, 5*logs_left);
							}
							else
							{
								AT24CXX_ReadBytes(LOG1_ADDR + logs_upload*5, log_buf, 5*logs_left);
							}
							//Send event reporter to APP
							event_reporting(log_buf, logs_left);
							logs_left = 0;
							logs_upload = EEPROM_Log_Nums;
						}						
						free(log_buf);
					}	
					//clear event logs in EEPROM
					
					clear_buf = malloc(5*EEPROM_Log_Nums+1);
					memset(clear_buf, 0x0, 5*EEPROM_Log_Nums+1);
					if(AT24CXX_exist_flag == 0)
					{
						EEPROM_WriteBytes(LOG_NUMBER_ADDR, clear_buf, 5*EEPROM_Log_Nums+1);
					}
					else
					{
						AT24CXX_WriteBytes(LOG_NUMBER_ADDR, clear_buf, 5*EEPROM_Log_Nums+1);
					}		
					free(clear_buf);
					EEPROM_Log_Nums = 0;
					log_cur_pos = 0;
				}
				if(lamp_setting_err_report_flag)
				{
					uint8_t event_buf[5];
					event_buf[0] = lamp_setting_err.event_type;
					event_buf[1] = (lamp_setting_err.event_time >> 24) & 0xff;
					event_buf[2] = (lamp_setting_err.event_time >> 16) & 0xff;
					event_buf[3] = (lamp_setting_err.event_time >> 8) & 0xff;
					event_buf[4] = lamp_setting_err.event_time & 0xff;
					event_reporting(event_buf, 1);
					lamp_setting_err_report_flag = 0;
				}
				if(current_event_cnt > 0)
				{
					//Send event reporter to APP
					uint8_t *event_buf;
					printf("upload event log : event_num=%d\r\n ", current_event_cnt);
					event_buf = (uint8_t *)malloc(5*current_event_cnt);
					for(i = 0 ; i< current_event_cnt; i++)
					{
						event_buf[5*i] = log_event_data[i].event_type;
						event_buf[5*i + 1] = (log_event_data[i].event_time >> 24) & 0xff;
						event_buf[5*i + 2] = (log_event_data[i].event_time >> 16) & 0xff;
						event_buf[5*i + 3] = (log_event_data[i].event_time >> 8) & 0xff;
						event_buf[5*i + 4] = log_event_data[i].event_time & 0xff;
					}
					if(event_reporting(event_buf, current_event_cnt) == 1)
					{
						//event reporting failed and saved logs to eeprom
						Save_Logs();
					}
					
					free(event_buf);
					current_event_cnt = 0;
					memset(log_event_data, 0 , sizeof(log_event_data));
				}
				
			}
			else
			{
				Save_Logs();
			}
			FAN1_Frequency = 0;
			FAN2_Frequency = 0;
			fan1_speed = 0;
			fan2_speed = 0;
			log_flag = 0;
		}
		if((dev_data.First_Connect_Flag == 1) && (app_config_cnt >= 90) &&(app_config_flag == 0))
		{
					printf("app_config timeout, reset ble module...\r\n");
					PLAN_Clear();
					if(manual_schedule_flag == 0)
					{
						Manual_Schedule_Mode_Set(1);
					}
					BLErst_flag=1;//蓝牙复位置1
					BLE_Mesh(1);
					default_set();
					pilot_lamp_mode = P1_BT_UNPAIR;
					app_config_flag = 1;
					app_config_cnt = 0;				
		}
#endif
		//Record warranty start time
		if((pro_info.product_registration_flag == 0) &&(Warranty_start_flag == 0))
		{
			if((single_double_lamp_flag &&((UV1_Life_Hours >= 1)||(UV2_Life_Hours >= 1) || ((UV1_Life_Hours == 0)&& (UV2_Life_Hours == 0) && ((UV1_life_min_count + UV2_life_min_count) >= 60)))) ||
				((single_double_lamp_flag == 0)&&(UV1_Life_Hours >= 1) &&(UV2_Life_Hours >= 1)))
			{
				uint32_t warranty_start;
				RTC_DateTypeDef rtc_date;
				char warranty_start_string[9];
				uint8_t wars_data[4];
				
				//uint8_t tmp_buf[4];
				RTC_GetDate(RTC_Format_BIN, &rtc_date);
				Warranty_start_flag = 1;
				sprintf(warranty_start_string, "%04d%02d%02d", rtc_date.RTC_Year+2000, rtc_date.RTC_Month, rtc_date.RTC_Date);
				warranty_start_string[8] = '\0';
				warranty_start = atoi(warranty_start_string);
				//warranty_start = (uint32_t)RTC_To_UTC();
				
				pro_info.product_warranty_start_date = warranty_start;
				wars_data[0] = (uint8_t)((warranty_start >>24)&0xff);
				wars_data[1] = (uint8_t)((warranty_start >>16)&0xff);
				wars_data[2] = (uint8_t)((warranty_start >>8)&0xff);
				wars_data[3] = (uint8_t)(warranty_start&0xff);
				printf("warranty_start %d\r\n", warranty_start);
				if(AT24CXX_exist_flag == 0)
				{
					EEPROM_WriteBytes(PRODUCT_WARRANTY_START_UTC_ADDR, wars_data, 4);
					EEPROM_WriteBytes(PRODUCT_WARRANTY_START_FLAG, &Warranty_start_flag, 1);
				}
				else
				{
					AT24CXX_WriteBytes(PRODUCT_WARRANTY_START_UTC_ADDR, wars_data, 4);
					AT24CXX_WriteBytes(PRODUCT_WARRANTY_START_FLAG, &Warranty_start_flag, 1);
				}
			}
		}

//		if(BurnIn_mode == 1)
//		{
#if 1
			//printf("Uart5 Received %d bytes: %s\r\n", uart5_recv_len, USART_RX_BUF);
			
			if(VCP_CheckDataReceived() != 0 )
			{
				uint8_t ret, data_len;
				VCP_ReceiveData(&USB_OTG_dev, usb_rxbuf, usb_recvlen);
				data_len = receive_count;
				receive_count = 0;
//				printf("Received %s\r\n", usb_rxbuf);
				ret = TestCommand_Parse(usb_rxbuf, data_len);				
				if( ret == 0)
				{
					if(BurnIn_mode == 0)
					{
						BurnIn_mode = 1;
						turn_all_off();
					}
				}
				if( ret == 1)
				{
					while (VCP_CheckDataSent() == 1);
					VCP_SendData(&USB_OTG_dev, (uint8_t *)"CMD Error\r\n", sizeof("CMD Error\r\n"));
				}
				else if(ret == 2)
				{
					while (VCP_CheckDataSent() == 1);
					VCP_SendData(&USB_OTG_dev, (uint8_t *)"Parameter Error\r\n", sizeof("Parameter Error\r\n"));
				}
				memset(usb_rxbuf, 0,  sizeof(usb_rxbuf));
			}
#else
			//use uart5 as product test port
			if(uart5_frame_end == 1)
			{
				uint8_t ret;
				ret = TestCommand_Parse(USART_RX_BUF, uart5_recv_len);
				if( ret == 1)
				{
					printf("CMD Error\r\n");
				}
				else if(ret == 2)
				{
					printf("Parameter Error\r\n");
				}
				
				memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
				uart5_frame_end = 0;
				uart5_recv_len = 0;
			}
#endif
//		}
		delay_ms(10);
		IWDG_Feed();
    /****************************************/ 
	}
}


