/*
 * davis_vantage.h
 *
 *  Created on: 06.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_DAVIS_VANTAGE_H_
#define INCLUDE_DRIVERS_DAVIS_VANTAGE_H_

#include "drivers/serial.h"

#define DAVIS_OK		0

typedef enum davis_loop_query_state {
	DAVIS_LOOP_IDLE,
	DAVIS_LOOP_SENDING_QUERY,
	DAVIS_LOOP_RECEIVING,
	DAVIS_LOOP_OK,
	DAVIS_LOOP_ERROR
}davis_loop_query_state_t;

uint32_t davis_init(srl_context_t* srl_port);
uint32_t davis_wake_up(void);
uint32_t davis_do_test(void);
uint32_t davis_query_for_loop_packet(void);
uint32_t davis_leave_receiving_screen(void);
uint32_t davis_control_backlight(uint8_t state);

uint32_t davis_get_temperature(int32_t* output);
uint32_t davis_get_pressure(uint32_t* output);
uint32_t davis_get_wind(uint16_t* speed, uint16_t* gusts, uint16_t* direction);


#endif /* INCLUDE_DRIVERS_DAVIS_VANTAGE_H_ */
