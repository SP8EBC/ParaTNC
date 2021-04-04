/*
 * rtu_serial_io.c
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#include "modbus_rtu/rtu_configuration.h"
#include "modbus_rtu/rtu_serial_io.h"
#include "modbus_rtu/rtu_crc.h"
#include "modbus_rtu/rtu_parser.h"
#include "modbus_rtu/rtu_return_values.h"
#include "modbus_rtu/rtu_register_data_t.h"
#include "modbus_rtu/rtu_request.h"

#include "drivers/serial.h"

#include "main.h"
#include "rte_wx.h"
#include "rte_main.h"
#include "rte_rtu.h"

#include "station_config.h"

#include <string.h>
#include <stdio.h>

#define INTERFRAME_SP	20


#ifndef _RTU_SLAVE_LENGHT_1
	#define _RTU_SLAVE_LENGHT_1 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_2
	#define _RTU_SLAVE_LENGHT_2 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_3
	#define _RTU_SLAVE_LENGHT_3 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_4
	#define _RTU_SLAVE_LENGHT_4 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_5
	#define _RTU_SLAVE_LENGHT_5 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_6
	#define _RTU_SLAVE_LENGHT_6 0x1
#endif

typedef enum rtu_pool_state {
	RTU_POOL_IDLE,
	RTU_POOL_TRANSMITTING,
	RTU_POOL_RECEIVING,
	RTU_POOL_WAIT_AFTER_RECEIVE,
	RTU_POOL_RECEIVE_ERROR,
	RTU_POOL_STOP
} rtu_pool_state_t;

rtu_pool_state_t rtu_pool_state = RTU_POOL_STOP;

/**
 * Set to one to switch I/O operations to blocking mode
 */
uint8_t rtu_blocking_io = 0;

/**
 * Timestamp of last received modbus RTU response with good CRC
 */
uint32_t rtu_time_of_last_successfull_comm = 0;

/**
 * This variable latches the value of 'rtu_time_of_last_successfull_comm' across
 * consecutive messages with an error status. If the value is the same as during
 * previous transmission the controller is restarted
 */
uint32_t rtu_time_of_last_succ_comm_at_previous_error_status = 0;

/**
 * CRC value after the last call to rtu_serial_callback
 */
uint16_t rtu_serial_previous_crc = 0xFFFF;

/**
 * Cleared by 'rtu_serial_callback' when first byte from range 0x1..0xF7 is received
 */
uint8_t rtu_waiting_for_slave_addr = 0x1;

volatile rtu_pool_queue_t* rtu_used_queue;

volatile srl_context_t* rtu_used_serial_context;

volatile uint8_t rtu_current_03_slave_address;

volatile uint16_t rtu_current_03_base_register;

volatile uint8_t rtu_current_03_number_of_registers;

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

int32_t rtu_serial_init(rtu_pool_queue_t* queue, uint8_t io_mode, srl_context_t* serial_context) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	// zeroing the content of the structure
	memset(queue, 0x00, sizeof(rtu_pool_queue_t));

	rtu_blocking_io = io_mode;

	rtu_used_queue = queue;

	rtu_used_serial_context = serial_context;

#ifdef _MODBUS_RTU

#ifdef _RTU_SLAVE_ID_1
	queue->function_id[0] =_RTU_SLAVE_FUNC_1;
	queue->function_parameter[0] = &rte_wx_modbus_rtu_f1;

	rte_wx_modbus_rtu_f1.slave_address = _RTU_SLAVE_ID_1;
	rte_wx_modbus_rtu_f1.base_address = _RTU_SLAVE_ADDR_1;
	rte_wx_modbus_rtu_f1.number_of_registers = _RTU_SLAVE_LENGHT_1;
#endif

#ifdef _RTU_SLAVE_ID_2
	queue->function_id[1] =_RTU_SLAVE_FUNC_2;
	queue->function_parameter[1] = &rte_wx_modbus_rtu_f2;

	rte_wx_modbus_rtu_f2.slave_address = _RTU_SLAVE_ID_2;
	rte_wx_modbus_rtu_f2.base_address = _RTU_SLAVE_ADDR_2;
	rte_wx_modbus_rtu_f2.number_of_registers = _RTU_SLAVE_LENGHT_2;
#endif

#ifdef _RTU_SLAVE_ID_3
	queue->function_id[2] =_RTU_SLAVE_FUNC_3;
	queue->function_parameter[2] = &rte_wx_modbus_rtu_f3;

	rte_wx_modbus_rtu_f3.slave_address = _RTU_SLAVE_ID_3;
	rte_wx_modbus_rtu_f3.base_address = _RTU_SLAVE_ADDR_3;
	rte_wx_modbus_rtu_f3.number_of_registers = _RTU_SLAVE_LENGHT_3;
