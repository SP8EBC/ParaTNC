/*
 * delay.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include "main.h"

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <task.h>

int32_t preset_delay_msecs = 0;
uint8_t preset_use_random = 0;

// counter decrement in Systick handler
volatile int32_t delay_cnt = 0;

void delay_fixed (int32_t delay_in_msecs)
{

	delay_cnt = delay_in_msecs;

	/* Block for 60 seconds. */
	const TickType_t xDelay = delay_in_msecs / portTICK_PERIOD_MS;

	if (main_rtos_is_runing == 0) {
		while (delay_cnt > 0)
			;
	}
	else {
		vTaskDelay (xDelay);
	}

	return;
}

uint32_t delay_fixed_with_count (int32_t delay_in_msecs)
{

	uint32_t ret = 0;

	delay_cnt = delay_in_msecs;

	/* Block for 60 seconds. */
	const TickType_t xDelay = delay_in_msecs / portTICK_PERIOD_MS;

	if (main_rtos_is_runing == 0) {
		while (delay_cnt > 0) {
			ret++;
		}
	}
	else {
		vTaskDelay (xDelay);
	}

	return ret;
}

void delay_random (void)
{

	uint16_t sample = main_get_adc_sample ();

	// random element of delay value could vary from 0 to 300msecs in 20msec steps
	delay_cnt = (int32_t)(preset_delay_msecs / 4) + (sample % 15) * 20;
}

void delay_set (uint16_t delay_in_msecs, uint8_t randomize)
{
	preset_delay_msecs = delay_in_msecs * 50;

	if (randomize == 1) {
		preset_use_random = 1;
	}
	else {
		preset_use_random = 0;
	}
}

void delay_from_preset (void)
{

	delay_cnt = preset_delay_msecs;

	while (delay_cnt > (int32_t)0)
		;

	if (preset_use_random == 1) {
		delay_random ();
	}
}
