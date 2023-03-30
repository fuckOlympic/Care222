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
#include "key.h"
#include <stdlib.h>
#include <time.h>
u8 str_all[40]={0x55,0xAA,0x01,0x00,0x01};  //֡��ʽ
u8 Stop_send_flag=0;   //ֹͣ����ָ��״̬��־λ
u32 SN_count=0;//���к���������
//�ƻ���������
u8 Plan_buffer[60]={0};   //���ʱ�����������
u8 Plan_length=0;   //�ƻ������ݳ���
u8 Plan_happen=0;   //�ƻ�����������־λ
u8 BLE_PLAN_tab_flag=0;//BLEģ��ͼƻ�ģʽ�ڶ�ʱ��9�л�ʹ�ñ�־λ
RTC_TimeTypeDef RTC_TimeStruct;
u32 RTC_time_value=0;//RTCʱ����ֵ����
//u8 Plan_open_hour=0,Plan_open_minte=0,Plan_close_hour=0,Plan_close_minte=0,Plan_mode=0;//���üƻ��͹رռƻ�ʱ������
//u8 Plan_run_hour=0,Plan_run_minte=0,Plan_stop_hour=0,Plan_stop_minte=0,Plan_run_stop_mode=0;//�����üƻ�ʱ������
  
//�豸״̬����
u8 Device_status=0;
u8 Device_mode=6;

//�豸��ʼ���������ݱ���
u8 Device_data[18]={0};
//Duty_Cycleֵ�����������
u16 Duty_cycle_write_buff[2]={0};
u16 Duty_cycle_read_buff[2]={0};
//Duty_cycle��״̬����ֵ
u8 Duty_cycle_status_value[2]={0};
//PIR��������־λ����
u8 PIR_TIM3_flag=0;//�ñ��������л��������Ϳ���ģʽת������

time_t tick; //����һ���ʺϴ洢����ʱ������
struct tm tm;//����һ����������ʱ������ڵĽṹ
char s[100];

//��ȡ��Ʒ��ǰ״̬
//������0X20
void GetAll_Status(void)
{
  float data;
  char temp_data;
  
 u8 str_all_status[16]={0x55,0xAA,0x01,0x00,0x20,0x20,0x08,0x00,0x00};
 
  data=0.125*(I2C_LM75read());
  temp_data=data*10/10;
	str_all_status[7]=Device_status;//�豸�Ŀ���״̬
  str_all_status[8]=Device_mode;//�豸�Ĺ���ģʽ
  str_all_status[9]=0x00;
  str_all_status[10]=temp_data+20;
	 str_all_status[11]=0x00;
	 str_all_status[12]=0x00;
	 str_all_status[13]=0x00;
	 str_all_status[14]=0x00;
 SN_count++;
 str_all_status[3]=0x00;
 str_all_status[4]=SN_count;
  str_all_status[15]=Sum_Rem(str_all_status,sizeof(str_all_status));
 if(SendBuff[5]==0x20)
  Usart1_Send(str_all_status,sizeof(str_all_status));
 Stop_send_flag=1;     //ֹͣ����ָ��״̬��־λ
}