#endif

#ifdef _RTU_SLAVE_ID_4
	queue->function_id[3] =_RTU_SLAVE_FUNC_4;
	queue->function_parameter[3] = &rte_wx_modbus_rtu_f4;

	rte_wx_modbus_rtu_f4.slave_address = _RTU_SLAVE_ID_4;
	rte_wx_modbus_rtu_f4.base_address = _RTU_SLAVE_ADDR_4;
	rte_wx_modbus_rtu_f4.number_of_registers = _RTU_SLAVE_LENGHT_4;
#endif

#ifdef _RTU_SLAVE_ID_5
	queue->function_id[4] =_RTU_SLAVE_FUNC_5;
	queue->function_parameter[4] = &rte_wx_modbus_rtu_f5;

	rte_wx_modbus_rtu_f5.slave_address = _RTU_SLAVE_ID_5;
	rte_wx_modbus_rtu_f5.base_address = _RTU_SLAVE_ADDR_5;
	rte_wx_modbus_rtu_f5.number_of_registers = _RTU_SLAVE_LENGHT_5;
#endif

#ifdef _RTU_SLAVE_ID_6
	queue->function_id[5] =_RTU_SLAVE_FUNC_6;
	queue->function_parameter[5] = &rte_wx_modbus_rtu_f6;

	rte_wx_modbus_rtu_f6.slave_address = _RTU_SLAVE_ID_6;
	rte_wx_modbus_rtu_f6.base_address = _RTU_SLAVE_ADDR_6;
	rte_wx_modbus_rtu_f6.number_of_registers = _RTU_SLAVE_LENGHT_6;
#endif

#endif

	return retval;

}

