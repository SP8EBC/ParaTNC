/*
 * io.c
 *
 *  Created on: 11.06.2020
 *      Author: mateusz
 */


#include <wx_pwr_switch.h>
#include "station_config_target_hw.h"
#include "variant.h"

#include "io.h"
#include "backup_registers.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#include <drivers/f1/gpio_conf_stm32f1x.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>

/**
 * Internal state of VBAT R pooler
 */
typedef enum io_pool_vbat_r_state_t {
	POOL_VBAT_R_TURNED_ON,		//!< VBAT_R is turned on but it is still 2 minutes remaining
	POOL_VBAT_R_TURNED_OFF,		//!< 2 minutes remaining and VBAT_R was turned off
	POOL_VBAT_R_WAIT,			//!< VBAT_R was turned off but wait
	POOL_VBAT_R_DONT_SWITCH		//!< more or less than 2 minutes to wx packet or VBAT_R was restarted already
}io_pool_vbat_r_state_t;
#endif

#include "station_config.h"

#if defined(PARAMETEO)
//!< Used across this file to configure I/O pins
LL_GPIO_InitTypeDef GPIO_InitTypeDef;

//!< coefficient used to convert value read by ADC into battery volage
int16_t io_vbat_a_coeff = 0, io_vbat_b_coeff = 0;

//!< lenght of an average buffer
#define VBATT_HISTORY_LN	16

//!< circular buffer used to calculate average battery voltage
static uint16_t io_vbatt_history[VBATT_HISTORY_LN];

//!< iterator across a buffer with battery measurements
static uint8_t io_vbatt_history_it = 0;

//!< State machine of battery voltage measurement
static io_vbat_state_t io_vbatt_state = IO_VBAT_UNINITIALIZED;

//!< maximum number of state machine iterations which will not trigger an assert
#define MAXIMUM_VBAT_GET_ASYNC_ITERATIONS	32

//!< minimum voltage as an increment of 10mV, which will me treated as senseful
#define MINIMUM_SENSEFUL_VBATT_VOLTAGE	512u

//! An instance of VBAT_R pooler internal state
io_pool_vbat_r_state_t io_vbat_r_state = POOL_VBAT_R_DONT_SWITCH;

#endif

/**
 * Initializes I/O pins used by all UARTs
 */
void io_uart_init(void) {

#if defined(STM32F10X_MD_VL)

  // Configure I/O pins for USART1 (Kiss modem)
  Configure_GPIO(GPIOA,10,PUD_INPUT);		// RX
  Configure_GPIO(GPIOA,9,AFPP_OUTPUT_2MHZ);	// TX

  // Configure I/O pins for USART2 (wx meteo comm)
  Configure_GPIO(GPIOA,3,PUD_INPUT);		// RX
  Configure_GPIO(GPIOA,2,AFPP_OUTPUT_2MHZ);	// TX

#endif

#if defined(STM32F10X_MD_VL)
  Configure_GPIO(GPIOA,8,GPPP_OUTPUT_2MHZ);	// re/te
  GPIO_ResetBits(GPIOA, GPIO_Pin_8);
#endif

#if defined(STM32L471xx)
  	// USART1 - KISS
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_10;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// RX

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_9;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// TX

	// USART2 - METEO
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_3;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// RX

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_2;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// TX

	// USART3 - GSM
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;	// LL_GPIO_MODE_ALTERNATE
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_10;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// TX

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;  // LL_GPIO_MODE_ALTERNATE
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_11;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// RX

#endif

#if defined(STM32L471xx)
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_8;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// RE-TE

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_8;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// DTR = Disconnect

	io___cntrl_gprs_dtr_high();
#endif
}

/**
 * Initializes open collector output
 */
void io_oc_init(void) {
#ifdef STM32F10X_MD_VL
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

#if defined(STM32L471xx)
	// PA7 - GPRS power key
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_7;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);

#endif
}

/**
 * SEts open collector output to active (low) states
 */
void io_oc_output_low(void) {
#ifdef STM32F10X_MD_VL

	GPIO_SetBits(GPIOA, GPIO_Pin_11);
#endif
}

/**
 * Sets open collector output to inactive (high-impedance) state
 */
void io_oc_output_hiz(void) {
#ifdef STM32F10X_MD_VL

	GPIO_ResetBits(GPIOA, GPIO_Pin_11);
#endif
}


