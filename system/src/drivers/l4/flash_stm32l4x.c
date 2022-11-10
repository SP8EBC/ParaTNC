/*
 * flash_stm32l4x.c
 *
 *  Created on: Jun 6, 2021
 *      Author: mateusz
 */

#include "./drivers/l4/flash_stm32l4x.h"
#include "stm32l4xx.h"

#define KB *1024


#define FLASH_KEY1                0x45670123U                          /*!< Flash key1 */
#define FLASH_KEY2                0xCDEF89ABU                          /*!< Flash key2: used with FLASH_KEY1
                                                                            to unlock the FLASH registers access */

/**
  * @brief  Returns the FLASH Bank1 Status.
  * @note   This function can be used for all STM32F10x devices, it is equivalent
  *         to FLASH_GetStatus function.
  * @param  None
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP or FLASH_COMPLETE
  */
FLASH_Status FLASH_GetBank1Status(void)
{
  FLASH_Status flashstatus = FLASH_COMPLETE;

  if((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY)
  {
    flashstatus = FLASH_BUSY;
  }
  else
  {
    if((FLASH->SR & FLASH_SR_FASTERR) != 0)
    {
      flashstatus = FLASH_ERROR_PG;
    }
    else if ((FLASH->SR & FLASH_SR_FASTERR) != 0)
    {
        flashstatus = FLASH_ERROR_PG;
    }
    else if ((FLASH->SR & FLASH_SR_PROGERR) != 0)
    {
        flashstatus = FLASH_ERROR_PG;
    }
    else if ((FLASH->SR & FLASH_SR_SIZERR) != 0)
    {
        flashstatus = FLASH_ERROR_PG;
    }
    else if ((FLASH->SR & FLASH_SR_PGSERR) != 0)
    {
        flashstatus = FLASH_ERROR_PG;
    }
    else
    {
      if((FLASH->SR & FLASH_SR_WRPERR) != 0 )
      {
        flashstatus = FLASH_ERROR_WRP;
      }
      else
      {
        flashstatus = FLASH_COMPLETE;
      }
    }
  }
  /* Return the Flash Status */
  return flashstatus;
}

FLASH_Status FLASH_ErasePage(uint32_t Page_Address) {

	FLASH_Status out;

	uint32_t Page = 0;

	uint32_t Banks = 0;

	if (FLASH_SIZE == 1024 KB) {
		// calculate the bank number
		if (Page_Address < 0x08080000) {
			Banks = FLASH_BANK_1;
		}
		else {
			Banks = FLASH_BANK_2;
		}
	}
	else if (FLASH_SIZE == 512 KB) {
		// calculate the bank number
		if (Page_Address < 0x08040000) {
			Banks = FLASH_BANK_1;
		}
		else {
			Banks = FLASH_BANK_2;
		}
	}



	FLASH_Unlock();

	Page = Page_Address - 0x08000000 - (0x80000 * (Banks - 1));

	Page = Page / 2048;

	#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
	    defined (STM32L496xx) || defined (STM32L4A6xx) || \
	    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
	    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
	#if defined (STM32L4P5xx) || defined (STM32L4Q5xx) || defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
	  if(READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK) == 0U)
	  {
	    CLEAR_BIT(FLASH->CR, FLASH_CR_BKER);
	  }
	  else
	#endif
	  {

	    if((Banks & FLASH_BANK_1) != 0U)
	    {
	      CLEAR_BIT(FLASH->CR, FLASH_CR_BKER);
	    }
	    else
	    {
	      SET_BIT(FLASH->CR, FLASH_CR_BKER);
	    }
	  }
	#else
	  /* Prevent unused argument(s) compilation warning */
	  UNUSED(Banks);
	#endif

	  SET_BIT(FLASH->CR, FLASH_CR_EOPIE);

	  /* Proceed to erase the page */
	  MODIFY_REG(FLASH->CR, FLASH_CR_PNB, ((Page & 0xFFU) << FLASH_CR_PNB_Pos));
	  SET_BIT(FLASH->CR, FLASH_CR_PER);
	  SET_BIT(FLASH->CR, FLASH_CR_STRT);


	  // wait for flash operation to finish
	  while((FLASH->SR & FLASH_SR_BSY) != 0);

	  if ((FLASH->SR & FLASH_SR_EOP) == FLASH_SR_EOP) {
		  out = FLASH_COMPLETE;
	  }
	  else {
		  out = FLASH_ERROR_WRP;
	  }

	  CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
	  CLEAR_BIT(FLASH->CR, FLASH_CR_EOPIE);

	  FLASH_Lock();

	  return out;
}

void FLASH_Unlock(void) {
	  //HAL_StatusTypeDef status = HAL_OK;

	  if(READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
	  {
	    /* Authorize the FLASH Registers access */
	    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
	    WRITE_REG(FLASH->KEYR, FLASH_KEY2);

	    /* Verify Flash is unlocked */
	    if(READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
	    {
	      //status = HAL_ERROR;
	    }
	  }

	  //return status;
}


void FLASH_Lock(void) {
	  SET_BIT(FLASH->CR, FLASH_CR_LOCK);

}

///**
//  * @brief  Erase the specified FLASH memory page.
//  * @param  Page FLASH page to erase
//  *         This parameter must be a value between 0 and (max number of pages in the bank - 1)
//  * @param  Banks Bank(s) where the page will be erased
//  *          This parameter can be one of the following values:
//  *            @arg FLASH_BANK_1: Page in bank 1 to be erased
//  *            @arg FLASH_BANK_2: Page in bank 2 to be erased
//  * @retval None
//  */
//void FLASH_PageErase(uint32_t Page, uint32_t Banks)
//{
//  /* Check the parameters */
//  assert_param(IS_FLASH_PAGE(Page));
//
//#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
//    defined (STM32L496xx) || defined (STM32L4A6xx) || \
//    defined (STM32L4P5xx) || defined (STM32L4Q5xx) || \
//    defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
//#if defined (STM32L4P5xx) || defined (STM32L4Q5xx) || defined (STM32L4R5xx) || defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
//  if(READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK) == 0U)
//  {
//    CLEAR_BIT(FLASH->CR, FLASH_CR_BKER);
//  }
//  else
//#endif
//  {
//
//    if((Banks & FLASH_BANK_1) != 0U)
//    {
//      CLEAR_BIT(FLASH->CR, FLASH_CR_BKER);
//    }
//    else
//    {
//      SET_BIT(FLASH->CR, FLASH_CR_BKER);
//    }
//  }
//#else
//  /* Prevent unused argument(s) compilation warning */
//  UNUSED(Banks);
//#endif
//
//  /* Proceed to erase the page */
//  MODIFY_REG(FLASH->CR, FLASH_CR_PNB, ((Page & 0xFFU) << FLASH_CR_PNB_Pos));
//  SET_BIT(FLASH->CR, FLASH_CR_PER);
//  SET_BIT(FLASH->CR, FLASH_CR_STRT);
//}
//
//
//FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout)
//{
//  FLASH_Status status = FLASH_COMPLETE;
//
//  /* Check for the Flash Status */
//  status = FLASH_GetBank1Status();
//  /* Wait for a Flash operation to complete or a TIMEOUT to occur */
//  while((status == FLASH_BUSY) && (Timeout != 0x00))
//  {
//    status = FLASH_GetBank1Status();
//    Timeout--;
//  }
//  if(Timeout == 0x00 )
//  {
//    status = FLASH_TIMEOUT;
//  }
//  /* Return the operation status */
//  return status;
//}




