/*
 * davis_vantage.c
 *
 *  Created on: 06.08.2020
 *      Author: mateusz
 */

#include <davis_vantage/davis.h>
#include <davis_vantage/davis_parsers.h>
#include <davis_vantage/davis_query_state_t.h>
#include <davis_vantage/davis_global_state_t.h>
#include <davis_vantage/davis_qf_t.h>

#include <string.h>

#include "main.h"

#define DAVIS_ACK 0x06

#define LOOP_PACKET_LN 				100
#define RX_CHECK_LN_WITH_ACK		32
#define RX_CHECK_MIN_LN_WITH_ACK	16

/**
 * Serial port context to be used for communication
 */
srl_context_t* davis_serial_context;

/**
 * The internal state during loop packet retrieval
 */
davis_query_state_t davis_loop_state;

/**
 * Internal state during waking up the station base
 */
davis_query_state_t davis_wake_up_state;

/**
 * Internal state during RXCHECK query
 */
davis_query_state_t davis_rx_check_state;

/**
 * Global state of Davis Vantage communication
 */
dallas_global_state_t davis_global_state;

/**
 *
 */
davis_qf_t davis_quality_factor;

/**
 * Set to one if station was detected during bootup
 */
uint8_t davis_avaliable;

uint16_t davis_base_total_packet_received = 0;

uint16_t davis_base_total_packet_missed = 0;

uint16_t davis_base_resynchronizations = 0;

uint16_t davis_base_packets_in_the_row = 0;

uint16_t davis_base_crc_errors = 0;

uint32_t davis_last_good_loop = 0;

uint32_t davis_last_good_rxcheck = 0;

static const char line_feed = '\n';
static const char line_feed_return[] = {'\n', '\r'};
static const char loop_command[] = "LOOP 1\n";
static const char lamps_off[] = "LAMPS 0\n";
static const char lamps_on[] = "LAMPS 1\n";
static const char leave_rx_screen[] = "RXTEST\n";	// it also resets
static const char rx_check[] = "RXCHECK\n";

uint32_t davis_init(srl_context_t* srl_port) {

	uint32_t retval = DAVIS_OK;

	davis_serial_context = srl_port;

	davis_quality_factor = DAVIS_QF_UNINITIALIZED;

	davis_loop_state = DAVIS_QUERY_IDLE;

	davis_wake_up_state = DAVIS_QUERY_IDLE;

	davis_rx_check_state = DAVIS_QUERY_IDLE;

	davis_global_state = DAVIS_GLOBAL_IDLE_OR_BLOCKING;

	davis_avaliable = 0;

	// set the timeout according to davis vantage documentation
	srl_switch_timeout(srl_port, 1, 1200);

	return retval;
}

/**
 * This function may have blocking I/O
 */