void io_pwr_init(void) {
#if defined(PARATNC)

			// RELAY_CNTRL
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init(GPIOB, &GPIO_InitStructure);

			// +12V PWR_CNTRL
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
			GPIO_Init(GPIOA, &GPIO_InitStructure);

			wx_pwr_state = WX_PWR_OFF;

			GPIO_ResetBits(GPIOB, GPIO_Pin_8);

			// +12V_SW PWR_CNTRL
			GPIO_ResetBits(GPIOA, GPIO_Pin_6);

#endif

#if defined(PARAMETEO)
			// PC13 - UC_CNTRL_VS
			GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
			GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitTypeDef.Pin = LL_GPIO_PIN_13;
			GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
			GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
			GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
			LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

			// PA6 - UC_CNTRL_VG
			GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
			GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitTypeDef.Pin = LL_GPIO_PIN_6;
			GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
			GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
			GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
			LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);

			// PA1 - UC_CNTRL_VC
			GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
			GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitTypeDef.Pin = LL_GPIO_PIN_1;
			GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
			GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
			GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
			LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);

			// PB1 - UC_CNTRL_VC
			GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
			GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitTypeDef.Pin = LL_GPIO_PIN_1;
			GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
			GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
			GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
			LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);

			// PB0 - UC_CNTRL_VM
			GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
			GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
			GPIO_InitTypeDef.Pin = LL_GPIO_PIN_0;
			GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
			GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
			GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
			LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);

			io___cntrl_vbat_r_disable();
			io___cntrl_vbat_g_disable();
			io___cntrl_vbat_c_disable();
			io___cntrl_vbat_s_disable();
			io___cntrl_vbat_m_disable();
#endif
}


void io_ext_watchdog_config(void) {
#ifdef PARATNC
	  // initialize Watchdog output
	  Configure_GPIO(GPIOA,12,GPPP_OUTPUT_50MHZ);
#endif

}

void io_ext_watchdog_service(void) {
#ifdef PARATNC
	if ((GPIOA->ODR & GPIO_ODR_ODR12) == 0) {
		// set high
		GPIOA->BSRR |= GPIO_BSRR_BS12;
	}
	else {
		// set low
		GPIOA->BSRR |= GPIO_BSRR_BR12;
	}
#endif
}

void io_buttons_init(void){
#ifdef PARAMETEO

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_3;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_UP;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_0;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_UP;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);
#endif
}

/**
 * Initializes battery voltage measurement and powers up the ADC
 * @param a_coeff
 * @param b_coeff
 */
void io_vbat_meas_init(int16_t a_coeff, int16_t b_coeff) {

#ifdef PARAMETEO
	if (a_coeff != io_vbat_a_coeff || b_coeff != io_vbat_b_coeff) {
		io_vbat_a_coeff = a_coeff;
		io_vbat_b_coeff = b_coeff;

		memset(io_vbatt_history, 0x00, VBATT_HISTORY_LN);
	}

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_5;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	LL_GPIO_EnablePinAnalogControl(GPIOC, LL_GPIO_PIN_5);

	io_vbat_meas_enable();

#endif
}

/**
 * Gets VBATT_HISTORY_LN battery voltage measurements and put them into an average
 * FIFO.
 * @param last
 * @param average
 */
void io_vbat_meas_fill(uint16_t * last, uint16_t * average) {

	uint16_t tempo_result = 0;

	uint16_t average_resut = 0;

	for (int i = 0; i < VBATT_HISTORY_LN; i++) {
		tempo_result = io_vbat_meas_get_synchro_old();

		average_resut = io_vbat_meas_average(tempo_result);
	}

	if (variant_validate_is_within_ram(last)) {
		*last = tempo_result;
	}

	if (variant_validate_is_within_ram(average)) {
		*average = average_resut;
	}
}

/**
 * Battery voltage measurement state machine pooler
 * @param result voltage measurements result
 * @return state after executing state machine
 */
