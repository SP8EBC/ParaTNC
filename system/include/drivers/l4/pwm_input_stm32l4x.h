/*
 * pwm_input_stm32l4x.h
 *
 *  Created on: Apr 17, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_L4_PWM_INPUT_STM32L4X_H_
#define INCLUDE_DRIVERS_L4_PWM_INPUT_STM32L4X_H_

#include <stdint.h>

void pwm_input_io_init(void);
void pwm_input_init(uint8_t channel);
void pwm_input_pool(void);

#endif /* INCLUDE_DRIVERS_L4_PWM_INPUT_STM32L4X_H_ */
