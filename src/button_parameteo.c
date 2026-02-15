/*
 * button_parameteo.c
 *
 *  Created on: Jun 10, 2023
 *      Author: mateusz
 */

#include "button.h"

#include "./gsm/sim800c.h"
#include "main.h"

#include "aprsis.h"
#include "beacon.h"

#include <stm32l4xx_ll_gpio.h>

#include "fanet_app.h"

/**
 * Used for
 */
uint8_t button_left_previous_state = 1;

uint8_t button_right_previous_state = 1;

void button_check_all (configuration_button_function_t left, configuration_button_function_t right)
{

	/**
	 * Naming convention: There are two buttons on the PCB. Lets call it
	 * left and right, with an assumption that You are holding a PCB
	 * horizontally with a battery and PV connector on bottom left corner.
	 *
	 * Left Button - BTN0 on schematic - connected to PA0
	 * Right Button - BTN1 on schematic - connected to PC3
	 *
	 * Buttons are present only on Hardware Revision C and later! They have
	 * internal wake pull-up enabled, so the io pin is in high state
	 * when a button is not pressed. the button shorts this to ground.
	 */

	// current state of right button
	const uint32_t state_right = LL_GPIO_IsInputPinSet (GPIOC, LL_GPIO_PIN_3);

	// current state of left button
	const uint32_t state_left = LL_GPIO_IsInputPinSet (GPIOA, LL_GPIO_PIN_0);

	volatile int res = 123;

	// check falling edge on right button
	if (state_left == 0 && button_left_previous_state == 1) {
		button_left_previous_state = 0;

		switch (left) {
		case BUTTON_SEND_BEACON:
#ifdef SX1262_IMPLEMENTATION
			res = fanet_test ();
#else
			beacon_send_own (0, 0);
#endif
			break;
		case BUTTON_RECONNECT_APRSIS: aprsis_disconnect (); break;
		case BUTTON_RESET_GSM_GPRS: gsm_sim800_reset (&main_gsm_state); break;
		case BUTTON_FUNCTION_SIMULATE_APRSIS_TIMEOUT: aprsis_debug_set_simulate_timeout (); break;
		default: break;
		}
	}

	// check falling edge on right button
	if (state_right == 0 && button_right_previous_state == 1) {
		button_right_previous_state = 0;

		switch (right) {
		case BUTTON_SEND_BEACON:
#ifdef SX1262_IMPLEMENTATION
			res = fanet_test ();
#else
			beacon_send_own (0, 0);
#endif
			break;
		case BUTTON_RECONNECT_APRSIS: aprsis_disconnect (); break;
		case BUTTON_RESET_GSM_GPRS: gsm_sim800_reset (&main_gsm_state); break;
		case BUTTON_FUNCTION_SIMULATE_APRSIS_TIMEOUT: aprsis_debug_set_simulate_timeout (); break;
		default: break;
		}
	}

	if (res == 0) {
		button_left_previous_state = 0;
	}
}

void button_debounce (void)
{
	button_left_previous_state = 1;

	button_right_previous_state = 1;
}
