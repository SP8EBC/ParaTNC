/*
 * davis_vantage.c
 *
 *  Created on: 06.08.2020
 *      Author: mateusz
 */

#include "./drivers/davis_vantage.h"

/**
 * This enum is private and used only internally by
 */
typedef enum davis_query_state {
	DAVIS_QUERY_IDLE,
	DAVIS_QUERY_SENDING_QUERY,
	DAVIS_QUERY_RECEIVING,
	DAVIS_QUERY_OK,
	DAVIS_QUERY_ERROR
}davis_query_state_t;

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
 *
 */
davis_qf_t davis_quality_factor;

/**
 * Set to one if station was detected during bootup
 */
uint8_t davis_avaliable;

static const char line_feed[] = "\n";
static const char line_feed_return[] = {'\n', '\r'};

uint32_t davis_init(srl_context_t* srl_port) {

	uint32_t retval = DAVIS_OK;

	davis_serial_context = srl_port;

	davis_quality_factor = DAVIS_QF_UNINITIALIZED;

	davis_loop_state = DAVIS_QUERY_IDLE;

	davis_wake_up_state = DAVIS_QUERY_IDLE;

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

	// sending the new line to wake up the console
	srl_send_data(davis_serial_context, (uint8_t*)line_feed, 1, 1, 0);

	// check if a user want to have blocking I/O
	if (is_io_blocking == 1) {
		// if yes wait for transmission completion and then wait for response
		srl_wait_for_tx_completion(davis_serial_context);

		// start waiting for console response
		srl_receive_data(davis_serial_context, 2, 0, 0, 0, 0, 0);

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
			srl_send_data(davis_serial_context, (uint8_t*)line_feed, 1, 1, 0);

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
			}
		}

	}
	else {
		// non blocking input/output
		switch (davis_wake_up_state) {
			case DAVIS_QUERY_IDLE: {
				// sending the new line to wake up the console
				srl_send_data(davis_serial_context, (uint8_t*)line_feed, 1, 1, 0);

				// switching the internal state
				davis_wake_up_state = DAVIS_QUERY_SENDING_QUERY;

				break;
			}
			case DAVIS_QUERY_SENDING_QUERY: {
				// check if wakeup has been sent

				if (davis_serial_context->srl_tx_state == SRL_TX_IDLE) {
					// if transmission was successfull trigger the reception
					srl_receive_data(davis_serial_context, 2, 0, 0, 0, 0, 0);

					// switching the internal state
					davis_wake_up_state = DAVIS_QUERY_RECEIVING;
				}

				if (davis_serial_context->srl_tx_state == SRL_TX_ERROR) {
					davis_wake_up_state = DAVIS_QUERY_ERROR;
				}

				break;
			}
			case DAVIS_QUERY_RECEIVING: {
				// check if receive has been successfull

				if (davis_serial_context->srl_rx_state == SRL_RX_DONE) {
					// check the content of what was received from the davis
					comparation_result = memcmp(line_feed_return, davis_serial_context->srl_rx_buf_pointer, 2);

					if (comparation_result == 0) {
						// if the
					}
					else {

					}
				}

				if (davis_serial_context->srl_rx_state == SRL_RX_ERROR) {
					davis_wake_up_state = DAVIS_QUERY_ERROR;
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