//��ѯ��Ʒ��Ϣ
void Search_Info(void)
{
  u8 str_all_info[33]={0x55,0xAA,0x01,0x00,0x01};  //֡��ʽ
  u8 str_version[25]={'{','"','p','"',':','"','a','k','e','s','o','"',',','"','v','"',':','"','1','.','0','.','0','"','}'};
    u8 i=0,temp=7;
   str_all_info[5]=0x01;
   str_all_info[6]=0x19;
    SN_count++;
    str_all_info[3]=0x00;
    str_all_info[4]=SN_count;
     for(i=0;i<sizeof(str_version);i++)
      {
        str_all_info[temp++]=str_version[i];
      }
      str_all_info[32]=Sum_Rem(str_all_info,sizeof(str_all_info));
      if(SendBuff[5]==0x01)
      {
        Usart1_Send(str_all_info,sizeof(str_all_info));
      }
      Stop_send_flag=1;    //ֹͣ����ָ��״̬��־λ
}
//����ģ������״̬
void Connect_Status(void)
{
  u8 str_connect_status[9]={0x55,0xAA,0x01,0x00,0x02,0x02,0x01,0x01};  //֡��ʽ
  delay_ms(10);
  SN_count++;
    str_connect_status[3]=0x00;
    str_connect_status[4]=SN_count;
  if(SendBuff[7]==0x01)
  {
    GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���õͣ�����R
    GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
    GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���õͣ�����B
    str_connect_status[7]=0x01;
    str_connect_status[8]=Sum_Rem(str_connect_status,sizeof(str_connect_status));
    Usart1_Send(str_connect_status,sizeof(str_connect_status));
  }
  else
  {
    GPIO_SetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
    GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���õͣ�����G
    GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���õͣ�����B
    str_connect_status[7]=0x00;
    str_connect_status[8]=Sum_Rem(str_connect_status,sizeof(str_connect_status));
    Usart1_Send(str_connect_status,sizeof(str_connect_status));
  }
  
  Stop_send_flag=1;     //ֹͣ����ָ��״̬��־λ
}
//����UV�ƿ���   ������δʹ�ã�����
void Switch_UV(void)
{
    u8 str_Switch_UV[9]={0x55,0xAA,0x01,0x00,0x05,0x21,0x01};  //֡��ʽ
    SN_count++;
    str_Switch_UV[3]=0x00;
    str_Switch_UV[4]=SN_count;
    if(SendBuff[7]==0x00)
    {
      UV1=1;
      UV2=1;
      str_Switch_UV[7]=0X00;
      str_Switch_UV[8]=Sum_Rem(str_Switch_UV,sizeof(str_Switch_UV));
       Usart1_Send(str_Switch_UV,sizeof(str_Switch_UV));
//      //
//       Usart1_Send(my_str,10);
//      //
    }
    else
    {
      UV1=0;
      UV2=0;
      str_Switch_UV[7]=0x06;
      str_Switch_UV[8]=Sum_Rem(str_Switch_UV,sizeof(str_Switch_UV));
      Usart1_Send(str_Switch_UV,sizeof(str_Switch_UV));
    }
    TIM_Cmd(TIM3,DISABLE);
    Stop_send_flag=1;    //ֹͣ����ָ��״̬��־λ
}
//����ģʽ
//�����֣�0x23
void Unoccupied_Mode(void)
{
	u8 str_Unoccupied_Mode[11]={0x55,0xAA,0x01,0x00,0x05,0x23,0x03};  //֡��ʽ
  SN_count++;
  str_Unoccupied_Mode[3]=0x00;
  str_Unoccupied_Mode[4]=SN_count;
  str_Unoccupied_Mode[7]=SendBuff[7];
  str_Unoccupied_Mode[8]=SendBuff[8];
  str_Unoccupied_Mode[9]=SendBuff[9];
  str_Unoccupied_Mode[10]=Sum_Rem(str_Unoccupied_Mode,sizeof(str_Unoccupied_Mode));
  Usart1_Send(str_Unoccupied_Mode,sizeof(str_Unoccupied_Mode));
  Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}
//����ģʽ
//�����֣�0X24
void Smart_Mode(void)
{
  u8 str_Smart_Mode[11]={0x55,0xAA,0x01,0x00,0x05,0x24,0x03};  //֡��ʽ
  SN_count++;
  str_Smart_Mode[3]=0x00;
  str_Smart_Mode[4]=SN_count;
  str_Smart_Mode[7]=0x00;
  str_Smart_Mode[8]=0x0a;
  str_Smart_Mode[9]=0x0a;
  
  str_Smart_Mode[9]=SendBuff[9];
  str_smart_value=(str_Smart_Mode[9]%10);
  str_Smart_Mode[10]=Sum_Rem(str_Smart_Mode,sizeof(str_Smart_Mode));
  Usart1_Send(str_Smart_Mode,sizeof(str_Smart_Mode));
  Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}
