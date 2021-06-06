/*
 * flash_stm32l4x.h
 *
 *  Created on: Jun 6, 2021
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_L4_FLASH_STM32L4X_H_
#define INCLUDE_DRIVERS_L4_FLASH_STM32L4X_H_

#include "station_config_target_hw.h"

typedef enum
{
  FLASH_BUSY = 1,
  FLASH_ERROR_PG,
  FLASH_ERROR_WRP,
  FLASH_COMPLETE,
  FLASH_TIMEOUT
}FLASH_Status;


#define FLASH_BANK_1              ((uint32_t)0x01)                          /*!< Bank 1   */
#if defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) || defined (STM32L486xx) || \
    defined (STM32L496xx) || defined (STM32L4A6xx) || defined (STM32L4P5xx) || defined (STM32L4Q5xx) || defined (STM32L4R5xx) || \
    defined (STM32L4R7xx) || defined (STM32L4R9xx) || defined (STM32L4S5xx) || defined (STM32L4S7xx) || defined (STM32L4S9xx)
#define FLASH_BANK_2              ((uint32_t)0x02)                          /*!< Bank 2   */
#define FLASH_BANK_BOTH           ((uint32_t)(FLASH_BANK_1 | FLASH_BANK_2)) /*!< Bank1 and Bank2  */
#else
#define FLASH_BANK_BOTH           ((uint32_t)(FLASH_BANK_1))                /*!< Bank 1   */
#endif

FLASH_Status FLASH_GetBank1Status(void);


#endif /* INCLUDE_DRIVERS_L4_FLASH_STM32L4X_H_ */
