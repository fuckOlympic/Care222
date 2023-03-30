#ifndef __TCA9555_H
#define __TCA9555_H

#define TCA9555_ADDR_W  0x48
#define TCA9555_ADDR_R  0x49
#define INPUT_PORT0 0x0
#define INPUT_PORT1 0x1
#define OUTPUT_PORT0 0x2
#define OUTPUT_PORT1 0x3
#define POLARITY_INVERSION_PORT0 0x4
#define POLARITY_INVERSION_PORT1 0x5
#define CONFIG_PORT0 0x6
#define CONFIG_PORT1 0x7

uint16_t I2C_TCA9555read(void);

#endif
