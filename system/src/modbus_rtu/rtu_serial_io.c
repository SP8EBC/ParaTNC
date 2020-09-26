/*
 * rtu_serial_io.c
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#include "modbus_rtu/rtu_serial_io.h"
#include "modbus_rtu/rtu_crc.h"
#include "modbus_rtu/rtu_parser.h"
#include "modbus_rtu/rtu_return_values.h"
#include "modbus_rtu/rtu_register_data_t.h"
#include "modbus_rtu/rtu_request.h"

#include "drivers/serial.h"

#include "main.h"
#include "rte_wx.h"

#include "station_config.h"

#include <string.h>

#define INTERFRAME_SP	20

typedef enum rtu_pool_state {
	RTU_POOL_IDLE,
	RTU_POOL_TRANSMITTING,
	RTU_POOL_RECEIVING,
	RTU_POOL_WAIT_AFTER_RECEIVE,
	RTU_POOL_RECEIVE_ERROR,
	RTU_POOL_STOP
} rtu_pool_state_t;

rtu_pool_state_t rtu_pool = RTU_POOL_STOP;

/**
 * Timestamp of last received modbus RTU response with good CRC
 */
uint32_t rtu_time_of_last_receive = 0;

/**
 * CRC value after the last call to rtu_serial_callback
 */
uint16_t rtu_serial_previous_crc = 0xFFFF;

/**
 * Cleared by 'rtu_serial_callback' when first byte from range 0x1..0xF7 is received
 */
uint8_t rtu_waiting_for_slave_addr = 0x1;



/**
 * The callback for stream CRC calculation
 */
uint8_t rtu_serial_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	uint8_t retval = 0;

	uint16_t new_crc = 0;

	// check if the callback still waits for first 'valid' byte to be received from RTU slave
	if (rtu_waiting_for_slave_addr == 0x1) {

		// check if the byte which was received from the slave is valid address
		if (current_data >= 0x01 && current_data <= 0xF7) {
			// clear this flag to start CRC calculation and data receiving
			rtu_waiting_for_slave_addr = 0;
		}
		else {
			// RTU slave cannot respond with the broadcast address (0x00), also 0xF8..0xFF
			// are not valid RTU address
			;
		}
	}

	// the second 'if' clause checks if the slave response has began
	if (rtu_waiting_for_slave_addr == 0x0) {

		// calculate new crc
		new_crc = rtu_crc_stream(rtu_serial_previous_crc, current_data);

		// if the new CRC value equals 0x0000 it means that this was MSB
		// of CRC from correctly received Modbus-RTU frame
		if (new_crc == 0) {
			// return '1' to terminate the transmission
			retval = 1;
		}
		else {
			rtu_serial_previous_crc = new_crc;
		}
	}

	return retval;
}

int32_t rtu_serial_init(rtu_pool_queue_t* queue) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	// zeroing the content of the structure
	memset(queue, 0x00, sizeof(rtu_pool_queue_t));


#ifdef _MODBUS_RTU

#ifdef _RTU_SLAVE_ID_1
	queue->function_id[0] =_RTU_SLAVE_FUNC_1;
	queue->function_parameter[0] = &rte_wx_modbus_rtu_f1;

	rte_wx_modbus_rtu_f1.slave_address = _RTU_SLAVE_ID_1;
	rte_wx_modbus_rtu_f1.base_address = _RTU_SLAVE_ADDR_1;
	rte_wx_modbus_rtu_f1.number_of_registers = 1;
#endif

#ifdef _RTU_SLAVE_ID_2
	queue->function_id[1] =_RTU_SLAVE_FUNC_2;
	queue->function_parameter[1] = &rte_wx_modbus_rtu_f2;

	rte_wx_modbus_rtu_f2.slave_address = _RTU_SLAVE_ID_2;
	rte_wx_modbus_rtu_f2.base_address = _RTU_SLAVE_ADDR_2;
	rte_wx_modbus_rtu_f2.number_of_registers = 1;
#endif

#ifdef _RTU_SLAVE_ID_3
	queue->function_id[2] =_RTU_SLAVE_FUNC_3;
	queue->function_parameter[2] = &rte_wx_modbus_rtu_f3;

	rte_wx_modbus_rtu_f3.slave_address = _RTU_SLAVE_ID_3;
	rte_wx_modbus_rtu_f3.base_address = _RTU_SLAVE_ADDR_3;
	rte_wx_modbus_rtu_f3.number_of_registers = 1;
#endif

#ifdef _RTU_SLAVE_ID_4
	queue->function_id[3] =_RTU_SLAVE_FUNC_4;
	queue->function_parameter[3] = &rte_wx_modbus_rtu_f4;

	rte_wx_modbus_rtu_f4.slave_address = _RTU_SLAVE_ID_4;
	rte_wx_modbus_rtu_f4.base_address = _RTU_SLAVE_ADDR_4;
	rte_wx_modbus_rtu_f4.number_of_registers = 1;
#endif

#endif

	return retval;

}

