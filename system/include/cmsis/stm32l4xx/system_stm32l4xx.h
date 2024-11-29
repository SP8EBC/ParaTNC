/*
 * system_stm32l4xx.h
 *
 *  Created on: Jul 3, 2021
 *      Author: mateusz
 */

#ifndef INCLUDE_CMSIS_STM32L4XX_SYSTEM_STM32L4XX_H_
#define INCLUDE_CMSIS_STM32L4XX_SYSTEM_STM32L4XX_H_

void system_clock_update_l4(void);		// SystemCoreClockUpdateL4
int system_clock_configure_l4(void);		// SystemClock_Config_L4
int system_clock_configure_rtc_l4(void);
void system_clock_configure_auto_wakeup_l4(uint16_t seconds);
int system_is_rtc_ok(void);

uint32_t system_get_rtc_date(void);
uint32_t system_get_rtx_time(void);

void system_set_rtc_date(uint16_t year, uint8_t month, uint8_t day_of_month);
void system_set_rtc_time(uint8_t hour, uint8_t minute, uint8_t second);


#endif /* INCLUDE_CMSIS_STM32L4XX_SYSTEM_STM32L4XX_H_ */
