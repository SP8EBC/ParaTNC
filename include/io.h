/*
 * io.h
 *
 *  Created on: 11.06.2020
 *      Author: mateusz
 */

#ifndef IO_H_
#define IO_H_

/**
 * This header file (and corresponding .c file is responsible for configuring and servicing
 * various things connected to GPIO pins. Watchdog, Output collector out (for ParaTNC) and
 * switching on/off different voltages across both ParaTNC and ParaMETEO
 */

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#endif

void io_uart_init(void);

void io_oc_init(void);
void io_oc_output_low(void);
void io_oc_output_hiz(void);

void io_pwr_init(void);

void io_ext_watchdog_config(void);
void io_ext_watchdog_service(void);

void io_buttons_init(void);

#ifdef PARAMETEO
void io_vbat_meas_init(int16_t a_coeff, int16_t b_coeff);
uint16_t io_vbat_meas_get(void);
uint16_t io_vbat_meas_average(uint16_t sample);
void io_vbat_meas_disable(void);
void io_vbat_meas_enable(void);

void io_pool_vbat_r(int16_t minutes_to_wx);
#endif



/**
 * Keep this uncommented to configure ADC which monitor Vbatt to
 * continous mode, instead of single shot
 */
//#define VBAT_MEAS_CONTINOUS

inline void io_5v_isol_sw_enable(void) {
	// ParaMETEO - UC_CNTRL_VS
	GPIOB->BSRR |= GPIO_BSRR_BS8;
}
inline void io_5v_isol_sw_disable(void) {
	// ParaMETEO - UC_CNTRL_VS
	GPIOB->BSRR |= GPIO_BSRR_BR8;
}

inline void io_12v_sw_enable(void) {
	// ParaMETEO - UC_CNTRL_VG
	GPIOA->BSRR |= GPIO_BSRR_BS6;

}
inline void io_12v_sw_disable(void) {
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

#ifdef PARAMETEO
inline void io___cntrl_vbat_g_enable(void) {
	GPIOA->BSRR |= GPIO_BSRR_BS6;
}

inline void io___cntrl_vbat_g_disable(void) {
	GPIOA->BSRR |= GPIO_BSRR_BR6;

}

inline void io___cntrl_vbat_s_enable(void) {
	GPIOC->BSRR |= GPIO_BSRR_BS13;

}

inline void io___cntrl_vbat_s_disable(void) {
	GPIOC->BSRR |= GPIO_BSRR_BR13;

}

inline void io___cntrl_vbat_c_enable(void) {
	GPIOA->BSRR |= GPIO_BSRR_BS1;

}

inline void io___cntrl_vbat_c_disable(void) {
	GPIOA->BSRR |= GPIO_BSRR_BR1;

}

inline void io___cntrl_vbat_r_enable(void) {
	GPIOB->BSRR |= GPIO_BSRR_BS1;

}
inline void io___cntrl_vbat_r_disable(void) {
	GPIOB->BSRR |= GPIO_BSRR_BR1;

}

inline void io___cntrl_vbat_m_enable(void) {
	GPIOB->BSRR |= GPIO_BSRR_BS0;

}
inline void io___cntrl_vbat_m_disable(void) {
	GPIOB->BSRR |= GPIO_BSRR_BR0;

}

inline void io___cntrl_gprs_pwrkey_press() {
	GPIOA->BSRR |= GPIO_BSRR_BS7;

}
inline void io___cntrl_gprs_pwrkey_release() {
	GPIOA->BSRR |= GPIO_BSRR_BR7;

}

inline void io___cntrl_gprs_dtr_low() {
	GPIOB->BSRR |= GPIO_BSRR_BR8;
}

inline void io___cntrl_gprs_dtr_high() {
	GPIOB->BSRR |= GPIO_BSRR_BS8;
}

inline uint8_t io_get_cntrl_vbat_c(void) {
	uint8_t out = 0;

	if ((GPIOA->ODR & GPIO_ODR_ODR_1) != 0) {
		out = 1;
	}

	return out;
}

inline uint8_t io_get_cntrl_vbat_s(void) {
	uint8_t out = 0;

	if ((GPIOC->ODR & GPIO_ODR_ODR_13) != 0) {
		out = 1;
	}

	return out;
}

inline uint8_t io_get_cntrl_vbat_m(void) {
	uint8_t out = 0;

	if ((GPIOB->ODR & GPIO_BSRR_BS0) != 0) {
		out = 1;
	}

	return out;
}

#else
inline uint8_t io_get_cntrl_vbat_s(void) {
	return 0;
}

#endif

#endif /* IO_H_ */