int32_t rtu_serial_pool(rtu_pool_queue_t* queue, srl_context_t* serial_context) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	int32_t result = MODBUS_RET_UNINITIALIZED;

	uint8_t output_data_lenght = 0;

	if (queue->it >= RTU_POOL_QUEUE_LENGHT) {
		queue->it = 0;

		// all queued modbus function has been queued
		rtu_pool = RTU_POOL_STOP;
	}

	switch (rtu_pool) {
		case RTU_POOL_IDLE: {

			// Enabling the timeout for Modbus-RTU.
			// This timeout starts after first received byte and triggers if
			// the slave will hang up and stop the transmission before the end of the frame
			// It doesn't need to be called each time but this is the only function which takes
			// the pointer to serial context
			srl_switch_timeout(serial_context, 1, 0);

			// check the function it at current queue position
			if (queue->function_id[queue->it] == 0x03) {
				// read holding registers

				// generate request content
				result = rtu_request_03_04_registers(
						1,
						serial_context->srl_tx_buf_pointer,
						serial_context->srl_tx_buf_ln,
						&output_data_lenght,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->slave_address,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->base_address,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->number_of_registers);
			}
			else if (queue->function_id[queue->it] == 0x04) {
				// read input registers

				// generate request content
				result = rtu_request_03_04_registers(
						0,
						serial_context->srl_tx_buf_pointer,
						serial_context->srl_tx_buf_ln,
						&output_data_lenght,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->slave_address,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->base_address,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->number_of_registers);
			}
			else {
				// any other unsupported or wrong function id. It will also stop at the last element
				// on the last element of the queue
				rtu_pool = RTU_POOL_STOP;

				retval = MODBUS_RET_WRONG_FUNCTION;
			}


			// if request has been generated correctly
			if (result == MODBUS_RET_OK) {
				// trigger the transmission itself
				result = srl_start_tx(serial_context, output_data_lenght);

				rtu_serial_previous_crc = 0xFFFF;

				rtu_waiting_for_slave_addr = 1;

				// if serial transmission has been starter
				if (result == SRL_OK) {
					// proceed to the next state (transmitting)
					rtu_pool = RTU_POOL_TRANSMITTING;

					retval = MODBUS_RET_OK;
				}
				else {
					// if not do nothing and try in next pooler call
					;
				}
			}
			else {
				retval = MODBUS_RET_REQUEST_GEN_ERR;
			}

			break;
		}
		case RTU_POOL_TRANSMITTING: {

			// if transmission is still pending
			if (serial_context->srl_tx_state == SRL_TXING || serial_context->srl_tx_state == SRL_TX_WAITING) {
				// wait until it will finish
				;
			}
			else {
				// trigger reception
				srl_receive_data_with_callback(serial_context, rtu_serial_callback);
				//srl_receive_data(serial_context, 8, 0, 0, 0, 0, 0);

				// enable the timeout in case the RTU slave won't respond
				// at all or there is no slaves connected to RS485 bus
				srl_switch_timeout_for_waiting(serial_context, 1);

				// switch the state
				rtu_pool = RTU_POOL_RECEIVING;

			}

			retval = MODBUS_RET_OK;

			break;
		}
		case RTU_POOL_RECEIVING: {

			// if data reception still took place
			if (serial_context->srl_rx_state == SRL_WAITING_TO_RX || serial_context->srl_rx_state == SRL_RXING || serial_context->srl_rx_state == SRL_RX_IDLE) {
				// wait
				;
			}
			else if (serial_context->srl_rx_state == SRL_RX_DONE) {
				// parse the response from RTU slave
				if (queue->function_id[queue->it] == 0x03 || queue->function_id[queue->it] == 0x04) {
					result = rtu_parser_03_04_registers(
							serial_context->srl_rx_buf_pointer,
							serial_context->srl_rx_bytes_counter,
							((rtu_register_data_t*)queue->function_parameter[queue->it]));
				}
				else {
					retval = MODBUS_RET_WRONG_FUNCTION;
				}

				// check parsing result
				if (result == MODBUS_RET_OK) {
					// store the current time
					queue->last_call_to_function[queue->it] = main_get_master_time();

					// switch the state to inter-frame silence period
					rtu_pool = RTU_POOL_WAIT_AFTER_RECEIVE;
				}
				else {
					// Receive error state will switch to the next function
					rtu_pool = RTU_POOL_RECEIVE_ERROR;
				}

				// get current time to start the inter-frame delay
				rtu_time_of_last_receive = main_get_master_time();

			}

			// in case of any error during data reception or the serial driver have fallen into unknown & unexpected
			// state
			else {
				rtu_pool = RTU_POOL_RECEIVE_ERROR;
			}
			break;
		}
		case RTU_POOL_WAIT_AFTER_RECEIVE: {

			// check if required interframe silence period elapsed
			if (main_get_master_time() - rtu_time_of_last_receive > INTERFRAME_SP) {
				rtu_pool = RTU_POOL_IDLE;

				queue->it++;
			}
			break;
		}
		case RTU_POOL_RECEIVE_ERROR: {
			// if the response from the slave was corrupted or any other serial I/O error
			// occured switch to the next function in the queue
			rtu_pool = RTU_POOL_IDLE;

			// move to the next function queued
			queue->it++;

			retval = MODBUS_RET_OK;

			break;
		}
		case RTU_POOL_STOP: {
			break;
		}
		default: {
			rtu_pool = RTU_POOL_STOP;
			break;
		}
	}

	return retval;
}

int32_t rtu_serial_start(void) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_pool = RTU_POOL_IDLE;

	return retval;
}