io_vbat_state_t io_vbat_meas_get(uint16_t * result) {

	io_vbat_state_t out = io_vbatt_state;

#ifdef PARAMETEO

	float temp = 0.0f, temp2 = 0.0f;

	if (out == IO_VBAT_UNINITIALIZED) {
		return out;
	}

#ifdef VBAT_MEAS_CONTINOUS

    // get conversion result
	out = ADC2->DR;

	io_vbatt_state = IO_VBAT_RESULT_AVAILABLE;

#else

	if (io_vbatt_state == IO_VBAT_ADC_DISABLE) {

		// if ADC is not enabled
		if ((ADC2->CR & ADC_CR_ADEN) == 0) {
			// start ADC
			ADC2->CR |= ADC_CR_ADEN;

		}

		io_vbatt_state = IO_VBAT_ADC_STARTING;

	}
	else if (io_vbatt_state == IO_VBAT_ADC_STARTING) {

		if ((ADC2->ISR & ADC_ISR_ADRDY) == ADC_ISR_ADRDY) {
			// start conversion
			ADC2->CR |= ADC_CR_ADSTART;

			io_vbatt_state = IO_VBAT_ADC_MEASURING;
		}
	}
	else if (io_vbatt_state == IO_VBAT_ADC_MEASURING) {

		if ((ADC2->ISR & ADC_ISR_EOC) == ADC_ISR_EOC) {
			if ((ADC2->ISR & ADC_ISR_EOS) == ADC_ISR_EOS) {

				ADC2->ISR |= ADC_ISR_EOS;
				ADC2->ISR |= ADC_ISR_EOC;

				temp = ADC2->DR;

				// adc has a resulution of 12bit, so with VDDA of 3.3V it gives about .00081V per division
				temp2 = (float)temp * 0.00081f;		// real voltage on ADC input

				*result = (uint16_t) (temp2 * (float)io_vbat_a_coeff + (float)io_vbat_b_coeff);

				io_vbatt_state = IO_VBAT_RESULT_AVAILABLE;
			}
		}
	}
	else if (io_vbatt_state == IO_VBAT_RESULT_AVAILABLE) {
		// disable the ADC
		ADC2->CR |= ADC_CR_ADDIS;

		io_vbatt_state = IO_VBAT_ADC_DISABLE;
	}
#endif



#endif
	out = io_vbatt_state;

	return out;
}

/**
 * Calls state machine pooler until a measurement is obtained from ADC
 * @return
 */
uint16_t io_vbat_meas_get_synchro(void) {

	uint16_t out = 0;

	uint8_t iterations = 0;

	if (io_vbatt_state != IO_VBAT_UNINITIALIZED) {


		if (io_vbatt_state != IO_VBAT_ADC_DISABLE) {
			io_vbat_meas_disable();
			io_vbat_meas_enable();
		}

		for (; iterations < MAXIMUM_VBAT_GET_ASYNC_ITERATIONS; iterations++) {
			const io_vbat_state_t res = io_vbat_meas_get(&out);

			if (res == IO_VBAT_ADC_DISABLE) {
				break;
			}
		}

		if (iterations >= MAXIMUM_VBAT_GET_ASYNC_ITERATIONS) {
			backup_assert(BACKUP_REG_ASSERT_GET_VBAT_SYNCHRONOUS_TOO_LONG);
		}
	}

	return out;
}

/**
 * Gets battery voltage synchronously in old way
 * @return
 */
uint16_t io_vbat_meas_get_synchro_old() {

	uint16_t out = 0;

#ifdef PARAMETEO

	float temp = 0.0f;

#ifdef VBAT_MEAS_CONTINOUS

    // get conversion result
	out = ADC2->DR;

#else

	// if ADC is not enabled
	if ((ADC2->CR & ADC_CR_ADEN) == 0) {
		// start ADC
		ADC2->CR |= ADC_CR_ADEN;

		// wait for startup
	    while((ADC2->ISR & ADC_ISR_ADRDY) == 0);
	}

	// start conversion
	ADC2->CR |= ADC_CR_ADSTART;

	// wait for conversion to finish
    while((ADC2->ISR & ADC_ISR_EOC) == 0) {
    	if ((ADC2->ISR & ADC_ISR_EOS) != 0) {
    		break;
    	}
    }

    // get conversion result
	out = ADC2->DR;

    // if end of sequence flag is not cleared
	if ((ADC2->ISR & ADC_ISR_EOS) != 0) {
		ADC2->ISR |= ADC_ISR_EOS;
	}

	// disable the ADC
	ADC2->CR |= ADC_CR_ADDIS;

	// wait for disable to complete
	while((ADC2->CR & ADC_CR_ADDIS) != 0);

#endif

	// adc has a resulution of 12bit, so with VDDA of 3.3V it gives about .00081V per division
	temp = (float)out * 0.00081f;		// real voltage on ADC input

	out = (uint16_t) (temp * (float)io_vbat_a_coeff + (float)io_vbat_b_coeff);

#endif
	return out;
}

/**
 * Puts new sample into average FIFO and return averaged result if FIFO
 * is fully populated
 * @param sample
 * @return zero if FIFO id not fully populated, or average value
 */
uint16_t io_vbat_meas_average(uint16_t sample) {

	uint16_t out = 0;
#ifdef PARAMETEO

	int i = 0;

	uint32_t average_acc = 0;

	// if the iterator reached the end of buffer
	if (io_vbatt_history_it >= VBATT_HISTORY_LN) {
		// reset it to the begining
		io_vbatt_history_it = 0;
	}

	// but new sample in the buffer
	io_vbatt_history[io_vbatt_history_it++] = sample;


	for (i = 0; i < VBATT_HISTORY_LN; i++) {

		// break from the loop if buffer isn't fully filled with data
		if (io_vbatt_history[i] < MINIMUM_SENSEFUL_VBATT_VOLTAGE) {
			break;
		}

		// sum sample
		average_acc += io_vbatt_history[i];
	}

	// if whole buffer has been used for average calculation
	if (i >= VBATT_HISTORY_LN) {
		// replace output
		out = (uint16_t)(average_acc / VBATT_HISTORY_LN);
	}

#endif
	return out;
}