//Duty_Cycle
void Duty_Cycle(void)
{
  u8 str_Duty_Cycle[10]={0x55,0xAA,0x01,0x00,0x05,0x2D,0x02};  //֡��ʽ
  SN_count++;
  str_Duty_Cycle[3]=0x00;
  str_Duty_Cycle[4]=SN_count;
  str_Duty_Cycle[7]=0x00;
  str_Duty_Cycle[8]=0x0a;
  str_Duty_Cycle[7]=SendBuff[7];
  str_Duty_Cycle[8]=SendBuff[8];
	if(SendBuff[7]==0x00&&SendBuff[8]==0x00)//����Duty_cycle ֵ
	{
		AT24CXX_Read(1,Duty_cycle_read_buff,2);//�Ӵ洢���ж���Duty_cycleֵ
		str_Duty_Cycle[7]=Duty_cycle_read_buff[0];
		str_Duty_Cycle[8]=Duty_cycle_read_buff[1];
		str_Duty_Cycle[9]=Sum_Rem(str_Duty_Cycle,sizeof(str_Duty_Cycle));
		if(SendBuff[5]==0x2D)
    Usart1_Send(str_Duty_Cycle,sizeof(str_Duty_Cycle));
	}
	else
	{
		Duty_cycle_status_value[0]=str_Duty_Cycle[7];
		Duty_cycle_status_value[1]=str_Duty_Cycle[8];
		str_duty_value=(str_Duty_Cycle[7]*256+str_Duty_Cycle[8])*1.5;//Duty-Cycleֵ����
		Duty_cycle_write_buff[0]=str_Duty_Cycle[7];
		Duty_cycle_write_buff[1]=str_Duty_Cycle[8];
		AT24CXX_Write(1,(u16*)Duty_cycle_write_buff,2);    //AT24LC64д������
		str_Duty_Cycle[9]=Sum_Rem(str_Duty_Cycle,sizeof(str_Duty_Cycle));
		duty_count=0;//TIM3����������0
		TIM_Cmd(TIM3,DISABLE);
	//  TIM_Cmd(TIM9,DISABLE);
		if(SendBuff[5]==0x2D)
		Usart1_Send(str_Duty_Cycle,sizeof(str_Duty_Cycle));
 }
  Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}
//�豸ģʽ 
//������0X22
void Device_Mode(void)
{
  u8 str_Device_Mode[9]={0x55,0xAA,0x01,0x00,0x05,0x22,0x01};  //֡��ʽ
  SN_count++;
  str_Device_Mode[3]=0x00;
  str_Device_Mode[4]=SN_count;
    if(SendBuff[7]==0x00)
    {
      UV1=1;
      UV2=1;
      str_Device_Mode[7]=0x00;
			Device_status=0;
			Device_mode=0;
      str_Device_Mode[8]=Sum_Rem(str_Device_Mode,sizeof(str_Device_Mode));
       Usart1_Send(str_Device_Mode,sizeof(str_Device_Mode));
			AT24CXX_Read(1,Duty_cycle_read_buff,2);//�Ӵ洢���ж���Duty_cycleֵ
			str_duty_value=(Duty_cycle_read_buff[0]*256+Duty_cycle_read_buff[1])*1.5;//Duty-Cycleֵ���㵽��ʱ��ִ��
			 duty_count=0;//TIM3����������0
			TIM_Cmd(TIM3,ENABLE);
    }
    else if(SendBuff[7]==0x06)
    {
      UV1=0;
      UV2=0;
      str_Device_Mode[7]=0x06;
			Device_status=1;
			Device_mode=6;
      str_Device_Mode[8]=Sum_Rem(str_Device_Mode,sizeof(str_Device_Mode));
      Usart1_Send(str_Device_Mode,sizeof(str_Device_Mode));
			duty_count=0;//TIM3����������0
			TIM_Cmd(TIM3,DISABLE);
			TIM_Cmd(TIM4,DISABLE); //ʧ�ܶ�ʱ��4
    }
    Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}
