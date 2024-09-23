/*
 * analog_anemometer.c
 *
 *  Created on: 25.12.2019
 *      Author: mateusz
 */

#include "station_config.h"
#include "station_config_target_hw.h"

#if defined(_ANEMOMETER_ANALOGUE_SPARKFUN) || defined(_ANEMOMETER_ANALOGUE)

#define WIND_DEBUG

#include "drivers/analog_anemometer.h"

#include <stdint.h>
#include <string.h>
#include "drivers/dma_helper_functions.h"
#include "rte_wx.h"
#include "main.h"
#include "wx_handler.h"
#include "LedConfig.h"
#include "io.h"

#include <event_log.h>
#include <events_definitions/events_drv_anemometer.h>

#ifdef STM32F10X_MD_VL
#include <stm32f10x_tim.h>
#include <stm32f10x_dma.h>
#include <drivers/f1/gpio_conf_stm32f1x.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_tim.h>
#include <stm32l4xx_ll_dma.h>
#include <stm32l4xx_ll_gpio.h>
#endif

#define MINUM_PULSE_LN 15
#define MAXIMUM_PULSE_SLEW_RATE 4000

#define UF_MAXIMUM_FREQUENCY 8280//32767
#define UPSCALED_MAX_ANGLE 		(360 * 100)
#define UPSCALED_MAX_ANGLE_2 	(360 * 10)

#ifdef STM32F10X_MD_VL
	DMA_InitTypeDef DMA_InitStruct;
#endif

#ifdef STM32L471xx
	LL_DMA_InitTypeDef DMA_InitStruct;
#endif

// an array where DMA will store values of the timer latched by compare-capture input
uint16_t analog_anemometer_windspeed_pulses_time[ANALOG_ANEMOMETER_SPEED_PULSES_N];

// an array with calculated times between pulses
uint16_t analog_anemometer_time_between_pulses[ANALOG_ANEMOMETER_SPEED_PULSES_N];

#ifdef WIND_DEBUG
uint16_t analog_anemometer_direction_timer_values[ANALOG_ANEMOMETER_SPEED_PULSES_N];
uint8_t analog_anemometer_direction_timer_values_it = 0;
#endif

// a static copy of impulse-meters/second constant. This value expresses
// how many pulses in 10 seconds measurement time gives 1 m/s.
// Value of ten means that if within 10 second period 10 pulses were detected it gives
// 1m/s
uint16_t analog_anemometer_pulses_per_m_s_constant = 0;

// a flag which will be raised if not enought pulses has been copied by a DMA before a timer overflows
uint8_t analog_anemometer_timer_has_been_fired = 0;

uint8_t analog_anemometer_slew_limit_fired = 0;

uint8_t analog_anemometer_deboucing_fired = 0;

uint8_t analog_anemometer_direction_doesnt_work = 0;

// direction recalculated from v/f
uint16_t analog_anemometer_direction = 0;

// scaling value which sets the upper value in percents of the frequency in relation to 32767 Hz
// translating this to a voltage at an input of the U/f converter this sets a maximum ratio of the
// potentiometer inside the direction
int16_t analog_anemometer_b_coeff = 100;

int16_t analog_anemometer_a_coeff = 10;

// this controls if the direction increases (1) od decreaes (-1) with the frequency
int8_t analog_anemometer_direction_pol = 1;

uint16_t analog_anemometer_last_direction_cnt = 0;

// incremeneted by one if condition below is true
//
//			// if the value is greater than maximum one just ignore
//				if (current_value > UF_MAXIMUM_FREQUENCY) {
//
//					// and reinitialize the timer before returning from the function
//					analog_anemometer_direction_reset();
//
//					return rte_wx_winddirection_last;
//				}
uint8_t analog_anemometer__direction_freq_too_high = 0;

#ifdef _ANEMOMETER_ANALOGUE
int8_t  analog_anemometer_direction_mode = DIRECTION_REGULAR;
#endif

#ifdef _ANEMOMETER_ANALOGUE_SPARKFUN
int8_t  analog_anemometer_direction_mode = DIRECTION_SPARKFUN;
#endif

