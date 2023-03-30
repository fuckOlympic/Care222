/**
  ******************************************************************************
  * @file    EEPROM/EEPROM_Emulation/src/eeprom.c 
  * @author  MCD Application Team
  * @brief   This file provides all the EEPROM emulation firmware functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/** @addtogroup EEPROM_Emulation
  * @{
  */ 

/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"
#include "stm32f4xx_flash.h"
#include "stdio.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Global variable used to store variable value in read sequence */
uint16_t DataVar = 0;

/* Virtual address defined by the user: 0xFFFF value is prohibited */
//extern uint16_t VirtAddVarTab[NB_OF_VAR];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static FLASH_Status EE_Format(void);
static uint16_t EE_FindValidPage(uint8_t Operation);
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_VerifyPageFullyErased(uint32_t Address);
static uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);
static uint16_t EE_Init(void);
static uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);		
/**
  * @brief  Restore the pages to a known good state in case of page's status
  *   corruption after a power loss.
  * @param  None.
  * @retval - Flash error code: on write Flash error
  *         - FLASH_COMPLETE: on success
  */
static uint16_t EE_Init(void)
{
  uint16_t PageStatus0 = 6, PageStatus1 = 6;
  uint16_t VarIdx = 0;
  uint16_t EepromStatus = 0, ReadStatus = 0;
  int16_t x = -1;
  FLASH_Status  FlashStatus;
//	uint16_t i;
//  uint32_t SectorError = 0;
//  FLASH_EraseInitTypeDef pEraseInit;

  /* Get Page0 status */
  PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);
  /* Get Page1 status */
  PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

