/*
 * davis_vantage.c
 *
 *  Created on: 06.08.2020
 *      Author: mateusz
 */

#include "./drivers/davis_vantage.h"

srl_context_t* davis_serial_context;

typedef enum davis_loop_query_state {

};

uint32_t davis_init(srl_context_t* srl_port) {

	uint32_t retval = DAVIS_OK;

	return retval;
}

uint32_t davis_wake_up(void) {

	uint32_t retval = DAVIS_OK;

	return retval;
}
uint32_t davis_do_test(void) {

	uint32_t retval = DAVIS_OK;

	return retval;
}

uint32_t davis_query_for_loop_packet(void) {

	uint32_t retval = DAVIS_OK;

	return retval;
}

uint32_t davis_leave_receiving_screen(void) {

	uint32_t retval = DAVIS_OK;

	return retval;
}

uint32_t davis_control_backlight(uint8_t state) {

	uint32_t retval = DAVIS_OK;

	return retval;
}

uint32_t davis_get_temperature(int32_t* output) {

	uint32_t retval = DAVIS_OK;

	return retval;
}

uint32_t davis_get_pressure(uint32_t* output) {

	uint32_t retval = DAVIS_OK;

	return retval;
}

uint32_t davis_get_wind(uint16_t* speed, uint16_t* gusts, uint16_t* direction) {

	uint32_t retval = DAVIS_OK;

	return retval;
}
