/*
 * analog_anemometer.c
 *
 *  Created on: 25.12.2019
 *      Author: mateusz
 */

#include "drivers/analog_anemometer.h"

#include <stdint.h>
#include <string.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_dma.h>
#include "drivers/gpio_conf.h"
#include "drivers/dma_helper_functions.h"
#include "rte_wx.h"

#define MINUM_PULSE_LN 15
#define MAXIMUM_PULSE_SLEW_RATE 4000

#define UF_MAXIMUM_FREQUENCY 32767

// an array where DMA will store values of the timer latched by compare-capture input
uint16_t analog_anemometer_windspeed_pulses_time[ANALOG_ANEMOMETER_SPEED_PULSES_N];

// an array with calculated times between pulses
uint16_t analog_anemometer_time_between_pulses[ANALOG_ANEMOMETER_SPEED_PULSES_N];

// a static copy of impulse-meters/second contact
uint16_t analog_anemometer_pulses_per_m_s_constant = 0;

// a flag which will be raised if not enought pulses has been copied by a DMA before a timer overflows
uint8_t analog_anemometer_timer_has_been_fired = 0;

uint8_t analog_anemometer_slew_limit_fired = 0;

uint8_t analog_anemometer_deboucing_fired = 0;

DMA_InitTypeDef DMA_InitStruct;

// direction recalculated from v/f
uint16_t analog_anemometer_direction = 0;

// scaling value which sets the upper value in percents of the frequency in relation to 32767 Hz
// translating this to a voltage at an input of the U/f converter this sets a maximum ratio of the
// potentiometer inside the direction
uint8_t analog_anemometer_b_coeff = 100;

uint8_t analog_anemometer_a_coeff = 10;

// this controls if the direction increases (1) od decreaes (-1) with the frequency
int8_t analog_anemometer_direction_pol = 1;

uint16_t analog_anemometer_last_direction_cnt = 0;

void analog_anemometer_init(uint16_t pulses_per_meter_second, float anemometer_lower_boundary,
		float anemometer_upper_boundary, uint8_t direction_polarity) {

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	volatile float a = 0.0f, b = 0.0f;

	analog_anemometer_pulses_per_m_s_constant = pulses_per_meter_second;
	a = -360 / (360 * anemometer_lower_boundary - 360 * anemometer_upper_boundary);
	b = (360 * anemometer_lower_boundary * 360) / (anemometer_lower_boundary * 360 - anemometer_upper_boundary * 360);
	//analog_anemometer_b_coeff = (uint8_t)(anemometer_upper_boundary / 100);
	analog_anemometer_direction_pol = direction_polarity;

	// initializing arrays;
	memset(analog_anemometer_windspeed_pulses_time, 0x00, ANALOG_ANEMOMETER_SPEED_PULSES_N);
	memset(analog_anemometer_time_between_pulses, 0x00, ANALOG_ANEMOMETER_SPEED_PULSES_N);

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

	return;
}

void analog_anemometer_timer_irq(void) {
	analog_anemometer_timer_has_been_fired = 1;
}

