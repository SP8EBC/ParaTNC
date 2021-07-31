/*
 * io.h
 *
 *  Created on: 11.06.2020
 *      Author: mateusz
 */

#ifndef IO_H_
#define IO_H_

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#endif

void io_oc_init(void);
void io_oc_output_low(void);
void io_oc_output_hiz(void);

void io_ext_watchdog_config(void);
void io_ext_watchdog_service(void);

//void io_5v_isol_sw_cntrl_vbat_s_enable(void);
//void io_5v_isol_sw_cntrl_vbat_s_disable(void);
//
//void io_12v_sw_cntrl_vbat_g_enable(void);
//void io_12v_sw_cntrl_vbat_g_disable(void);

inline void io_5v_isol_sw_cntrl_vbat_s_enable(void) {
	GPIOB->BSRR |= GPIO_BSRR_BS8;
}
inline void io_5v_isol_sw_cntrl_vbat_s_disable(void) {
	GPIOB->BSRR |= GPIO_BSRR_BR8;
}

inline void io_12v_sw_cntrl_vbat_g_enable(void) {
	GPIOA->BSRR |= GPIO_BSRR_BS6;

}
inline void io_12v_sw_cntrl_vbat_g_disable(void) {
	GPIOA->BSRR |= GPIO_BSRR_BR6;

}

#endif /* IO_H_ */
