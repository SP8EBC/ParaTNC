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
#include "rte_wx.h"

#define MINUM_PULSE_LN 15
#define MAXIMUM_PULSE_SLEW_RATE 4000

// an array where DMA will store values of the timer latched by compare-capture input
uint16_t analog_anemometer_windspeed_pulses_time[ANALOG_ANEMOMETER_SPEED_PULSES_N];

uint16_t analog_anemometer_pulses_durations[ANALOG_ANEMOMETER_SPEED_PULSES_N];

// a static copy of impulse-meters/second contact
uint16_t analog_anemometer_pulses_per_ms_constant = 0;

uint8_t analog_anemometer_timer_has_been_fired = 0;

DMA_InitTypeDef DMA_InitStruct;

void analog_anemometer_init(uint16_t pulses_per_ms, uint16_t mvolts_for_1deg,
		uint16_t mvolts_for_359deg, uint8_t reversed) {

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	analog_anemometer_pulses_per_ms_constant = pulses_per_ms;

	// initializing arrays;
	memset(analog_anemometer_windspeed_pulses_time, 0x00, ANALOG_ANEMOMETER_SPEED_PULSES_N);
	memset(analog_anemometer_pulses_durations, 0x00, ANALOG_ANEMOMETER_SPEED_PULSES_N);

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

	DMA_Init(DMA1_Channel7, &DMA_InitStruct);
	DMA1_Channel7->CCR |= DMA_CCR7_EN;
	DMA1_Channel7->CCR |= DMA_CCR7_TCIE;

	NVIC_EnableIRQ( DMA1_Channel7_IRQn );


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

	// checking if timer overflowed (raised an iterrupt)
	if (analog_anemometer_timer_has_been_fired == 1) {
		rte_wx_windspeed_pulses = 1;

		// reseting array to default values
		for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
			analog_anemometer_windspeed_pulses_time[i] = 0;

		DMA_Init(DMA1_Channel7, &DMA_InitStruct);
		DMA1_Channel7->CCR |= DMA_CCR7_EN;
		DMA1_Channel7->CCR |= DMA_CCR7_TCIE;

		return;
	}

	// calculating pulses duration time
	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N - 1; i++) {
		pulse_ln = analog_anemometer_windspeed_pulses_time[i + 1] -
				analog_anemometer_windspeed_pulses_time[i];

		analog_anemometer_pulses_durations[i] = pulse_ln;
	}

	// debouncing captured times
	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++) {
		if (analog_anemometer_pulses_durations[i] < MINUM_PULSE_LN)
			analog_anemometer_pulses_durations[i] = 0;
	}

	// limiting slew rate
	for (i = 1; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++) {
		previous_pulse_ln = analog_anemometer_pulses_durations[i - 1];
		pulse_ln = analog_anemometer_pulses_durations[i];

		// skipping pulses erased by debouncing
		if (pulse_ln == 0 || previous_pulse_ln == 0) {
			continue;
		}

		int32_t diff = pulse_ln - previous_pulse_ln;

		// if current pulse is much longer than previous
		if ( diff > MAXIMUM_PULSE_SLEW_RATE ) {
			analog_anemometer_pulses_durations[i] = previous_pulse_ln + MAXIMUM_PULSE_SLEW_RATE;
		}
		// if previous pulse is much longer than current
		else {
			analog_anemometer_pulses_durations[i - 1] = pulse_ln + MAXIMUM_PULSE_SLEW_RATE;
		}
	}

	// find maximum and minimum values within pulses duration
	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++) {

		pulse_ln = analog_anemometer_pulses_durations[i];

		// skipping pulses erased by debouncing
		if (pulse_ln == 0)
			continue;

		// find maximum and minimum values within pulses duration
		if (pulse_ln < minimum_pulse_ln)
			minimum_pulse_ln = pulse_ln;

		if (pulse_ln > maximum_pulse_ln)
			maximum_pulse_ln = pulse_ln;

	}

	// calculating the target pulse duration
	rte_wx_windspeed_pulses = (uint16_t)((maximum_pulse_ln + minimum_pulse_ln) / 2);

	// resetting the timer
	analog_anemometer_timer_has_been_fired = 0;

	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
		analog_anemometer_windspeed_pulses_time[i] = 0;

	for (i = 0; i < ANALOG_ANEMOMETER_SPEED_PULSES_N; i++)
		analog_anemometer_pulses_durations[i] = 0;

	DMA_Init(DMA1_Channel7, &DMA_InitStruct);
	DMA1_Channel7->CCR |= DMA_CCR7_EN;
	DMA1_Channel7->CCR |= DMA_CCR7_TCIE;

	return;
}
