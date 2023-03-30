#include "iwdg.h"

/******************************************************
*
*Function name : IWDG_Init
*Description 	 : 初始化独立看门狗
*Parameter		 : 
* @prer        : 预分频系数 0~7
* @rlr				 ：重装载寄存器值，低11位有效
*Return				 : NULL
*
******************************************************/

void IWDG_Init(u8 prer,u16 rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); /* ??????IWDG_PR?IWDG_RLR????*/
    IWDG_SetPrescaler(prer);    /*??IWDG????:??IWDG????*/
    IWDG_SetReload(rlr);     /*??IWDG????*/
    IWDG_ReloadCounter();    /*??IWDG???????????IWDG???*/
    IWDG_Enable();        /*??IWDG*/
}

/******************************************************
*
*Function name : IWDG_Feed
*Description 	 : 喂狗函数
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();    /*reload*/
}