//  pEraseInit.TypeErase = TYPEERASE_SECTORS;
//  pEraseInit.Sector = PAGE0_ID;
//  pEraseInit.NbSectors = 1;
//  pEraseInit.VoltageRange = VOLTAGE_RANGE;
//		printf("Page Status %d %d\r\n", PageStatus0,PageStatus1);
  /* Check for invalid header states and repair if necessary */
  switch (PageStatus0)
  {
    case ERASED:
//			printf("PageStatus0 == ERASED");
      if (PageStatus1 == VALID_PAGE) /* Page0 erased, Page1 valid */
      {
          /* Erase Page0 */
        if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
        {
          FlashStatus = FLASH_EraseSector(PAGE0_ID, VoltageRange_3);//HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
          /* If erase operation was failed, a Flash error code is returned */
          if (FlashStatus != FLASH_COMPLETE)
          {
//						printf("PageStatus1 == VALID_PAGE  Erased error\r\n");
            return FlashStatus;
          }
        }
      }
      else if (PageStatus1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
      {
        /* Erase Page0 */
        if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
        { 
          FlashStatus = FLASH_EraseSector(PAGE0_ID, VoltageRange_3);//HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
          /* If erase operation was failed, a Flash error code is returned */
          if (FlashStatus != FLASH_COMPLETE)
          {
//						printf("PageStatus1 == RECEIVE_DATA  Erased error\r\n");
            return FlashStatus;
          }
        }
        /* Mark Page1 as valid */
        FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, VALID_PAGE);//HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, PAGE1_BASE_ADDRESS, VALID_PAGE);
        /* If program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
//					printf("FLASH_ProgramHalfWord error\r\n");
          return FlashStatus;
        }
      }
      else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        FlashStatus = EE_Format();
        /* If erase/program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
//					printf("EE_Format error\r\n");
          return FlashStatus;
        }
      }
      break;

    case RECEIVE_DATA:
//			printf("PageStatus0 == RECEIVE_DATA");
      if (PageStatus1 == VALID_PAGE) /* Page0 receive, Page1 valid */
      {
        /* Transfer data from Page1 to Page0 */
        for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
        {
          if (( *(__IO uint16_t*)(PAGE0_BASE_ADDRESS + 6)) == VarIdx)
					//if (( *(__IO uint16_t*)(PAGE0_BASE_ADDRESS + 6)) == VirtAddVarTab[VarIdx])
          {
            x = VarIdx;
          }
          if (VarIdx != x)
          {
            /* Read the last variables' updates */
            ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
						//ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1)
            {
              /* Transfer the variable to the Page0 */
							EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
              //EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
              /* If program operation was failed, a Flash error code is returned */
              if (EepromStatus != FLASH_COMPLETE)
              {
                return EepromStatus;
              }
            }
          }
        }
        /* Mark Page0 as valid */
        FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE);//HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, PAGE0_BASE_ADDRESS, VALID_PAGE);
        /* If program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
//        pEraseInit.Sector = PAGE1_ID;
//        pEraseInit.NbSectors = 1;
//        pEraseInit.VoltageRange = VOLTAGE_RANGE;
        /* Erase Page1 */
        if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
        { 
          FlashStatus = FLASH_EraseSector(PAGE1_ID, VoltageRange_3);//HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
          /* If erase operation was failed, a Flash error code is returned */
          if (FlashStatus != FLASH_COMPLETE)
          {
            return FlashStatus;
          }
        }
      }
      else if (PageStatus1 == ERASED) /* Page0 receive, Page1 erased */
      {
//        pEraseInit.Sector = PAGE1_ID;
//        pEraseInit.NbSectors = 1;
//        pEraseInit.VoltageRange = VOLTAGE_RANGE;
        /* Erase Page1 */
        if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
        { 
          //FlashStatus = HAL_FLASHEx_Erase(PAGE1_ID, &SectorError);
					FlashStatus = FLASH_EraseSector(PAGE1_ID,VoltageRange_3);
          /* If erase operation was failed, a Flash error code is returned */
          if (FlashStatus != FLASH_COMPLETE)
          {
            return FlashStatus;
          }
        }
        /* Mark Page0 as valid */
        FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE); //HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, PAGE0_BASE_ADDRESS, VALID_PAGE);
        /* If program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      else /* Invalid state -> format eeprom */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        FlashStatus = EE_Format();
        /* If erase/program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      break;

    case VALID_PAGE:
//			printf("PageStatus0 == VALID_PAGE");
      if (PageStatus1 == VALID_PAGE) /* Invalid state -> format eeprom */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        FlashStatus = EE_Format();
        /* If erase/program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      else if (PageStatus1 == ERASED) /* Page0 valid, Page1 erased */
      {
//        pEraseInit.Sector = PAGE1_ID;
//        pEraseInit.NbSectors = 1;
//        pEraseInit.VoltageRange = VOLTAGE_RANGE;
        /* Erase Page1 */
        if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
        { 
          FlashStatus = FLASH_EraseSector(PAGE1_ID, VoltageRange_3);//HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
          /* If erase operation was failed, a Flash error code is returned */
          if (FlashStatus != FLASH_COMPLETE)
          {
            return FlashStatus;
          }
        }
      }
      else /* Page0 valid, Page1 receive */
      {
        /* Transfer data from Page0 to Page1 */
        for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
        {
          if ((*(__IO uint16_t*)(PAGE1_BASE_ADDRESS + 6)) == VarIdx)
					//if ((*(__IO uint16_t*)(PAGE1_BASE_ADDRESS + 6)) == VirtAddVarTab[VarIdx])
          {
            x = VarIdx;
          }
          if (VarIdx != x)
          {
            /* Read the last variables' updates */
            ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
						//ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1)
            {
              /* Transfer the variable to the Page1 */
              EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
							//EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
              /* If program operation was failed, a Flash error code is returned */
              if (EepromStatus != FLASH_COMPLETE)
              {
                return EepromStatus;
              }
            }
          }
        }
        /* Mark Page1 as valid */
        FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, VALID_PAGE); //HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, PAGE1_BASE_ADDRESS, VALID_PAGE);        
        /* If program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
//        pEraseInit.Sector = PAGE0_ID;
//        pEraseInit.NbSectors = 1;
//        pEraseInit.VoltageRange = VOLTAGE_RANGE;
        /* Erase Page0 */
        if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
        { 
          FlashStatus = FLASH_EraseSector(PAGE0_ID, VoltageRange_3);//HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
          /* If erase operation was failed, a Flash error code is returned */
          if (FlashStatus != FLASH_COMPLETE)
          {
            return FlashStatus;
          }
        }
      }
      break;

    default:  /* Any other state -> format eeprom */
      /* Erase both Page0 and Page1 and set Page0 as valid page */