uint32_t davis_wake_up(uint8_t is_io_blocking) {

	uint32_t retval = DAVIS_OK;

	int comparation_result = -1;

	if (davis_global_state == DAVIS_GLOBAL_IDLE_OR_BLOCKING || davis_global_state == DAVIS_GLOBAL_WAKE_UP) {

		// switch the internal state
		davis_global_state = DAVIS_GLOBAL_WAKE_UP;

		// sending the new line to wake up the console
		srl_send_data(davis_serial_context, (uint8_t*)&line_feed, 1, 1, 0);

		// check if a user want to have blocking I/O
		if (is_io_blocking == 1) {
			// if yes wait for transmission completion and then wait for response
			srl_wait_for_tx_completion(davis_serial_context);

			// start waiting for console response
			srl_receive_data_with_instant_timeout(davis_serial_context, 2, 0, 0, 0, 0, 0);

			// wait for station response or for timeout
			srl_wait_for_rx_completion_or_timeout(davis_serial_context, ((uint8_t*)&retval));

			// if response has been received
			if (retval == SRL_OK) {
				//
				retval = DAVIS_OK;

				// set the quality factor appropriate to signalize that the station
				// is avaliable
				davis_quality_factor = DAVIS_QF_FULL;

				davis_avaliable = 1;
			}
			else {
				// send the wake up command one more time

				// sending the new line to wake up the console
				srl_send_data(davis_serial_context, (uint8_t*)&line_feed, 1, 1, 0);

				// if yes wait for transmission completion and then wait for response
				srl_wait_for_tx_completion(davis_serial_context);

				// start waiting for console response
				srl_receive_data(davis_serial_context, 2, 0, 0, 0, 0, 0);

				// wait for station response or for timeout
				srl_wait_for_rx_completion_or_timeout(davis_serial_context, ((uint8_t*)&retval));

				// if second attempt was successful
				if (retval == SRL_OK) {
					retval = DAVIS_OK;

					// set the quality factor appropriate to signalize that the station
					// is avaliable
					davis_quality_factor = DAVIS_QF_FULL;

					davis_avaliable = 1;
				}
				else {
					// if not the station is dead an
					davis_quality_factor = DAVIS_QF_NOT_AVALIABLE;

					retval = DAVIS_NOT_AVALIABLE;
				}

			}

			// switch the internal state to release the station
			davis_global_state = DAVIS_GLOBAL_IDLE_OR_BLOCKING;

		}
		else {
			// non blocking input/output
			switch (davis_wake_up_state) {
				case DAVIS_QUERY_IDLE: {
					// sending the new line to wake up the console
					srl_send_data(davis_serial_context, (uint8_t*)&line_feed, 1, 1, 0);

					// switching the internal state
					davis_wake_up_state = DAVIS_QUERY_SENDING_QUERY;

					// switch the internal state to block another poolers
					davis_global_state =  DAVIS_GLOBAL_WAKE_UP;

					break;
				}
				case DAVIS_QUERY_SENDING_QUERY: {
					// check if wakeup has been sent

					if (davis_serial_context->srl_tx_state == SRL_TX_IDLE) {
						// if transmission was successful trigger the reception
						srl_receive_data(davis_serial_context, 2, 0, 0, 0, 0, 0);

						// switching the internal state
						davis_wake_up_state = DAVIS_QUERY_RECEIVING;
					}

					if (davis_serial_context->srl_tx_state == SRL_TX_ERROR) {
						davis_wake_up_state = DAVIS_QUERY_ERROR;

						davis_global_state =  DAVIS_GLOBAL_IDLE_OR_BLOCKING;
					}

					break;
				}
				case DAVIS_QUERY_RECEIVING: {
					// check if receive has been successfull

					if (davis_serial_context->srl_rx_state == SRL_RX_DONE) {
						// check the content of what was received from the davis
						comparation_result = memcmp(line_feed_return, srl_get_rx_buffer(davis_serial_context), 2);

						if (comparation_result == 0) {
							// if the base of davis wx station responden with '\r\n' it measn that wake up was sucessfull
							davis_wake_up_state = DAVIS_QUERY_OK;
						}
						else {
							davis_wake_up_state = DAVIS_QUERY_ERROR;
						}

						davis_global_state =  DAVIS_GLOBAL_IDLE_OR_BLOCKING;
					}

					if (davis_serial_context->srl_rx_state == SRL_RX_ERROR) {
						davis_wake_up_state = DAVIS_QUERY_ERROR;

						davis_global_state =  DAVIS_GLOBAL_IDLE_OR_BLOCKING;
					}


					break;
				}
				case DAVIS_QUERY_OK: {
					davis_avaliable = 1;
					break;
				}
				case DAVIS_QUERY_ERROR: {
					davis_avaliable = 0;
					break;
				}
			}
		}
	}
	else {
		// if station is currently busy on another transmission return with an error
		retval = DAVIS_BUSY;
	}

	return retval;
}
uint32_t davis_rxcheck_packet_pooler(void) {
	uint32_t retval = DAVIS_OK;

	if (davis_global_state == DAVIS_GLOBAL_IDLE_OR_BLOCKING || davis_global_state == DAVIS_GLOBAL_RXCHECK) {

		switch(davis_rx_check_state) {
			case DAVIS_QUERY_IDLE: {

				// if station isn't avalaible switch to error w/o any further
				// comms
				if (davis_avaliable == 0) {
					davis_loop_state = DAVIS_QUERY_ERROR;

					retval = DAVIS_NOT_AVALIABLE;
				}
				else {
					// send the command
					srl_send_data(davis_serial_context, (uint8_t*)&rx_check, 1, sizeof(rx_check), 0);

					davis_rx_check_state = DAVIS_QUERY_SENDING_QUERY;

					davis_global_state = DAVIS_GLOBAL_RXCHECK;
				}


				break;
			}
			case DAVIS_QUERY_SENDING_QUERY: {

				if (davis_serial_context->srl_tx_state == SRL_TX_IDLE) {
					// if transmission was successful trigger the reception
					srl_receive_data(davis_serial_context, RX_CHECK_LN_WITH_ACK, 0, 0, 0, 0, 0);

					// switching the internal state
					davis_rx_check_state = DAVIS_QUERY_RECEIVING;
				}

				if (davis_serial_context->srl_tx_state == SRL_TX_ERROR) {
					davis_rx_check_state = DAVIS_QUERY_ERROR;
				}

				break;
			}
			case DAVIS_QUERY_RECEIVING_ACK: {

				// this state is illegal as the wx station base sends
				// ACK and command result in one transaction
				davis_rx_check_state = DAVIS_QUERY_ERROR;

				davis_global_state = DAVIS_GLOBAL_IDLE_OR_BLOCKING;

				break;
			}
			case DAVIS_QUERY_RECEIVING: {

				// the result of RXCHECK command has variable lenght depends on counters values
				if (davis_serial_context->srl_rx_state == SRL_RX_DONE || davis_serial_context->srl_rx_state == SRL_RX_ERROR) {
					// so check how many bytes has been received
					if (davis_serial_context->srl_rx_bytes_counter > RX_CHECK_MIN_LN_WITH_ACK) {
						// if the base unit transmitted more bytes than minimal length of RXCHECK is
						// we might assume that the content is somewhat correct

						// parse the response
						retval = davis_parsers_rxcheck(
								srl_get_rx_buffer(davis_serial_context),
								davis_serial_context->srl_rx_bytes_counter,
								&davis_base_total_packet_received,
								&davis_base_total_packet_missed,
								&davis_base_resynchronizations,
								&davis_base_packets_in_the_row,
								&davis_base_crc_errors);

						if (retval == DAVIS_PARSERS_OK) {
							davis_rx_check_state = DAVIS_QUERY_OK;

							davis_last_good_rxcheck = main_get_master_time();
						}
						else
							davis_rx_check_state = DAVIS_QUERY_ERROR;
					}
					else {
						;
					}

					davis_global_state = DAVIS_GLOBAL_IDLE_OR_BLOCKING;
				}

				break;
			}
			case DAVIS_QUERY_OK: {
				break;
			}
			case DAVIS_QUERY_ERROR: {
				break;
			}
		}
	}
	else {
		retval = DAVIS_BUSY;
	}

	return retval;
}