void analog_anemometer_dma_irq(void) {
	int i = 0;
	uint16_t pulse_ln = 0;
	uint16_t previous_pulse_ln = 0;
	uint16_t minimum_pulse_ln = 60000;
	uint16_t maximum_pulse_ln = 0;

	// resetting flags
	analog_anemometer_slew_limit_fired = 0;
	analog_anemometer_deboucing_fired = 0;

	// checking if timer overflowed (raised an iterrupt)
	if (analog_anemometer_timer_has_been_fired == 1) {
		rte_wx_windspeed_pulses = 1;

		// reseting array to default values
		for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
			analog_anemometer_windspeed_pulses_time[i] = 0;

		// restarting the DMA channel
		dma_helper_start_ch7(&DMA_InitStruct);

		return;
	}

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
		}
	}

	// limiting slew rate
	for (i = 1; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++) {
		previous_pulse_ln = analog_anemometer_time_between_pulses[i - 1];
		pulse_ln = analog_anemometer_time_between_pulses[i];

		// skipping pulses erased by debouncing
		if (pulse_ln == 0 || previous_pulse_ln == 0) {
			continue;
		}

		int32_t diff = pulse_ln - previous_pulse_ln;

		// if current inter-pulse time is much longer than previous (some pulse is missing?)
		if ( diff > MAXIMUM_PULSE_SLEW_RATE ) {
			analog_anemometer_time_between_pulses[i] = previous_pulse_ln + ((uint32_t)MAXIMUM_PULSE_SLEW_RATE >> 3);
			analog_anemometer_slew_limit_fired = 1;
		}
		// if previous inter-pulse time is much longer than current
		else if (diff < -MAXIMUM_PULSE_SLEW_RATE){
			analog_anemometer_time_between_pulses[i - 1] = pulse_ln + ((uint32_t)MAXIMUM_PULSE_SLEW_RATE >> 3);
			analog_anemometer_slew_limit_fired = 1;
		}
		// if this pulse time is ok do nothing.
		else {
			;
		}
	}

	// find maximum and minimum values within inter-pulses times
	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++) {

		pulse_ln = analog_anemometer_time_between_pulses[i];

		// skipping pulses erased by debouncing
		if (pulse_ln == 0)
			continue;

		// find maximum and minimum values within pulses duration
		if (pulse_ln < minimum_pulse_ln)
			minimum_pulse_ln = pulse_ln;

		if (pulse_ln > maximum_pulse_ln)
			maximum_pulse_ln = pulse_ln;

	}

	// calculating the target inter-pulse duration
	rte_wx_windspeed_pulses = (uint16_t)((maximum_pulse_ln + minimum_pulse_ln) / 2);

	// resetting the timer
	analog_anemometer_timer_has_been_fired = 0;

	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
		analog_anemometer_windspeed_pulses_time[i] = 0;

	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
		analog_anemometer_time_between_pulses[i] = 0;

	dma_helper_start_ch7(&DMA_InitStruct);

	return;
}

/**
 * This functions takes the average time between two pulses expressed as
 * a multiplicity of one millisecond (2500 equals two and half of a second)
 * and converts it to the windspeed in 0.1 m/s incremenets (4 equals to .4m/s, 18 equals to 1.8m/s)
 */
uint32_t analog_anemometer_get_ms_from_pulse(uint16_t inter_pulse_time) {
	uint32_t output = 0;

	uint32_t scaled_pulses_frequency = 1000000 / (inter_pulse_time * 10);
	output = scaled_pulses_frequency / (analog_anemometer_pulses_per_m_s_constant);

	return output;
}

int16_t analog_anemometer_direction_handler(void) {

	TIM_Cmd(TIM3, DISABLE);

	// getting current counter value
	uint16_t current_value = TIM_GetCounter(TIM3);

	// if the value is greater than maximum one just ignore
	if (current_value > UF_MAXIMUM_FREQUENCY) {

		// and reinitialize the timer before returning from the function
		analog_anemometer_direction_reset();

		return rte_wx_winddirection_last;
	}

	// upscaling by factor of 1000 to omit usage of the floating point arithmetics
	uint32_t upscaled_frequecy = current_value * 1000;

	// calculating the ratio between the current input frequency and the maximum one
	uint16_t ratio_of_upscaled_frequency = upscaled_frequecy / UF_MAXIMUM_FREQUENCY;

	// converting the upscaled ratio into the upscaled angle
	uint32_t upscaled_angle = ratio_of_upscaled_frequency * 360;

	uint32_t angle_adjusted_to_real_freq_borders = analog_anemometer_a_coeff +
											upscaled_angle * analog_anemometer_b_coeff;

	// downscaling the angle
	uint16_t downscaled_angle = angle_adjusted_to_real_freq_borders / 100000;

	// adjusting to polarity of the signal
	downscaled_angle *= analog_anemometer_direction_pol;

	analog_anemometer_last_direction_cnt = 0;

	rte_wx_winddirection_last = downscaled_angle;

	TIM_SetCounter(TIM3, 0);

	TIM_Cmd(TIM3, ENABLE);

	return downscaled_angle;
}

void analog_anemometer_direction_reset(void) {

	// stopping the timer
	TIM_Cmd(TIM3, DISABLE);

	// resetting it
	TIM_SetCounter(TIM3, 0);

	// end then restarting once again
	TIM_Cmd(TIM3, ENABLE);
}