int32_t rtu_serial_pool(void) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;
#ifdef _MODBUS_RTU
	int32_t result = MODBUS_RET_UNINITIALIZED;

	uint8_t output_data_lenght = 0;

	rtu_register_data_t* ptr_func_03;

	// if there were any serial I/O error
	if (rte_rtu_number_of_serial_io_errors > 0) {

		// check how many serial I/O erros have been detected so far
		if ((rte_rtu_number_of_serial_io_errors % RTU_NUMBER_OF_ERRORS_TO_TRIG_STATUS) == 0) {
			// set the status trigger
			rte_main_trigger_modbus_status = 1;

			// increment the error counter artificially to protect sending status in the loop
			rte_rtu_number_of_serial_io_errors++;

			// stupid workaround. If there is a lot of I/O errors reset the controller

			if (rte_rtu_number_of_serial_io_errors >= (0xFF - RTU_NUMBER_OF_ERRORS_TO_TRIG_STATUS))
			{
				rte_main_reboot_req = 1;
			}

			// latch the current value of last successfull communication
			rtu_time_of_last_succ_comm_at_previous_error_status = rtu_time_of_last_successfull_comm;
		}
	}

	if (rtu_used_queue->it >= RTU_POOL_QUEUE_LENGHT) {
		rtu_used_queue->it = 0;

		// all queued modbus functions have been serviced
		rtu_pool_state = RTU_POOL_STOP;
	}

	switch (rtu_pool_state) {
		case RTU_POOL_IDLE: {

			// initialize the serial port.
			srl_init(	rtu_used_serial_context,
					rtu_used_serial_context->port,
					rtu_used_serial_context->srl_rx_buf_pointer,
					rtu_used_serial_context->srl_rx_buf_ln,
					rtu_used_serial_context->srl_tx_buf_pointer,
					rtu_used_serial_context->srl_tx_buf_ln,
					rtu_used_serial_context->port_baurate,
					rtu_used_serial_context->port_stopbits);

			// Enabling the timeout for Modbus-RTU.
			// This timeout starts after first received byte and triggers if
			// the slave will hang up and stop the transmission before the end of the frame
			// It doesn't need to be called each time but this is the only function which takes
			// the pointer to serial context
			srl_switch_timeout(rtu_used_serial_context, 1, 0);

			srl_switch_tx_delay(rtu_used_serial_context, 1);

			// check the function it at current queue position
			if (rtu_used_queue->function_id[rtu_used_queue->it] == 0x03) {
				// read holding registers
				ptr_func_03 = ((rtu_register_data_t*)rtu_used_queue->function_parameter[rtu_used_queue->it]);

				rtu_current_03_slave_address = ptr_func_03->slave_address;
				rtu_current_03_base_register = ptr_func_03->base_address;
				rtu_current_03_number_of_registers = ptr_func_03->number_of_registers;

				// generate request content
				result = rtu_request_03_04_registers(
						1,
						rtu_used_serial_context->srl_tx_buf_pointer,
						rtu_used_serial_context->srl_tx_buf_ln,
						&output_data_lenght,
						rtu_current_03_slave_address,
						rtu_current_03_base_register,
						rtu_current_03_number_of_registers);
			}
			else if (rtu_used_queue->function_id[rtu_used_queue->it] == 0x04) {
				// read input registers

				// generate request content
				result = rtu_request_03_04_registers(
						0,
						rtu_used_serial_context->srl_tx_buf_pointer,
						rtu_used_serial_context->srl_tx_buf_ln,
						&output_data_lenght,
						((rtu_register_data_t*)rtu_used_queue->function_parameter[rtu_used_queue->it])->slave_address,
						((rtu_register_data_t*)rtu_used_queue->function_parameter[rtu_used_queue->it])->base_address,
						((rtu_register_data_t*)rtu_used_queue->function_parameter[rtu_used_queue->it])->number_of_registers);
			}
			else {
				// any other unsupported or wrong function id. It will also stop at the last element
				// on the last element of the queue
				rtu_pool_state = RTU_POOL_STOP;

				// rewind the iterator back to the begining
				rtu_used_queue->it = 0;

				retval = MODBUS_RET_WRONG_FUNCTION;
			}


			// if request has been generated correctly
			if (result == MODBUS_RET_OK) {

				// check if block I/O mode shall be used for the communication
				if (rtu_blocking_io) {
					// call the function which will process the same stuff than
					// RTU_POOL_IDLE (from transmitting), RTU_POOL_TRANSMITTING
					// and partially RTU_POOL_RECEIVING
					result = rtu_serial_blocking_io(rtu_used_serial_context, output_data_lenght);

					if (result == MODBUS_RET_OK) {
						// if transmission and reception was successful switch to the
						// receiving state. The 'serial_context->srl_rx_state' will be
						// alredy set to 'SRL_RX_DONE'
						rtu_pool_state = RTU_POOL_RECEIVING;
					}
					else {
						rtu_pool_state = RTU_POOL_RECEIVE_ERROR;
					}
				}
				else {
					// trigger the transmission itself
					result = srl_start_tx(rtu_used_serial_context, output_data_lenght);

					// reset the CRC value to default
					rtu_serial_previous_crc = 0xFFFF;

					rtu_waiting_for_slave_addr = 1;

					// if serial transmission has been starter
					if (result == SRL_OK) {
						// proceed to the next state (transmitting)
						rtu_pool_state = RTU_POOL_TRANSMITTING;

						retval = MODBUS_RET_OK;
					}
					else {
						// if not do nothing and try in next pooler call
						;
					}
				}


			}
			else {
				retval = MODBUS_RET_REQUEST_GEN_ERR;
			}

			break;
		}
		case RTU_POOL_TRANSMITTING: {

			// if transmission is still pending
			if (rtu_used_serial_context->srl_tx_state == SRL_TXING || rtu_used_serial_context->srl_tx_state == SRL_TX_WAITING) {
				// wait until it will finish
				;
			}
			else {
				// trigger reception
				srl_receive_data_with_callback(rtu_used_serial_context, rtu_serial_callback);

				// enable the timeout in case the RTU slave won't respond
				// at all or there is no slaves connected to RS485 bus
				srl_switch_timeout_for_waiting(rtu_used_serial_context, 1);

				// switch the state
				rtu_pool_state = RTU_POOL_RECEIVING;

			}

			retval = MODBUS_RET_OK;

			break;
		}
		case RTU_POOL_RECEIVING: {

			// if data reception still took place
			if (rtu_used_serial_context->srl_rx_state == SRL_WAITING_TO_RX || rtu_used_serial_context->srl_rx_state == SRL_RXING || rtu_used_serial_context->srl_rx_state == SRL_RX_IDLE) {
				// wait
				;
			}
			else if (rtu_used_serial_context->srl_rx_state == SRL_RX_DONE) {
				// parse the response from RTU slave // here there is a problem with changing slave address
				if (rtu_used_queue->function_id[rtu_used_queue->it] == 0x03 || rtu_used_queue->function_id[rtu_used_queue->it] == 0x04) {
					result = rtu_parser_03_04_registers(
							rtu_used_serial_context->srl_rx_buf_pointer,
							rtu_used_serial_context->srl_rx_bytes_counter,
							((rtu_register_data_t*)rtu_used_queue->function_parameter[rtu_used_queue->it]),
							&rte_rtu_last_modbus_exception);
				}
				else {
					retval = MODBUS_RET_WRONG_FUNCTION;
				}

				// check parsing result
				if (result == MODBUS_RET_OK) {
					// store the current time
					rtu_used_queue->last_successfull_call_to_function[rtu_used_queue->it] = main_get_master_time();

					// switch the state to inter-frame silence period
					rtu_pool_state = RTU_POOL_WAIT_AFTER_RECEIVE;
				}
				else if (result == MODBUS_RET_GOT_EXCEPTION) {
					// in case of an excetpion store the current timestamp
					rte_rtu_last_modbus_exception_timestamp = main_get_master_time();

					// switch the state to inter-frame silence period
					rtu_pool_state = RTU_POOL_WAIT_AFTER_RECEIVE;
				}
				else {
					// Receive error state will switch to the next function
					rtu_pool_state = RTU_POOL_RECEIVE_ERROR;
				}

				// get current time to start the inter-frame delay
				rtu_time_of_last_successfull_comm = main_get_master_time();

				rte_rtu_number_of_successfull_serial_comm++;

				// Close the serial port. This is a part of the stupid workaround of the problem
				// with a serial port which leads to receiving a lot of idle frames of unknown origin
				// and corrupting some part of data at the begining of some Modbus-RTU frames
				srl_close(rtu_used_serial_context);

			}

			// in case of any error during data reception or the serial driver have fallen into unknown & unexpected
			// state
			else {

				rtu_pool_state = RTU_POOL_RECEIVE_ERROR;

				srl_close(rtu_used_serial_context);
			}

			break;
		}
		case RTU_POOL_WAIT_AFTER_RECEIVE: {

			// check if required interframe silence period elapsed
			if (main_get_master_time() - rtu_time_of_last_successfull_comm > INTERFRAME_SP) {
				rtu_pool_state = RTU_POOL_IDLE;

				rtu_used_queue->it++;
			}
			break;
		}
		case RTU_POOL_RECEIVE_ERROR: {
			// if the response from the slave was corrupted or any other serial I/O error
			// occured switch to the next function in the queue
			rtu_pool_state = RTU_POOL_IDLE;

			// increasing the global counter of io errors
			rte_rtu_number_of_serial_io_errors++;

			rte_rtu_last_modbus_rx_error_timestamp = main_get_master_time();

			// icrease the error counter for this queue element
			rtu_used_queue->number_of_errors[rtu_used_queue->it] = rtu_used_queue->number_of_errors[rtu_used_queue->it] + 1;

			// move to the next function queued
			rtu_used_queue->it++;

			retval = MODBUS_RET_OK;

			break;
		}
		case RTU_POOL_STOP: {
			break;
		}
		default: {
			rtu_pool_state = RTU_POOL_STOP;
			break;
		}
	}