uint32_t davis_loop_packet_pooler(uint8_t* loop_avaliable_flag) {

	uint32_t retval = DAVIS_OK;

	if (davis_global_state == DAVIS_GLOBAL_IDLE_OR_BLOCKING || davis_global_state == DAVIS_GLOBAL_LOOP) {

		switch (davis_loop_state) {
			case DAVIS_QUERY_IDLE: {

				// if station isn't avalaible switch to error w/o any further
				// comms
				if (davis_avaliable == 0) {
					davis_loop_state = DAVIS_QUERY_ERROR;

					retval = DAVIS_NOT_AVALIABLE;
				}
				else {
					// send the LOOP query
					srl_send_data(davis_serial_context, (uint8_t*)loop_command, 1, 7, 0);

					*loop_avaliable_flag = 0;

					davis_loop_state = DAVIS_QUERY_SENDING_QUERY;

					// set the global state to block another poolers to jam or screw
					// the transmission in another way
					davis_global_state = DAVIS_GLOBAL_LOOP;
				}

				break;
			}
			case DAVIS_QUERY_SENDING_QUERY: {

				// if transmission was successful
				if (davis_serial_context->srl_tx_state == SRL_TX_IDLE) {
					// trigger the reception of ACK message
					srl_receive_data(davis_serial_context, LOOP_PACKET_LN, 0, 0, 0, 0, 0);

					// switching the internal state
					davis_loop_state = DAVIS_QUERY_RECEIVING;
				}

				if (davis_serial_context->srl_tx_state == SRL_TX_ERROR) {
					davis_loop_state = DAVIS_QUERY_ERROR;

					// unlock the global state to free the communication with
					// the station base
					davis_global_state = DAVIS_GLOBAL_IDLE_OR_BLOCKING;
				}

				break;
			}
			case DAVIS_QUERY_RECEIVING: {
				if (davis_serial_context->srl_rx_state == SRL_RX_DONE) {
					// parse the loop packet
					*loop_avaliable_flag = 1;

					davis_last_good_loop = main_get_master_time();

					davis_global_state = DAVIS_GLOBAL_IDLE_OR_BLOCKING;
				}
				else if (davis_serial_context->srl_rx_state == SRL_RX_ERROR) {
					davis_loop_state = DAVIS_QUERY_ERROR;

					// unlock the global state to free the communication with
					// the station base
					davis_global_state = DAVIS_GLOBAL_IDLE_OR_BLOCKING;
				}

				break;
			}
			case DAVIS_QUERY_OK: {

				break;
			}
			case DAVIS_QUERY_ERROR: {
				// clear data availability flag
				*loop_avaliable_flag = 0;
				break;
			}
			default: {
				davis_loop_state = DAVIS_QUERY_ERROR;
			}
		}
	}
	else {
		retval = DAVIS_BUSY;
	}

	return retval;
}