//MCU������ģ�����//��������
void BLE_Mesh(void)
{
  u8 str_BLE_Mesh[9]={0x55,0xAA,0x01,0x00,0xff,0X03,0X01,0X01}; 
  str_BLE_Mesh[8]=Sum_Rem(str_BLE_Mesh,sizeof(str_BLE_Mesh));
  Usart1_Send(str_BLE_Mesh,sizeof(str_BLE_Mesh));
 // delay_ms(500);
}
//��ȡ����ʱ��
void Get_Time(void)
{
  
  u8 str_Get_Time[9]={0x55,0xAA,0x01,0x00,0xff,0X04,0X01,0X01}; 
  SN_count++;
  str_Get_Time[3]=0x00;
  str_Get_Time[4]=SN_count;
  str_Get_Time[8]=Sum_Rem(str_Get_Time,sizeof(str_Get_Time));
//   if(SendBuff[5]==0x04)
  Usart1_Send(str_Get_Time,sizeof(str_Get_Time));
  Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}
//�Ƶ�����
void Life_Time(void)
{
  u8 str_Life_Time[12]={0x55,0xAA,0x01,0x00,0xff,0X25,0X04,0X00,0X00,0X00,0X00}; 
  SN_count++;
  str_Life_Time[3]=0x00;
  str_Life_Time[4]=SN_count;
  str_Life_Time[11]=Sum_Rem(str_Life_Time,sizeof(str_Life_Time));
  if(SendBuff[5]==0x25)
  Usart1_Send(str_Life_Time,sizeof(str_Life_Time));
  Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}
//�ƻ�
//����0X28
void Plan(void)
{
  u16 i=0,temp=0;
  u8 str_Plan[9]={0x55,0xAA,0x01,0x00,0xff,0X28,0X01,0X01}; 
  SN_count++;
  str_Plan[3]=0x00;
  str_Plan[4]=SN_count;
  for(i=0;i<SendBuff[6];i++)
  {
      Plan_buffer[i]=SendBuff[7+i];       //����ƻ�����
  }
	
  Plan_length=SendBuff[6];
  str_Plan[8]=Sum_Rem(str_Plan,sizeof(str_Plan));
 if(SendBuff[5]==0x28)
  Usart1_Send(str_Plan,sizeof(str_Plan));
 delay_ms(100);
 AT24CXX_Read(1,Duty_cycle_read_buff,2);//�Ӵ洢���ж���Duty_cycleֵ
 str_duty_value=(Duty_cycle_read_buff[0]*256+Duty_cycle_read_buff[1])*1.5;//Duty-Cycleֵ���㵽��ʱ��ִ��
 Plan_happen=1;//���û�APP�˴򿪼ƻ�����ʱ���ƻ���������־λ��1
 execute_Plan_flag=1;//������ִ�мƻ�����������־λ
 BLE_PLAN_tab_flag=1;//BLEģ��ͼƻ�ģʽ�ڶ�ʱ��4�л�ʹ�ñ�־λ
// if(Stop_send_flag==0)
//		{
//			for(temp=0;temp<SendBuff[6];temp++)
//			printf("%d,\r\n",Plan_buffer[temp]);
//		}
  Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}