// this array consists voltage ranges to calculate
const int16_t analog_anemometer_direction_sparkfun_ranges[16][3] = {
		{4018, 4112, 112},
		{4112, 4182, 67},
		{4182, 4296, 90},
		{4296, 4537, 157},
		{4537, 4840, 135},
		{4840, 5107, 202},
		{5107, 5529, 180},
		{5529, 5982, 22},
		{5982, 6490, 45},
		{6490, 6935, 247},
		{6935, 7201, 225},
		{7201, 7607, 337},
		{7607, 7932, 0},
		{7932, 8349, 292},
		{8349, 8746, 270},
		{8746, 9276, 315}
};

void analog_anemometer_init(uint16_t pulses_per_meter_second, uint8_t anemometer_lower_boundary,
		uint8_t anemometer_upper_boundary, uint8_t direction_polarity) {

#ifdef STM32F10X_MD_VL
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
#endif

#ifdef STM32L471xx
	LL_TIM_InitTypeDef TIM_InitTypeDef;
	LL_TIM_IC_InitTypeDef TIM_IC_InitTypeDef;
	LL_GPIO_InitTypeDef GPIO_InitTypeDef;
#endif

	analog_anemometer_pulses_per_m_s_constant = pulses_per_meter_second;

	// Solving the linear equation to find 'a' and 'b' coefficient needed to rescale the wind direction
	// from raw value calculated from an input frequency, to physical value which includes the lower and
	// the higher value of anemometer resistance / frequency
	// * 100
	analog_anemometer_a_coeff = ((10000 * -UPSCALED_MAX_ANGLE) / (UPSCALED_MAX_ANGLE * anemometer_lower_boundary - UPSCALED_MAX_ANGLE * anemometer_upper_boundary));
	// * 10
	analog_anemometer_b_coeff = (UPSCALED_MAX_ANGLE_2 * anemometer_lower_boundary * UPSCALED_MAX_ANGLE_2) / (anemometer_lower_boundary * UPSCALED_MAX_ANGLE_2 - anemometer_upper_boundary * UPSCALED_MAX_ANGLE_2);

	// signal polariy
	analog_anemometer_direction_pol = direction_polarity;

	// initializing arrays;
	memset(analog_anemometer_windspeed_pulses_time, 0x00, ANALOG_ANEMOMETER_SPEED_PULSES_N);
	memset(analog_anemometer_time_between_pulses, 0x00, ANALOG_ANEMOMETER_SPEED_PULSES_N);
#ifdef WIND_DEBUG
	memset(analog_anemometer_direction_timer_values, 0x00, ANALOG_ANEMOMETER_SPEED_PULSES_N);
#endif

#ifdef STM32F10X_MD_VL
	// enabling the clock for TIM17
	RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	// Configuring a pin where pulses from anemometer are connected
	Configure_GPIO(GPIOB,9,FLOATING_INPUT);

	// resetting the timer to defaults
	TIM_DeInit(TIM17);

	// initializing structure with default values
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);

	TIM_TimeBaseInitStruct.TIM_Prescaler = 23999;					// PSC 23999
	TIM_TimeBaseInitStruct.TIM_Period = 60000;					// ARR
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

	// Configuring basics of thr timer
	TIM_TimeBaseInit(TIM17, &TIM_TimeBaseInitStruct);

	// Enabling capture input
	TIM_TIxExternalClockConfig(TIM17, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0);

	// Starting timer
	TIM_Cmd(TIM17, ENABLE);

	// Enabling a DMA request signal from first capture-compare channel
	TIM_DMACmd(TIM17, TIM_DMA_CC1, ENABLE);

	// Enabling an interrupt
	TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);
	NVIC_EnableIRQ( TIM1_TRG_COM_TIM17_IRQn );

	// Initializing the struct with DMA configuration
	DMA_StructInit(&DMA_InitStruct);

	// De initializing DMA1
	DMA_DeInit(DMA1_Channel7);

	DMA_InitStruct.DMA_BufferSize = ANALOG_ANEMOMETER_SPEED_PULSES_N;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)analog_anemometer_windspeed_pulses_time;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&TIM17->CCR1;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

	dma_helper_start_ch7(&DMA_InitStruct);

	NVIC_EnableIRQ( DMA1_Channel7_IRQn );

	// Initializing direction

	// Configuring PD2 as an input for TIM3_ETR
	Configure_GPIO(GPIOD,2,FLOATING_INPUT);

	// initializing structure with default values
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);

	// using default values of InitStruct
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

	// enabling an external trigger to the TIM3
	TIM_ETRClockMode2Config(TIM3, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_Inverted, 0);

	// Starting timer
	TIM_Cmd(TIM3, ENABLE);

	// disable an interrupt from TIMER3
	NVIC_DisableIRQ(TIM3_IRQn);

