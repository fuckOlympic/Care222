#ifndef __UV_LAMP_H
#define __UV_LAMP_H
#include "sys.h"

//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//LED¶Ë¿Ú¶¨Òå
#define UV_PWR_EN PBout(3)
#define LAMP_A_ENABLE PCout(0)
#define INVERTER_DRV_A_CMD PCout(2)
#define LAMP_B_ENABLE PCout(4)
#define INVERTER_DRV_B_CMD PCout(6)
#define LAMP_ON_A PBin(0)
#define LAMP_ON_B PCin(3)
#define SW6 PBin(10)
#define SW5 PBin(12)
#define UV1 PCout(4)	// UV1
#define UV2 PCout(0)	// UV2	 
//#define LAMP_ON_A PCin(3)  //LAMP_ON_A
//#define LAMP_ON_B PBin(0)  //LAMP_ON_B

void UV_Init(void);		 				    
#endif
