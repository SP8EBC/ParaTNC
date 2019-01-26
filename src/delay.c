/*
 * delay.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include "main.h"

uint16_t preset_delay_msecs = 0;
uint8_t preset_use_random = 0;

// counter decrement in Systick handler
uint16_t delay_cnt = 0;

void delay_fixed(uint16_t delay_in_msecs) {

	delay_cnt = delay_in_msecs;

	while(delay_cnt > 0);

	return;

}

void delay_random(void) {

	uint16_t sample = main_get_adc_sample();

	// random element of delay value could vary from 0 to 300msecs in 20msec steps
	delay_cnt = (uint16_t)(preset_delay_msecs / 4) + (sample % 15) * 20;

}

void delay_set(uint16_t delay_in_msecs, uint8_t randomize) {
	preset_delay_msecs = delay_in_msecs;

	if (randomize == 1) {
		preset_use_random = 1;
	}
	else {
		preset_use_random = 0;
	}
}

void delay_from_preset(void) {

	delay_cnt = preset_delay_msecs;

	while(delay_cnt > 0);

	if (preset_use_random == 1) {
		delay_random();
	}


}