#endif


#ifdef STM32L471xx
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_9;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_2;

	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);

#ifdef HI_SPEED
	// set parameters for TIM4 used for windspeed
	TIM_InitTypeDef.Prescaler = 47999;
#else
	// set parameters for TIM4 used for windspeed
	TIM_InitTypeDef.Prescaler = 23999;
#endif
	// set parameters for TIM4 used for windspeed
	TIM_InitTypeDef.Prescaler = 47999;
	TIM_InitTypeDef.Autoreload = 60000;
	TIM_InitTypeDef.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitTypeDef.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitTypeDef.RepetitionCounter = 0;

	// deinitialize TIM4 (windspeed)
	LL_TIM_DeInit(TIM4);

	LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_5);

	// initialize timer basics
	LL_TIM_Init(TIM4, &TIM_InitTypeDef);

	// set parameters for TIM4 capture/compare 3 mapped to TI4 (pin PB9)
	TIM_IC_InitTypeDef.ICFilter = LL_TIM_IC_FILTER_FDIV1;
	TIM_IC_InitTypeDef.ICPolarity = LL_TIM_IC_POLARITY_RISING;
	TIM_IC_InitTypeDef.ICPrescaler = 0;
	TIM_IC_InitTypeDef.ICActiveInput = LL_TIM_ACTIVEINPUT_INDIRECTTI;	// 10: CC3 channel is configured as input, IC3 is mapped on TI4

	// configure and activate fourth channel
	LL_TIM_IC_Init(TIM4, LL_TIM_CHANNEL_CH3, &TIM_IC_InitTypeDef);

	LL_TIM_EnableDMAReq_UPDATE(TIM4);

	// enable DMA request for fourth channel
	LL_TIM_EnableDMAReq_CC3(TIM4);

	DMA_InitStruct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
	DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)analog_anemometer_windspeed_pulses_time;
	DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
	DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
	DMA_InitStruct.Mode = LL_DMA_MODE_NORMAL;
	DMA_InitStruct.NbData = ANALOG_ANEMOMETER_SPEED_PULSES_N;
	DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&TIM4->CCR3;
	DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
	DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_MEMORY_NOINCREMENT;
	DMA_InitStruct.PeriphRequest = LL_DMA_REQUEST_6; // LL_DMAMUX_REQ_TIM4_CH3

	LL_DMA_Init(DMA1, LL_DMA_CHANNEL_5, &DMA_InitStruct);

	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_5);

	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);

	LL_TIM_EnableCounter(TIM4);

	NVIC_EnableIRQ( DMA1_Channel5_IRQn );

	// direction
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_2;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_2;

	LL_GPIO_Init(GPIOD, &GPIO_InitTypeDef);

	LL_TIM_StructInit(&TIM_InitTypeDef);

	LL_TIM_Init(TIM3, &TIM_InitTypeDef);

	//	TIM_ETRClockMode2Config(TIM3, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_Inverted, 0);
	LL_TIM_EnableExternalClock(TIM3);

	// invert ETR polarity
	//TIM3->SMCR |= (1 << 15);
	LL_TIM_ConfigETR(TIM3, LL_TIM_ETR_POLARITY_INVERTED, LL_TIM_ETR_PRESCALER_DIV1, LL_TIM_ETR_FILTER_FDIV1);

	LL_TIM_EnableCounter(TIM3);

	NVIC_EnableIRQ( TIM3_IRQn );
#endif

	analog_anemometer_timer_has_been_fired = 0;

	return;
}

/**
 * This function denitialize everything related to analog anemometr to eliminate transient glitches
 * in windspeed
 */