//�ƻ�ִ��
void execute_Plan(void)
{
	u8 i=0,j=0;
	u8 tbuf[40];
	u8 RTC_time[3]={0};
	if(Plan_happen==1)
	  {
			sprintf((char*)tbuf,"%02d%02d%02d",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
			RTC_time_value=atoi((char*)tbuf);
			RTC_time[0]=RTC_time_value/10000;
			RTC_time[1]=RTC_time_value%10000/100;
			TIM_Cmd(TIM4,ENABLE); //ʹ�ܶ�ʱ��4
			for(i=0;i<2;i++)
			{
				printf("time1:%d,",RTC_time[i]);
//				printf("time2:%d,time3:%d",Plan_buffer[i*6+1],Plan_buffer[i*6+2]);
			}
			for(i=0;i<Plan_length/6;i++)
			{
				if(Plan_buffer[i*6]==0x00)
				{
					switch(Plan_buffer[i*6+3])
					{
						case 0x00://CMNS_continuous Clean
						{
							if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
								{
									TIM_Cmd(TIM3,ENABLE);   //�򿪶�ʱ��3��Duty-CYCLE��������
									Device_mode=0;
									hidden_open=0;					//�����ܹر�
									printf("CMNS_continuous Clean:%d,%d\r\n",duty_count,str_duty_value);
								}
								else
								{
									UV1=0;
									UV2=0;
									TIM_Cmd(TIM3,DISABLE);   //�رն�ʱ��3��Duty-CYCLE���ܹر�
								}
								break;
						}
						case 0x01://CMS_continuous Clean
						{
							if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
								{
									TIM_Cmd(TIM3,ENABLE);   //�򿪶�ʱ��3��Duty-CYCLE��������
									Device_mode=1;
									hidden_open=1;//�����ܿ���
									printf("CMS_continuous Clean:%d,%d\r\n",duty_count,str_duty_value);
									
								}
								else
								{
									UV1=0;
									UV2=0;
									TIM_Cmd(TIM3,DISABLE);   //�رն�ʱ��3��Duty-CYCLE���ܹر�
								}
								break;
						}
						case 0x02://UMNS_Unoccupied Mode
						{
							if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
								{
									hidden_open=0;					//�����ܹر�
									if(PIR_input==1)  //PIR���ܴ�
									{
										UV1=1;
										UV2=1;
										PIR_TIM3_flag=1;//PIR��TIM3�л���־λ����1ʧ��TIM3
										printf("Hello\r\n");
									}
									else
									{
										UV1=0;
										UV2=0;
										if(hidden_open==0)//����ģʽ��־λ�򿪻�ر�
											{
												GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
												GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
												GPIO_SetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
											}
											else
											{
												GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
												GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
												GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
											}
										printf("Nobody\r\n");
									}
									if(PIR_TIM3_flag==1)
										TIM_Cmd(TIM3,DISABLE);
									else
										TIM_Cmd(TIM3,ENABLE);
									Device_mode=2;
									printf("UMNS_Unoccupied Mode\r\n");
								}
								break;
						}
						case 0x03://UMS_Unoccupied Mode
						{
							if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
								{
									hidden_open=1;//�����ܿ���
									if(PIR_input==1)  //PIR���ܴ�
									{
										UV1=1;
										UV2=1;
										
										PIR_TIM3_flag=1;//PIR��TIM3�л���־λ����1ʧ��TIM3
										printf("Hello\r\n");
									}
									else
									{
										UV1=0;
										UV2=0;
										if(hidden_open==0)//����ģʽ��־λ�򿪻�ر�
											{
												GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
												GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
												GPIO_SetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
											}
											else
											{
												GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
												GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
												GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
											}
										printf("Nobody\r\n");
									}
									if(PIR_TIM3_flag==1)
										TIM_Cmd(TIM3,DISABLE);
									else
										TIM_Cmd(TIM3,ENABLE);
									Device_mode=2;
									printf("UMS_Unoccupied Mode\r\n");
								}
								break;
						}
						case 0x04://SMNS_Smart Mode
						{
							if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
								{
									hidden_open=0;					//�����ܹر�
									if(PIR_input==1)
									{
										UV1=0;
										UV2=0;
										printf("Hello\r\n");
									}
									else
									{
										UV1=1;
										UV2=1;
										if(hidden_open==0)//����ģʽ��־λ�򿪻�ر�
											{
												GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
												GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
												GPIO_SetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
											}
											else
											{
												GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
												GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
												GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
											}
										printf("Nobody\r\n");
									}
									Device_mode=5;
									printf("SMNS_Smart Mode\r\n");
								}
								break;
						}
						case 0x05://SMS_Smart Mode
						{
							hidden_open=1;					//�����ܿ���
							if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
								{
									if(PIR_input==1)
									{
										UV1=0;
										UV2=0;
										printf("Hello\r\n");
									}
									else
									{
										UV1=1;
										UV2=1;
										if(hidden_open==0)//����ģʽ��־λ�򿪻�ر�
											{
												GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
												GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
												GPIO_SetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
											}
											else
											{
												GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
												GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���øߣ�����G
												GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���øߣ�����B
											}
										printf("Nobody\r\n");
									}
									Device_mode=5;
									printf("SMS_Smart Mode\r\n");
								}
								break;
						}
						case 0x06://SBNS_Standby Mode
						{
							if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
								{
									UV1=0;
									UV2=0;
									hidden_open=1;					//�����ܿ���
									TIM_Cmd(TIM3,DISABLE);
									printf("SBNS_Standby Mode\r\n");
								}
								break;
						}
						case 0x07://SBS_Standby Mode
						{
							if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
								{
									UV1=0;
									UV2=0;
									hidden_open=1;					//�����ܿ���
									TIM_Cmd(TIM3,DISABLE);
									printf("SBS_Standby Mode\r\n");
								}
								break;
						}
					}
//					if(Plan_buffer[i*6+3]==0x01)
//					{
//						if((Plan_buffer[i*6+1]*60+Plan_buffer[i*6+2]<=RTC_time[0]*60+RTC_time[1])&&(Plan_buffer[i*6+4]*60+Plan_buffer[i*6+5]>=RTC_time[0]*60+RTC_time[1]))
//						{
//							printf("successful\r\n");
//						}
//					}
					
					
//					for(j=0;j<Plan_length;j++)
//					printf("%d,",Plan_buffer[j]);
//					printf("length:%d",Plan_length);
		//			Plan_buffer[i*6+1]=
		//			Plan_buffer[i*6+2]=
		//			Plan_buffer[i*6+3]=
		//			Plan_buffer[i*6+4]=
		//			Plan_buffer[i*6+5]=
				}
			}
			//Plan_happen=0;//ִ�мƻ�����ȡ����־λ
		}
}
//ָʾ�ƴ�
//�����֣�0X2F
void Identify_Device(void)
{
	u8 str_Identify_Device[8]={0x55,0xAA,0x01,0x00,0xff,0X2F,0X00,0X00}; 
  SN_count++;
  str_Identify_Device[3]=0x00;
  str_Identify_Device[4]=SN_count;
	
		Identify_Device_flag=1;
		TIM_Cmd(TIM9,ENABLE); //ʹ�ܶ�ʱ��9
	while(Identify_Device_flag==1)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
    GPIO_ResetBits(GPIOB,GPIO_Pin_14);//GPIOA5���õͣ�����G
    GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���õͣ�����B
		delay_ms(500);
		GPIO_ResetBits(GPIOB,GPIO_Pin_1);//GPIOB1���øߣ�����R
    GPIO_SetBits(GPIOB,GPIO_Pin_14);//GPIOA5���õͣ�����G
    GPIO_ResetBits(GPIOB,GPIO_Pin_15);//GPIOA5���õͣ�����B
		delay_ms(500);
	}
  str_Identify_Device[7]=Sum_Rem(str_Identify_Device,sizeof(str_Identify_Device));
  if(SendBuff[5]==0x2F)
  Usart1_Send(str_Identify_Device,sizeof(str_Identify_Device));
  Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}