#ifdef PARAMETEO
/**
 * This function has to be called before switching to IDLE state to turn off the ADC
 */
void io_vbat_meas_disable(void) {
	if ((ADC2->CR & ADC_CR_ADEN) != 0) {
		// disable conversion
		ADC2->CR |= ADC_CR_ADSTP;

		while((ADC2->CR & ADC_CR_ADSTP) == ADC_CR_ADSTP);
	}

	// disable the ADC
	ADC2->CR |= ADC_CR_ADDIS;

	// wait for disable to complete
	while((ADC2->CR & ADC_CR_ADDIS) != 0);

	ADC2->CR |= ADC_CR_DEEPPWD;

	io_vbatt_state = IO_VBAT_UNINITIALIZED;
}

void io_vbat_meas_enable(void) {

	volatile int stupid_delay = 0;

	// reset the clock for ADC
//	RCC->AHB2ENR &= (0xFFFFFFFF ^ RCC_AHB2ENR_ADCEN);
//	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;

	// check if ADC is enabled
	if ((ADC2->CR & ADC_CR_ADEN) != 0) {
		// disable it
		ADC2->CR |= ADC_CR_ADDIS;

		// and wait for disabling to complete
	    while((ADC2->CR & ADC_CR_ADDIS) == ADC_CR_ADDIS);
	}

	// exit from deep-power-down mode
	ADC2->CR &= (0xFFFFFFFF ^ ADC_CR_DEEPPWD);

	// start ADC voltage regulator
	ADC2->CR |= ADC_CR_ADVREGEN;

	// wait for voltage regulator to start
	for (; stupid_delay < 0x3FFFF; stupid_delay++);

	// start the calibration
	ADC2->CR |= ADC_CR_ADCAL;

	// wait for calibration to finish
    while((ADC2->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL);

    // set the first (and only channel in a conversion sequence) channel 14
    ADC2->SQR1 |= (14 << 6);

    // set the sampling rate to 24.5 ADC clock cycles
    ADC2->SMPR1 |= 0x3;

#ifdef VBAT_MEAS_CONTINOUS

    // disable discontinous mode
	ADC2->CFGR &= (0xFFFFFFFF ^ ADC_CFGR_DISCEN);

    // set continous conversion
	ADC2->CFGR |= ADC_CFGR_CONT;

	// ignore overrun and overwrite data register content with new conversion result
	ADC2->CFGR |= ADC_CFGR_OVRMOD;

	// start ADC
	ADC2->CR |= ADC_CR_ADEN;

	// start conversion
	ADC2->CR |= ADC_CR_ADSTART;

#else
    // disable continous mode
	ADC2->CFGR &= (0xFFFFFFFF ^ ADC_CFGR_CONT);

    // set discontinous conversion
	ADC2->CFGR |= ADC_CFGR_DISCEN;

	// ignore overrun and overwrite data register content with new conversion result
	ADC2->CFGR |= ADC_CFGR_OVRMOD;
#endif

	io_vbatt_state = IO_VBAT_ADC_DISABLE;
}

void io_pool_vbat_r(int16_t minutes_to_wx) {

	// check how many minutes reamins to weather packet
	if (minutes_to_wx == 1) {
		// hardcoded to 2 minutes

		switch(io_vbat_r_state) {
		case POOL_VBAT_R_TURNED_ON:
			io_vbat_r_state = POOL_VBAT_R_TURNED_OFF;

			// turn off VBAT_R
			io___cntrl_vbat_r_disable();

			break;
		case POOL_VBAT_R_TURNED_OFF:
			// wait few more seconds before turning on back
			io_vbat_r_state = POOL_VBAT_R_WAIT;

			break;
		case POOL_VBAT_R_WAIT:
			io_vbat_r_state = POOL_VBAT_R_DONT_SWITCH;

			// turn back on
			io___cntrl_vbat_r_enable();

			break;
		case POOL_VBAT_R_DONT_SWITCH:
			// VBAT_R has been already power cycled, do nothing more
			break;
		}
	}
	else {
		// don't do nothing with VBAT_R and keep it turned on
		io_vbat_r_state = POOL_VBAT_R_TURNED_ON;
	}
}

#endif