void analog_anemometer_deinit(void) {
#ifdef STM32L471xx
	// disable interrupt generated by DMA channel used for windspeed
	LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_5);

	// disable DMA channel used for windspeed measurement
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);

	// disable DMA request for fourth channel which is used for windspeed
	LL_TIM_DisableDMAReq_CC3(TIM4);

	// disable timer itself (windspeed)
	LL_TIM_DisableCounter(TIM4);

	LL_TIM_DisableCounter(TIM3);
#endif
}

/**
 * windspeed
 */
void analog_anemometer_timer_irq(void) {
	analog_anemometer_timer_has_been_fired = 1;

	rte_wx_analog_anemometer_counter_timer_has_been_fired++;
}

/**
 * windspeed
 */
void analog_anemometer_dma_irq(void) {
	int i = 0;
	uint16_t pulse_ln = 0;
	uint16_t previous_pulse_ln = 0;
	uint16_t shorter_pulse = 0;
	volatile uint16_t minimum_pulse_ln = 60000;
	volatile uint16_t previous_minimum_pulse_ln = 60000;	// first value bigger than minimal one
	volatile uint16_t maximum_pulse_ln = 0;
	volatile uint16_t previous_maximum_pulse_ln = 0;		//
	volatile uint16_t slew_rate_limit = 60000;

	// resetting flags
	analog_anemometer_slew_limit_fired = 0;
	analog_anemometer_deboucing_fired = 0;

	// checking if timer overflowed (raised an iterrupt)
	if (analog_anemometer_timer_has_been_fired == 1) {

	    event_log_sync(EVENT_WARNING, EVENT_SRC_DRV_ANEMOMETER,
	    		  EVENTS_DRV_ANEMOMETER_WARN_NO_PULSES_INT_FIRED,
				  0, 0, rte_wx_windspeed_pulses,
				  analog_anemometer_windspeed_pulses_time[0],
				  analog_anemometer_windspeed_pulses_time[1],
				  analog_anemometer_windspeed_pulses_time[2]);

		rte_wx_windspeed_pulses = 1;

		analog_anemometer_timer_has_been_fired = 0;

		// reseting array to default values
		for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
			analog_anemometer_windspeed_pulses_time[i] = 0;

		// restarting the DMA channel
		dma_helper_start_ch7(&DMA_InitStruct);

		return;
	}

	// blinking the led - led will blink every 10 pulses, so if wind is 1m/s it will blink every 10 seconds
	led_blink_led2_botoom();

	// calculating time between pulses
	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N - 1; i++) {
		pulse_ln = analog_anemometer_windspeed_pulses_time[i + 1] -
				analog_anemometer_windspeed_pulses_time[i];

		analog_anemometer_time_between_pulses[i] = pulse_ln;
	}

	// debouncing captured pulse times
	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N - 1; i++) {
		if (analog_anemometer_time_between_pulses[i] < MINUM_PULSE_LN) {
			analog_anemometer_time_between_pulses[i] = 0;
			analog_anemometer_deboucing_fired = 1;
			rte_wx_analog_anemometer_counter_deboucing_fired++;
		}
	}

	// limiting slew rate
	for (i = 1; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++) {
		previous_pulse_ln = analog_anemometer_time_between_pulses[i - 1];
		pulse_ln = analog_anemometer_time_between_pulses[i];

		// checking which inter-pulse time is shorter
		if (previous_pulse_ln < pulse_ln)
			shorter_pulse = previous_pulse_ln;
		else
			shorter_pulse = pulse_ln;

		// calculating maximum slew rate basing on current inter pulse ln
		if (shorter_pulse >= 1000) {
			// 1 meter per second
			slew_rate_limit = shorter_pulse;
		}
		else if (shorter_pulse >= 200 && shorter_pulse < 1000) {
			// from 1 to 5 meters per second
			slew_rate_limit = shorter_pulse >> 1;
		}
		else {
			// more than 5 meters per second
			slew_rate_limit = shorter_pulse >> 2;
		}

		// skipping pulses erased by debouncing
		if (pulse_ln == 0 || previous_pulse_ln == 0) {
			continue;
		}

		int32_t diff = pulse_ln - previous_pulse_ln;

		// if current inter-pulse time is much longer than previous (some pulse is missing?)
		if ( diff > slew_rate_limit ) {

		    event_log_sync(EVENT_WARNING, EVENT_SRC_DRV_ANEMOMETER,
		    		  EVENTS_DRV_ANEMOMETER_WARN_EXCESIVE_SLEW,
					  0, 0,
					  pulse_ln, previous_pulse_ln,
					  diff, slew_rate_limit);

			analog_anemometer_time_between_pulses[i] = previous_pulse_ln + ((uint32_t)slew_rate_limit);
			analog_anemometer_slew_limit_fired = 1;
			rte_wx_analog_anemometer_counter_slew_limit_fired++;
		}
		// if previous inter-pulse time is much longer than current
		else if (diff < -slew_rate_limit){
			analog_anemometer_time_between_pulses[i - 1] = pulse_ln + ((uint32_t)slew_rate_limit);
			analog_anemometer_slew_limit_fired = 1;
			rte_wx_analog_anemometer_counter_slew_limit_fired++;
		}
		// if this pulse time is ok do nothing.
		else {
			;
		}
	}

	minimum_pulse_ln = 60000;
	previous_minimum_pulse_ln = 60000;

	maximum_pulse_ln = 0;
	previous_maximum_pulse_ln = 0;

	// find maximum and minimum values within inter-pulses times
	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++) {

		pulse_ln = analog_anemometer_time_between_pulses[i];

		// skipping pulses erased by debouncing
		if (pulse_ln == 0)
			continue;

		// find maximum and minimum values within pulses duration
		if (pulse_ln < minimum_pulse_ln) {

			// check if 'previous' has a default value of 60k
			if (previous_minimum_pulse_ln == 60000) {
				// if yes store the current value to handle a situation than whole
				// circular buffer conssit the same value
				previous_minimum_pulse_ln = pulse_ln;
			}
			else {
				// copying previous minimal value
				previous_minimum_pulse_ln = minimum_pulse_ln;
			}

			// setting current minimal value
			minimum_pulse_ln = pulse_ln;
		}

		if (pulse_ln > maximum_pulse_ln) {

			if (previous_maximum_pulse_ln == 0) {
				previous_maximum_pulse_ln = pulse_ln;
			}
			else {
				previous_maximum_pulse_ln = maximum_pulse_ln;
			}

			maximum_pulse_ln = pulse_ln;
		}

	}

	// calculating the target inter-pulse duration
	rte_wx_windspeed_pulses = (uint16_t)((previous_maximum_pulse_ln + previous_minimum_pulse_ln) / 2);

	// resetting the timer
	analog_anemometer_timer_has_been_fired = 0;

	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
		analog_anemometer_windspeed_pulses_time[i] = 0;

	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
		analog_anemometer_time_between_pulses[i] = 0;

	dma_helper_start_ch7(&DMA_InitStruct);

