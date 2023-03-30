#include "myiic.h"
#include "tca9555.h"

uint16_t I2C_TCA9555read(void)
{
	u16 readDATA=0x0000;
	u8 gpio_p0=0x00;
	u8 gpio_p1=0x00;
	
	IIC_Start();
	IIC_Send_Byte(TCA9555_ADDR_W);
	IIC_Wait_Ack();
	IIC_Send_Byte(INPUT_PORT0);
	IIC_Wait_Ack();
	
	IIC_Start();
	IIC_Send_Byte(TCA9555_ADDR_R);
	IIC_Wait_Ack();
	gpio_p0=IIC_Read_Byte(1);
	gpio_p1=IIC_Read_Byte(0);
	IIC_Stop();
	readDATA=((u16)gpio_p1<<8 )| gpio_p0;
	return readDATA;
}
