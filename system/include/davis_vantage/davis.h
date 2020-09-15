/*
 * davis_vantage.h
 *
 *  Created on: 06.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_DAVIS_VANTAGE_H_
#define INCLUDE_DRIVERS_DAVIS_VANTAGE_H_

#include "drivers/serial.h"

#include <davis_vantage/davis_retval_def.h>
#include <davis_vantage/davis_qf_t.h>
#include <davis_vantage/davis_loop_t.h>


#define DAVIS_DEFAULT_BAUDRATE 19200u

#define DAVIS_BLOCKING_IO	1

extern davis_qf_t davis_quality_factor;
extern uint8_t davis_avaliable;

uint32_t davis_init(srl_context_t* srl_port);
uint32_t davis_wake_up(uint8_t is_io_blocking);
uint32_t davis_rxcheck_packet_pooler(void);
uint32_t davis_loop_packet_pooler(uint8_t* loop_avaliable_flag);
uint32_t davis_trigger_rxcheck_packet(void);
uint32_t davis_trigger_loop_packet(void);
uint32_t davis_leave_receiving_screen(void);
uint32_t davis_control_backlight(uint8_t state);

uint32_t davis_get_temperature(davis_loop_t* input, float* output);
uint32_t davis_get_pressure(davis_loop_t* input, float* output);
uint32_t davis_get_wind(davis_loop_t* input, uint16_t* speed, uint16_t* gusts, uint16_t* direction);


#endif /* INCLUDE_DRIVERS_DAVIS_VANTAGE_H_ */