uint32_t davis_trigger_rxcheck_packet(void) {
	davis_rx_check_state = DAVIS_QUERY_IDLE;

	return DAVIS_OK;
}

/** This function will be called from the for(;;) loop in main.c every
 * ten seconds to trigger the pooler to query for next 'LOOP' packet
 *
 */
uint32_t davis_trigger_loop_packet(void) {

	uint32_t retval = DAVIS_OK;

	davis_loop_state = DAVIS_QUERY_IDLE;

	return retval;
}

/**
 * This function will send 'RXTEST' command which resets the counters regarding
 * communication with Outdoor ODU and also leaves the 'Receiving...' if this is a
 * state the weather station is when the function is called. This function has blocking
 * I/O
 */
uint32_t davis_leave_receiving_screen(void) {

	uint32_t retval = DAVIS_OK;

	uint8_t transmission_result;

	transmission_result = srl_send_data(davis_serial_context, (uint8_t*)&leave_rx_screen, 1, 1, 0);

	srl_wait_for_tx_completion(davis_serial_context);

	// wait for the response from the wx console only to clear the input buffer
	transmission_result = srl_receive_data_with_instant_timeout(davis_serial_context, 2, 0, 0, 0, 0, 0);

	if (transmission_result == SRL_OK ) {
		srl_wait_for_rx_completion_or_timeout(davis_serial_context, ((uint8_t*)&retval));

	}

	return retval;
}

/** This function sends the "LAMPS" commands to the station base unit
 * which controls the backliht of main LCD screen. Warning! this call
 * has blocking I/O and waits until the transmission through RS232 port
 * finish
 *
 *
 */
uint32_t davis_control_backlight(uint8_t state) {

	uint32_t retval = DAVIS_OK;

	uint8_t serial_result = SRL_UNINITIALIZED;

	// switch over desired backlight state
	if (state == 0) {
		serial_result = srl_send_data(davis_serial_context, (uint8_t*)lamps_off, 1, strlen(lamps_off), 0);
	}
	else if (state == 1) {
		serial_result = srl_send_data(davis_serial_context, (uint8_t*)lamps_on, 1, strlen(lamps_on), 0);

	}
	else {
		retval = DAVIS_WRONG_PARAMETER;
	}

	// check the transmission request result
	if (serial_result == SRL_OK) {
		// if it was OK wait until the transmission will finish
		srl_wait_for_tx_completion(davis_serial_context);
	}
	else {
		retval = DAVIS_GEN_FAIL;
	}

	return retval;
}

uint32_t davis_get_temperature(davis_loop_t* input, float* output) {

	// The value is sent as 10 th of a degree in F. For example, 795 is
	// returned for 79.5°F.

	uint32_t retval = DAVIS_OK;

	float out = 0.0f;

	out = (float)input->outside_temperature / 10.0f;

	out = 0.56f * (out - 32.0f);

	*output = out;

	return retval;
}

uint32_t davis_get_pressure(davis_loop_t* input, float* output) {

	// Current Barometer. Units are (in Hg / 1000). The barometric
	// value should be between 20 inches and 32.5 inches in Vantage
	// Pro and between 20 inches and 32.5 inches in both Vantatge Pro
	// Vantage Pro2. Values outside these ranges will not be logged.

	uint32_t retval = DAVIS_OK;

	float out = 0.0f;

	out = (float)input->barometer * 1000.0f;

	out *= 33.86f;

	*output = out;

	return retval;
}

uint32_t davis_get_wind(davis_loop_t* input, uint16_t* speed, uint16_t* gusts, uint16_t* direction) {

	uint32_t retval = DAVIS_OK;

	// It is a two byte unsigned value from 1 to 360 degrees. (0° is no
	// wind data, 90° is East, 180° is South, 270° is West and 360° is
	// north)
	*direction = input->wind_direction - 1;

	// It is a byte unsigned value in mph. If the wind speed is dashed
	// because it lost synchronization with the radio or due to some
	// other reason, the wind speed is forced to be 0.

	// this will give a value in 0.01 m/s incremenets
	*speed = ((input->wind_speed) * 45);

	// this will truncate the precision to .1 of m/s
	*speed = (uint16_t)((uint16_t)(*speed) / 10u);

	if (input->wind_direction == 0 || input->wind_direction > 360)
		retval = DAVIS_NOT_AVALIABLE;

	return retval;
}