//			printf("default\r\n");
      FlashStatus = EE_Format();
      /* If erase/program operation was failed, a Flash error code is returned */
      if (FlashStatus != FLASH_COMPLETE)
      {
        return FlashStatus;
      }
      break;
  }
//	printf("FLASH_COMPLETE");
  return FLASH_COMPLETE;
}

/**
  * @brief  Verify if specified page is fully erased.
  * @param  Address: page address
  *   This parameter can be one of the following values:
  *     @arg PAGE0_BASE_ADDRESS: Page0 base address
  *     @arg PAGE1_BASE_ADDRESS: Page1 base address
  * @retval page fully erased status:
  *           - 0: if Page not erased
  *           - 1: if Page erased
  */
static uint16_t EE_VerifyPageFullyErased(uint32_t Address)
{
  uint32_t ReadStatus = 1;
  uint16_t AddressValue = 0x5555;
    
  /* Check each active page address starting from end */
  while (Address <= PAGE0_END_ADDRESS)
  {
    /* Get the current location content to be compared with virtual address */
    AddressValue = (*(__IO uint16_t*)Address);

    /* Compare the read address with the virtual address */
    if (AddressValue != ERASED)
    {
      
      /* In case variable value is read, reset ReadStatus flag */
      ReadStatus = 0;

      break;
    }
    /* Next address location */
    Address = Address + 4;
  }
  
  /* Return ReadStatus value: (0: Page not erased, 1: Sector erased) */
  return ReadStatus;
}

/**
  * @brief  Returns the last stored variable data, if found, which correspond to
  *   the passed virtual address
  * @param  VirtAddress: Variable virtual address
  * @param  Data: Global variable contains the read variable value
  * @retval Success or error status:
  *           - 0: if variable was found
  *           - 1: if the variable was not found
  *           - NO_VALID_PAGE: if no valid page was found.
  */
static uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data)
{
  uint16_t ValidPage = PAGE0;
  uint16_t AddressValue = 0x5555, ReadStatus = 1;
  uint32_t Address = EEPROM_START_ADDRESS, PageStartAddress = EEPROM_START_ADDRESS;

  /* Get active Page for read operation */
  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  /* Check if there is no valid page */
  if (ValidPage == NO_VALID_PAGE)
  {
    return  NO_VALID_PAGE;
  }

  /* Get the valid Page start Address */
  PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

  /* Get the valid Page end Address */
  Address = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ValidPage) * PAGE_SIZE));

  /* Check each active page address starting from end */
  while (Address > (PageStartAddress + 2))
  {
    /* Get the current location content to be compared with virtual address */
    AddressValue = (*(__IO uint16_t*)Address);

    /* Compare the read address with the virtual address */
    if (AddressValue == VirtAddress)
    {
      /* Get content of Address-2 which is variable value */
      *Data = (*(__IO uint16_t*)(Address - 2));

      /* In case variable value is read, reset ReadStatus flag */
      ReadStatus = 0;

      break;
    }
    else
    {
      /* Next address location */
      Address = Address - 4;
    }
  }

  /* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
  return ReadStatus;
}

/**
  * @brief  Writes/upadtes variable data in EEPROM.
  * @param  VirtAddress: Variable virtual address
  * @param  Data: 16 bit data to be written
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  uint16_t Status = 0;

  /* Write the variable virtual address and value in the EEPROM */
  Status = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

  /* In case the EEPROM active page is full */
  if (Status == PAGE_FULL)
  {
    /* Perform Page transfer */
    Status = EE_PageTransfer(VirtAddress, Data);
  }

  /* Return last operation status */
  return Status;
}

