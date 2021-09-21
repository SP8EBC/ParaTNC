/*
 * LedConfig.h
 *
 *  Created on: 05.07.2017
 *      Author: mateusz
 */

#ifndef LEDCONFIG_H_
#define LEDCONFIG_H_

// PC8 - LED1 - upper
// PC9 - LED2 - lower

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>
#endif

#include <stdint.h>
#include <stdbool.h>

#define BLINK_DURATION_MSEC 		20 * BLINK_MSEC_PER_SVC_CALL
#define BLINK_MSEC_PER_SVC_CALL 	10

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t led_blinking_led2;
extern uint8_t led_blinking_led1;

void led_config(void);
void led_init(void);
void led_service_blink(void);

#ifdef STM32F10X_MD_VL
inline void led_control_led1_upper(bool _in) {
	if (_in == true) {
		GPIOC->BSRR |= GPIO_BSRR_BS8;
	}
	else {
		GPIOC->BSRR |= GPIO_BSRR_BR8;
	}
}

inline void led_control_led2_bottom(bool _in) {
	if (_in == true) {
		GPIOC->BSRR |= GPIO_BSRR_BS9;
	}
	else {
		GPIOC->BSRR |= GPIO_BSRR_BR9;
	}
}

inline void led_flip_led1_upper(void) {
	if ((GPIOC->ODR & GPIO_ODR_ODR8)  == GPIO_ODR_ODR8) {
		GPIOC->BSRR |= GPIO_BSRR_BR8;
	}
	else {
		GPIOC->BSRR |= GPIO_BSRR_BS8;
	}
}

inline void led_flip_led2_bottom(void) {
	if ((GPIOC->ODR & GPIO_ODR_ODR9)  == GPIO_ODR_ODR9) {
		GPIOC->BSRR |= GPIO_BSRR_BR9;
	}
	else {
		GPIOC->BSRR |= GPIO_BSRR_BS9;
	}
}

inline void led_blink_led1_upper(void) {
	led_blinking_led1 = BLINK_DURATION_MSEC;

	led_flip_led1_upper();
}

inline void led_blink_led2_botoom(void) {
	led_blinking_led2 = BLINK_DURATION_MSEC;

	led_flip_led2_bottom();

}
#endif

#ifdef STM32L471xx
inline void led_control_led1_upper(bool _in) {
	if (_in == true) {
		GPIOC->BSRR |= GPIO_BSRR_BS8;
	}
	else {
		GPIOC->BSRR |= GPIO_BSRR_BR8;
	}
}

inline void led_control_led2_bottom(bool _in) {
	if (_in == true) {
		GPIOC->BSRR |= GPIO_BSRR_BS9;
	}
	else {
		GPIOC->BSRR |= GPIO_BSRR_BR9;
	}
}

inline void led_flip_led1_upper(void) {
	if ((GPIOC->ODR & GPIO_ODR_ODR_8)  == GPIO_ODR_ODR_8) {
		GPIOC->BSRR |= GPIO_BSRR_BR8;
	}
	else {
		GPIOC->BSRR |= GPIO_BSRR_BS8;
	}
}

inline void led_flip_led2_bottom(void) {
	if ((GPIOA->ODR & GPIO_ODR_ODR_9)  == GPIO_ODR_ODR_9) {
		GPIOC->BSRR |= GPIO_BSRR_BR9;
	}
	else {
		GPIOC->BSRR |= GPIO_BSRR_BS9;
	}
}

inline void led_blink_led1_upper(void) {
	led_blinking_led1 = BLINK_DURATION_MSEC;

	led_flip_led1_upper();
}

inline void led_blink_led2_botoom(void) {
	led_blinking_led2 = BLINK_DURATION_MSEC;

	led_flip_led2_bottom();

}

#endif

#ifdef __cplusplus
}
#endif


#endif /* LEDCONFIG_H_ */
