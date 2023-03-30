#include "iwdg.h"

/******************************************************
*
*Function name : IWDG_Init
*Description 	 : ��ʼ���������Ź�
*Parameter		 : 
* @prer        : Ԥ��Ƶϵ�� 0~7
* @rlr				 ����װ�ؼĴ���ֵ����11λ��Ч
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
*Description 	 : ι������
*Parameter		 : NULL
*Return				 : NULL
*
******************************************************/
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();    /*reload*/
}