/**
  * @brief  Erases PAGE and PAGE1 and writes VALID_PAGE header to PAGE
  * @param  None
  * @retval Status of the last operation (Flash write or erase) done during
  *         EEPROM formating
  */
static FLASH_Status EE_Format(void)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;
//  uint32_t SectorError = 0;
//  FLASH_EraseInitTypeDef pEraseInit;

//  pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;  
//  pEraseInit.Sector = PAGE0_ID;
//  pEraseInit.NbSectors = 1;
//  pEraseInit.VoltageRange = VOLTAGE_RANGE;
  /* Erase Page0 */
//	printf("Erase Page0\r\n");
  if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
  {
    FlashStatus = FLASH_EraseSector(PAGE0_ID, VoltageRange_3); //HAL_FLASHEx_Erase(&pEraseInit, &SectorError); 
    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != FLASH_COMPLETE)
    {
      return FlashStatus;
    }
  }
  /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
//	printf("Set Page0 as valid page\r\n");
  FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE);//HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, PAGE0_BASE_ADDRESS, VALID_PAGE); 
  /* If program operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }

//  pEraseInit.Sector = PAGE1_ID;
  /* Erase Page1 */
//	printf("Erase Page1\r\n");
  if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
  {  
    FlashStatus = FLASH_EraseSector(PAGE1_ID, VoltageRange_3); //HAL_FLASHEx_Erase(&pEraseInit, &SectorError); 
    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != FLASH_COMPLETE)
    {
      return FlashStatus;
    }
  }
  
  return FLASH_COMPLETE;
}

/**
  * @brief  Find valid Page for write or read operation
  * @param  Operation: operation to achieve on the valid page.
  *   This parameter can be one of the following values:
  *     @arg READ_FROM_VALID_PAGE: read operation from valid page
  *     @arg WRITE_IN_VALID_PAGE: write operation from valid page
  * @retval Valid page number (PAGE or PAGE1) or NO_VALID_PAGE in case
  *   of no valid page was found
  */
static uint16_t EE_FindValidPage(uint8_t Operation)
{
  uint16_t PageStatus0 = 6, PageStatus1 = 6;

  /* Get Page0 actual status */
  PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);

  /* Get Page1 actual status */
  PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

  /* Write or read operation */
  switch (Operation)
  {
    case WRITE_IN_VALID_PAGE:   /* ---- Write operation ---- */
      if (PageStatus1 == VALID_PAGE)
      {
        /* Page0 receiving data */
        if (PageStatus0 == RECEIVE_DATA)
        {
          return PAGE0;         /* Page0 valid */
        }
        else
        {
          return PAGE1;         /* Page1 valid */
        }
      }
      else if (PageStatus0 == VALID_PAGE)
      {
        /* Page1 receiving data */
        if (PageStatus1 == RECEIVE_DATA)
        {
          return PAGE1;         /* Page1 valid */
        }
        else
        {
          return PAGE0;         /* Page0 valid */
        }
      }
      else
      {
        return NO_VALID_PAGE;   /* No valid Page */
      }

    case READ_FROM_VALID_PAGE:  /* ---- Read operation ---- */
      if (PageStatus0 == VALID_PAGE)
      {
        return PAGE0;           /* Page0 valid */
      }
      else if (PageStatus1 == VALID_PAGE)
      {
        return PAGE1;           /* Page1 valid */
      }
      else
      {
        return NO_VALID_PAGE ;  /* No valid Page */
      }

    default:
      return PAGE0;             /* Page0 valid */
  }
}