//�豸��ʼ������
void Device_Init(void)
{
 u16 i=0;
	u32 time_value=0;
  u8 str_Device_Init[9]={0x55,0xAA,0x01,0x00,0xff,0X30,0X01,0X01}; 
  SN_count++;
  str_Device_Init[3]=0x00;
  str_Device_Init[4]=SN_count;
	Duty_cycle_write_buff[0]=0X03;
	Duty_cycle_write_buff[1]=0XE8;
	AT24CXX_Write(1,(u16*)Duty_cycle_write_buff,2);    //AT24LC64д������
  for(i=0;i<SendBuff[6];i++)
  {
    Device_data[i]=SendBuff[7+i];
    
  }
	tick=Device_data[14]*16777216+Device_data[15]*65536+Device_data[16]*256+Device_data[17];
			//tick=1608694096;
	tm=*localtime(&tick);
	strftime(s,sizeof(s),"%H%M%S",&tm);
//	printf("time:%d:\n",(int)tick);
//	printf("times:%s\n",s);
	time_value=atoi(s);
	RTC_Set_Time(time_value/10000+8,time_value%10000/100,56,RTC_H12_AM);	//����ʱ��
	printf("hour:%d,minte:%d\r\n",time_value/10000+8,time_value%10000/100);
  str_Device_Init[8]=Sum_Rem(str_Device_Init,sizeof(str_Device_Init));
  if(SendBuff[5]==0x30)
  Usart1_Send(str_Device_Init,sizeof(str_Device_Init));
  Stop_send_flag=1;//ֹͣ����ָ��״̬��־λ
}


