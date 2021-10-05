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


inline void io_5v_isol_sw___cntrl_vbat_s_enable(void) {
	// ParaMETEO - UC_CNTRL_VS
	GPIOB->BSRR |= GPIO_BSRR_BS8;
}
inline void io_5v_isol_sw___cntrl_vbat_s_disable(void) {
	// ParaMETEO - UC_CNTRL_VS
	GPIOB->BSRR |= GPIO_BSRR_BR8;
}

inline void io_12v_sw___cntrl_vbat_g_enable(void) {
	// ParaMETEO - UC_CNTRL_VG
	GPIOA->BSRR |= GPIO_BSRR_BS6;

}
inline void io_12v_sw___cntrl_vbat_g_disable(void) {
	// ParaMETEO - UC_CNTRL_VG
	GPIOA->BSRR |= GPIO_BSRR_BR6;

}

inline uint8_t io_get_5v_isol_sw___cntrl_vbat_s(void) {
	if ((GPIOB->ODR & (1 << 8)) != 0) {
		return 1;
	}
	else {
		return 0;
	}
}

inline uint8_t io_get_12v_sw___cntrl_vbat_g(void) {
	if ((GPIOA->ODR & (1 << 6)) != 0) {
		return 1;
	}
	else {
		return 0;
	}
}

inline void io___cntrl_vbat_r_enable(void) {
	;
}
inline void io___cntrl_vbat_r_disable(void) {
	;
}

#endif /* IO_H_ */