/**
  * @brief  Verify if active page is full and Writes variable in EEPROM.
  * @param  VirtAddress: 16 bit virtual address of the variable
  * @param  Data: 16 bit data to be written as variable value
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;
  uint16_t ValidPage = PAGE0;
  uint32_t Address = EEPROM_START_ADDRESS, PageEndAddress = EEPROM_START_ADDRESS+PAGE_SIZE;

  /* Get valid Page for write operation */
  ValidPage = EE_FindValidPage(WRITE_IN_VALID_PAGE);
  
  /* Check if there is no valid page */
  if (ValidPage == NO_VALID_PAGE)
  {
    return  NO_VALID_PAGE;
  }

  /* Get the valid Page start Address */
  Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

  /* Get the valid Page end Address */
  PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 1) + (uint32_t)((ValidPage + 1) * PAGE_SIZE));

  /* Check each active page address starting from begining */
  while (Address < PageEndAddress)
  {
    /* Verify if Address and Address+2 contents are 0xFFFFFFFF */
    if ((*(__IO uint32_t*)Address) == 0xFFFFFFFF)
    {
      /* Set variable data */
      FlashStatus = FLASH_ProgramHalfWord(Address, Data); //HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, Address, Data);       
      /* If program operation was failed, a Flash error code is returned */
      if (FlashStatus != FLASH_COMPLETE)
      {
        return FlashStatus;
      }
      /* Set variable virtual address */
      FlashStatus = FLASH_ProgramHalfWord(Address + 2, VirtAddress); //HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, Address + 2, VirtAddress);       
      /* Return program operation status */
      return FlashStatus;
    }
    else
    {
      /* Next address location */
      Address = Address + 4;
    }
  }

  /* Return PAGE_FULL in case the valid page is full */
  return PAGE_FULL;
}

/**
  * @brief  Transfers last updated variables data from the full Page to
  *   an empty one.
  * @param  VirtAddress: 16 bit virtual address of the variable
  * @param  Data: 16 bit data to be written as variable value
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;
  uint32_t NewPageAddress = EEPROM_START_ADDRESS;
  uint16_t OldPageId=0;
  uint16_t ValidPage = PAGE0, VarIdx = 0;
  uint16_t EepromStatus = 0, ReadStatus = 0;
//  uint32_t SectorError = 0;
 // FLASH_EraseInitTypeDef pEraseInit;

  /* Get active Page for read operation */
  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  if (ValidPage == PAGE1)       /* Page1 valid */
  {
    /* New page address where variable will be moved to */
    NewPageAddress = PAGE0_BASE_ADDRESS;

    /* Old page ID where variable will be taken from */
    OldPageId = PAGE1_ID;
  }
  else if (ValidPage == PAGE0)  /* Page0 valid */
  {
    /* New page address  where variable will be moved to */
    NewPageAddress = PAGE1_BASE_ADDRESS;

    /* Old page ID where variable will be taken from */
    OldPageId = PAGE0_ID;
  }
  else
  {
    return NO_VALID_PAGE;       /* No valid Page */
  }

  /* Set the new Page status to RECEIVE_DATA status */
  FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, RECEIVE_DATA);//HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, NewPageAddress, RECEIVE_DATA);  
  /* If program operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }
  
  /* Write the variable passed as parameter in the new active page */
  EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data);
  /* If program operation was failed, a Flash error code is returned */
  if (EepromStatus != FLASH_COMPLETE)
  {
    return EepromStatus;
  }

  /* Transfer process: transfer variables from old to the new active page */
  for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
  {
    if (VarIdx != VirtAddress)  /* Check each variable except the one passed as parameter */
    //if (VirtAddVarTab[VarIdx] != VirtAddress)  /* Check each variable except the one passed as parameter */
    {
      /* Read the other last variable updates */
      ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
			//ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
      /* In case variable corresponding to the virtual address was found */
      if (ReadStatus != 0x1)
      {
        /* Transfer the variable to the new active page */
        EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
				//EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
        /* If program operation was failed, a Flash error code is returned */
        if (EepromStatus != FLASH_COMPLETE)
        {
          return EepromStatus;
        }
      }
    }
  }

