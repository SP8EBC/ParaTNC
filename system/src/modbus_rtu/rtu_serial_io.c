/*
 * rtu_serial_io.c
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#include "modbus_rtu/rtu_serial_io.h"
#include "modbus_rtu/rtu_crc.h"
#include "modbus_rtu/rtu_return_values.h"
#include "modbus_rtu/rtu_register_data_t.h"
#include "modbus_rtu/rtu_request.h"

#include "drivers/serial.h"
#include "main.h"

typedef enum rtu_pool_state {
	RTU_POOL_IDLE,
	RTU_POOL_TRANSMITTING,
	RTU_POOL_RECEIVING,
	RTE_POOL_WAIT_AFTER_RECEIVE,
	RTU_POOL_RECEIVE_ERROR,
	RTU_POOL_STOP
} rtu_pool_state_t;

rtu_pool_state_t rtu_pool = RTU_POOL_STOP;

uint32_t rtu_time_of_last_receive = 0;

/**
 * CRC value after the last call to rtu_serial_callback
 */
uint16_t rtu_serial_previous_crc = 0;

/**
 * A callback for stream CRC calculation
 */
uint8_t rtu_serial_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	uint8_t retval = 0;

	uint16_t new_crc = 0;

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

	return retval;
}

int32_t rtu_serial_pool(rtu_pool_queue_t* queue, srl_context_t* serial_context) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	int32_t result = MODBUS_RET_UNINITIALIZED;

	if (queue->it >= RTU_POOL_QUEUE_LENGHT) {
		queue->it = 0;
	}

	switch (rtu_pool) {
		case RTU_POOL_IDLE: {

			// check the function it at current queue position
			if (queue->function_id[queue->it] == 0x03) {
				// read holding registers

				// generating request content
				result = rtu_request_03_04_registers(
						0,
						serial_context->srl_tx_buf_pointer,
						serial_context->srl_tx_buf_ln,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->slave_address,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->base_address,
						((rtu_register_data_t*)queue->function_parameter[queue->it])->number_of_registers);

				// if request has been generated correctly
				if (result == MODBUS_RET_OK) {
					;
				}
				else {
					;
				}
			}
			else if (queue->function_id[queue->it] == 0x04) {
				// read input registers
				;
			}
			else {
				// any other unsupported or wrong function id
				rtu_pool = RTU_POOL_STOP;
			}

			break;
		}
		case RTU_POOL_TRANSMITTING: {
			break;
		}
		case RTU_POOL_RECEIVING: {
			break;
		}
		case RTE_POOL_WAIT_AFTER_RECEIVE: {
			break;
		}
		case RTU_POOL_RECEIVE_ERROR: {
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