#endif
	return retval;
}

int32_t rtu_serial_blocking_io(srl_context_t* serial_context, uint8_t query_ln) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	uint8_t serial_result = SRL_UNINITIALIZED;

	// reset the CRC value to default
	rtu_serial_previous_crc = 0xFFFF;

	rtu_waiting_for_slave_addr = 1;

	// sending data
	serial_result = srl_start_tx(serial_context, query_ln);

	// if tx has been triggered successfully
	if (serial_result == SRL_OK) {

		// wait for transmission to complete
		srl_wait_for_tx_completion(serial_context);

		// trigger reception
		serial_result = srl_receive_data_with_callback(serial_context, rtu_serial_callback);

		if (serial_result == SRL_OK) {

			// enable the timeout in case the RTU slave won't respond
			// at all or there is no slaves connected to RS485 bus
			srl_switch_timeout_for_waiting(serial_context, 1);

			// wait for the slave response
			srl_wait_for_rx_completion_or_timeout(serial_context, &serial_result);

			if (serial_result == SRL_OK) {
				retval = MODBUS_RET_OK;
			}
			else {
				retval = MODBUS_RET_GOT_EXCEPTION;
			}
		}

	}

	return retval;
}

int32_t rtu_serial_start(void) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_pool_state = RTU_POOL_IDLE;

	return retval;
}

int32_t rtu_serial_get_status_string(rtu_pool_queue_t* queue, srl_context_t* srl_ctx, char* out, uint16_t out_buffer_ln, uint8_t* generated_string_ln) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;
	int string_ln = 0;

	memset(out, 0x00, out_buffer_ln);
#ifdef _MODBUS_RTU

	string_ln = snprintf(out, out_buffer_ln, ">MT %X, LRET %X, LSCT %X, NSSC %X, NSE %X, RXB %X, RXI %X, TXB %X",
												main_get_master_time(),
												rte_rtu_last_modbus_rx_error_timestamp,
												rtu_time_of_last_successfull_comm,
												(int)rte_rtu_number_of_successfull_serial_comm,
												(int)rte_rtu_number_of_serial_io_errors,
												srl_ctx->total_rx_bytes,
												srl_ctx->total_idle_counter,
												srl_ctx->total_tx_bytes);

	*generated_string_ln = (uint8_t) string_ln;
#endif
	return retval;
}

