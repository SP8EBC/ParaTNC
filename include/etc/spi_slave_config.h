/*
 * spi_slave_config.h
 *
 * This file consist definition and SS configuration
 * for all slaves connected to SPI bus
 *
 *  Created on: Sep 22, 2022
 *      Author: mateusz
 */

#ifndef ETC_SPI_SLAVE_CONFIG_H_
#define ETC_SPI_SLAVE_CONFIG_H_

#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>

#define PRINT_SLAVE(id, gpio, pin)	{id, (uint32_t)gpio, pin},


#define EVAL_SLAVE_ARR	\
	uint32_t spi_slaves_cfg[3][9] = {	\
		SPI_SLAVES_CONFIG(PRINT_SLAVE)	\
			};							\



#define SPI_SLAVES_CONFIG(SLAVE)	\
	SLAVE(1, GPIOA, LL_GPIO_PIN_11)	\
	SLAVE(2, GPIOA, LL_GPIO_PIN_11)	\
	SLAVE(3, GPIOB, LL_GPIO_PIN_12)	\

#endif



#endif /* ETC_SPI_SLAVE_CONFIG_H_ */