//  pEraseInit.TypeErase = TYPEERASE_SECTORS;
//  pEraseInit.Sector = OldPageId;
//  pEraseInit.NbSectors = 1;
//  pEraseInit.VoltageRange = VOLTAGE_RANGE;
  
  /* Erase the old Page: Set old Page status to ERASED status */
  FlashStatus = FLASH_EraseSector(OldPageId, VoltageRange_3); //HAL_FLASHEx_Erase(&pEraseInit, &SectorError);  
  /* If erase operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }

  /* Set new Page status to VALID_PAGE status */
  FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, VALID_PAGE); //HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, NewPageAddress, VALID_PAGE);   
  /* If program operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }

  /* Return last operation flash status */
  return FlashStatus;
}


uint16_t EEPROM_Init()
{
	uint16_t status;
	FLASH_Unlock();
	status = EE_Init();
	FLASH_Lock();
	return status;
}

uint16_t EEPROM_WriteByte(uint16_t address, uint16_t Data)
{
	uint16_t status;
	uint16_t temp_data;
	temp_data = Data|0xff00;
	FLASH_Unlock();
	status = EE_WriteVariable(address, temp_data);
	FLASH_Lock();
//	printf("EEPROM_WriteByte: addr = %d value = %d\r\n", address, Data);
	return status;
}

uint16_t EEPROM_ReadByte(uint16_t address, uint16_t *Data)
{
	uint16_t status;
	uint16_t temp_data;
	status = EE_ReadVariable(address, &temp_data);
	if(status == 0)
	{
		*Data = temp_data&0xff;
	}
	else
		*Data = 0;
//	printf("EEPROM_ReadByte: addr = %d value = %d status = %d\r\n", address, *Data, status);
	return status;
}

uint8_t EEPROM_Check()
{
	uint16_t temp_data;
	uint16_t i;
	if(EEPROM_Init() == FLASH_COMPLETE)
	{
		if(EEPROM_ReadByte(NB_OF_VAR - 1, &temp_data) == 0)
		{
			if(temp_data == 0x55)
			{
				printf("EEPROM_Check OK\r\n");
				return 0;
			}
			else
			{
				EEPROM_WriteByte(NB_OF_VAR - 1, 0x55);
				return 0;
			}
		}
		else if(EEPROM_ReadByte(NB_OF_VAR - 1, &temp_data) == 1)
		{
			printf("First init emulation eeprom ,need write initial data\r\n");			
			for(i = 0 ; i< NB_OF_VAR -1; i++)
				EEPROM_WriteByte(i, 0xff);
			EEPROM_WriteByte(NB_OF_VAR - 1, 0x55);
			return 0;	
		}		
	}
	return 1;
}

void EEPROM_Read(uint16_t ReadAddr,uint16_t *pBuffer, uint16_t NumToRead)
{
	uint16_t i;
	for(i = 0; i< NumToRead; i++)
		EEPROM_ReadByte(ReadAddr + i , pBuffer+i);
}

void EEPROM_ReadBytes(uint16_t ReadAddr,uint8_t *pBuffer, uint16_t NumToRead)
{
	uint16_t i, temp_data;
	for(i = 0; i< NumToRead; i++)
	{
		EEPROM_ReadByte(ReadAddr + i , &temp_data);
		*(pBuffer+i) = (uint8_t)temp_data & 0xff;
	}
}


void EEPROM_Write(uint16_t WriteAddr,uint16_t *pBuffer, uint16_t NumToWrite)
{
	uint16_t i;
	for(i = 0; i< NumToWrite; i++)
		EEPROM_WriteByte(WriteAddr+i, *(pBuffer+i));
}


void EEPROM_WriteBytes(uint16_t WriteAddr,uint8_t *pBuffer, uint16_t NumToWrite)
{
	uint16_t i, temp_data;
	for(i = 0; i< NumToWrite; i++)
	{
		temp_data = *(pBuffer+i) & 0xff;
		EEPROM_WriteByte(WriteAddr+i, temp_data);
	}
}


/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