#ifdef STM32F10X_MD_VL
	// Stopping timer
	TIM_Cmd(TIM17, DISABLE);

	// Resetting the counter
	TIM_SetCounter(TIM17, 0);

	// Enabling counter once again
	TIM_Cmd(TIM17, ENABLE);
#endif

#ifdef STM32L471xx

	LL_TIM_DisableCounter(TIM4);

	LL_TIM_SetCounter(TIM4, 0);

	LL_TIM_EnableCounter(TIM4);

#endif

	return;
}

/**
 * This functions takes the average time between two pulses expressed as
 * a multiplicity of one millisecond (2500 equals two and half of a second)
 * and converts it to the windspeed in 0.1 m/s incremenets (4 equals to .4m/s, 18 equals to 1.8m/s)
 */
uint32_t analog_anemometer_get_ms_from_pulse(uint16_t inter_pulse_time) {
	uint32_t output = 0;

	uint32_t scaled_pulses_frequency = 1000000 / (inter_pulse_time * 10);		// *100 from real value

	if (inter_pulse_time > 5)
		output = scaled_pulses_frequency / (analog_anemometer_pulses_per_m_s_constant);
	else
		output = 0;

	return output;
}

int16_t analog_anemometer_direction_handler(void) {

	uint16_t downscaled_angle = 0;

#ifdef STM32F10X_MD_VL
	if (io_get_5v_isol_sw___cntrl_vbat_s() == 0) {

#else
	if (io_get_cntrl_vbat_c() == 0) {
#endif
		return rte_wx_winddirection_last;
	}

#ifdef STM32F10X_MD_VL
	TIM_Cmd(TIM3, DISABLE);

	// getting current counter value
	uint16_t current_value = TIM_GetCounter(TIM3);

	// if the counter value is zero it means that probably U/f converter isn't running
	if (current_value == 0) {
		TIM_SetCounter(TIM3, 0);

		TIM_Cmd(TIM3, ENABLE);

		analog_anemometer_direction_doesnt_work = 1;

		return rte_wx_winddirection_last;
	}
#endif

#ifdef STM32L471xx
	LL_TIM_DisableCounter(TIM3);

	// getting current counter value
	volatile uint16_t current_value = LL_TIM_GetCounter(TIM3);//TIM_GetCounter(TIM3);

	// if the counter value is zero it means that probably U/f converter isn't running
	if (current_value == 0) {
	    event_log_sync(EVENT_ERROR, EVENT_SRC_DRV_ANEMOMETER,
	    		  EVENTS_DRV_ANEMOMETER_ERROR_UF_CONV_NOT_WORKING,
				  analog_anemometer_timer_has_been_fired, analog_anemometer_slew_limit_fired,
				  analog_anemometer_windspeed_pulses_time[0], analog_anemometer_windspeed_pulses_time[1],
				  analog_anemometer_windspeed_pulses_time[2], analog_anemometer_windspeed_pulses_time[3]);

		LL_TIM_SetCounter(TIM3, 0);

		LL_TIM_EnableCounter(TIM3);

		analog_anemometer_direction_doesnt_work = 1;

		rte_wx_analog_anemometer_counter_direction_doesnt_work++;

		return rte_wx_winddirection_last;
	}

#endif

	// update the last
	wx_last_good_wind_time = main_get_master_time();

#ifdef	WIND_DEBUG
	analog_anemometer_direction_timer_values[(analog_anemometer_direction_timer_values_it++) % ANALOG_ANEMOMETER_SPEED_PULSES_N] = current_value;
#endif

	// if the value is greater than maximum one just ignore
	if (current_value > UF_MAXIMUM_FREQUENCY) {

	    event_log_sync(EVENT_ERROR, EVENT_SRC_DRV_ANEMOMETER,
	    		EVENTS_DRV_ANEMOMETER_ERROR_UF_FREQ_TOO_HI,
				current_value, 0,
				analog_anemometer_windspeed_pulses_time[0], analog_anemometer_windspeed_pulses_time[1],
				analog_anemometer_windspeed_pulses_time[2], analog_anemometer_windspeed_pulses_time[3]);

		// and reinitialize the timer before returning from the function
		analog_anemometer_direction_reset();

		analog_anemometer__direction_freq_too_high++;

		return rte_wx_winddirection_last;
	}

	if (analog_anemometer_direction_mode == DIRECTION_REGULAR) {
		// upscaling by factor of 1000 to omit usage of the floating point arithmetics
		uint32_t upscaled_frequecy = current_value * 100;

		// calculating the ratio between the current input frequency and the maximum one
		uint16_t ratio_of_upscaled_frequency = upscaled_frequecy / UF_MAXIMUM_FREQUENCY;		// this val is * 100 from physical ratio

		// converting the upscaled ratio into the upscaled angle
		uint32_t upscaled_angle = ratio_of_upscaled_frequency * 360;			// this val is * 100 from physical

		// rescaling the angle according to lower and higher limit
		int32_t angle_adjusted_to_real_freq_borders = analog_anemometer_a_coeff *
												upscaled_angle + 1000 * analog_anemometer_b_coeff;

		if (angle_adjusted_to_real_freq_borders < 0)
			angle_adjusted_to_real_freq_borders = 0;

		// downscaling the angle
		downscaled_angle = angle_adjusted_to_real_freq_borders / 10000;

		// adjusting to polarity of the signal
		downscaled_angle *= analog_anemometer_direction_pol;
	}
	else if (analog_anemometer_direction_mode == DIRECTION_SPARKFUN) {
		downscaled_angle = analog_anemometer_direction_sparkfun(current_value);
	}
	else {
		;
	}

	analog_anemometer_last_direction_cnt = 0;

	rte_wx_winddirection_last = downscaled_angle;

	// set the led state
	if (rte_wx_winddirection_last > 0 && rte_wx_winddirection_last < 180) {
		led_control_led2_bottom(true);
	}
	else {
		led_control_led2_bottom(false);
	}

#ifdef STM32F10X_MD_VL
	TIM_SetCounter(TIM3, 0);

	TIM_Cmd(TIM3, ENABLE);
#endif

#ifdef STM32L471xx
	LL_TIM_SetCounter(TIM3, 0);

	LL_TIM_EnableCounter(TIM3);
#endif

	return downscaled_angle;
}

int16_t analog_anemometer_direction_sparkfun(uint32_t timer_value) {

	int16_t out = -1;

	// iterate through table which consist ranges of valid timer counter values
	// for each wind direction
	for (int i = 0; i < 16; i++)  {
		if (timer_value >= analog_anemometer_direction_sparkfun_ranges[i][0] &&
			timer_value < analog_anemometer_direction_sparkfun_ranges[i][1]	) {

			out = analog_anemometer_direction_sparkfun_ranges[i][2];

			// exit from the loop and then from the function if the angle has been found.
			break;
		}
	}

	return out;
}

void analog_anemometer_direction_reset(void) {

#ifdef STM32F10X_MD_VL
	// stopping the timer
	TIM_Cmd(TIM3, DISABLE);

	// resetting it
	TIM_SetCounter(TIM3, 0);

	// end then restarting once again
	TIM_Cmd(TIM3, ENABLE);
#endif

#ifdef STM32L471xx
	LL_TIM_DisableCounter(TIM3);

	LL_TIM_SetCounter(TIM3, 0);

	LL_TIM_EnableCounter(TIM3);
#endif
}

analog_wind_qf_t analog_anemometer_get_qf(void) {

	analog_wind_qf_t out;

	if (
			analog_anemometer_slew_limit_fired == 0 &&
			analog_anemometer_deboucing_fired == 0 &&
			analog_anemometer_direction_doesnt_work == 0
			)
	{
		out = AN_WIND_QF_FULL;
	}
	else if (
			analog_anemometer_slew_limit_fired == 1 &&
			analog_anemometer_deboucing_fired == 0 &&
			analog_anemometer_direction_doesnt_work == 0
			)
	{
		out = AN_WIND_QF_DEGRADED_SLEW;
	}
	else if (
			analog_anemometer_slew_limit_fired == 0 &&
			analog_anemometer_deboucing_fired == 1 &&
			analog_anemometer_direction_doesnt_work == 0
			)
	{
		out = AN_WIND_QF_DEGRADED_DEBOUNCE;

	}
	else if (
			analog_anemometer_slew_limit_fired == 1 &&
			analog_anemometer_deboucing_fired == 1 &&
			analog_anemometer_direction_doesnt_work == 0
			)
	{
		out = AN_WIND_QF_DEGRADED;

	}
	else if (
			analog_anemometer_slew_limit_fired == 0 &&
			analog_anemometer_deboucing_fired == 0 &&
			analog_anemometer_direction_doesnt_work == 1
			)
	{
		out = AN_WIND_QF_NOT_AVALIABLE;

	}
	else {
		out = AN_WIND_QF_UNKNOWN;

	}

	if (out != AN_WIND_QF_FULL) {
	    event_log_sync(EVENT_WARNING, EVENT_SRC_DRV_ANEMOMETER,
	    		EVENTS_DRV_ANEMOMETER_WARN_QF_NOT_FULL,
				analog_anemometer_slew_limit_fired, analog_anemometer_deboucing_fired,
				analog_anemometer_direction_doesnt_work, 0,
				0, 0);
	}

	// reseting flags
	analog_anemometer_slew_limit_fired = 0;
	analog_anemometer_deboucing_fired = 0;
	analog_anemometer_direction_doesnt_work = 0;

	return out;

}

#endif
