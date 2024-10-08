/*
 * kiss_callback.c
 *
 * This file contains callback to all async sudo-UDS services requests. Each
 * callback takes pointer for buffer and response. It returns a size of a
 * response.
 *
 *  Created on: Aug 17, 2022
 *      Author: mateusz
 */

#include <kiss_communication/kiss_communication.h>
#include <kiss_communication/types/kiss_communication_service_ids.h>
#include <kiss_communication/diagnostics_services/kiss_routine_control.h>
#include <kiss_communication/diagnostics_services/kiss_did.h>
#include <kiss_communication/diagnostics_services/kiss_read_memory.h>
#include <kiss_communication/diagnostics_services/kiss_security_access.h>
#include <kiss_communication/kiss_nrc_response.h>
#include "main.h"
#include "rte_main.h"

#include <etc/kiss_configuation.h>

#include <string.h>
#include <stdio.h>
#include <stored_configuration_nvm/configuration_handler.h>
#include <variant.h>

#include "backup_registers.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define KISS_MAX_CONFIG_PAYLOAD_SIZE	0x80
#define KISS_LAST_ASYNC_MSG				0xFF

#define KISS_RESET_SOFTRESET			0x03

#define KISS_CALLBACK_ROUTINE_CONTROL_START_MINIMUM_PAYLOAD_LN	9u

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

uint8_t kiss_async_message_counter = 0;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param input_frame_from_host
 * @param input_len
 * @param response_buffer
 * @param buffer_size
 * @return
 */
int32_t kiss_callback_get_running_config(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media) {

	#define CALLBACK_GET_RUNNING_CFG_LN	7

	uint32_t conf_size = 0;

	configuration_handler_region_t current_region;

	if (transport_media == KISS_TRANSPORT_SERIAL_PORT) {

		// set current message to start pooling
		kiss_current_async_message = KISS_RUNNING_CONFIG;

		// the rest of content of an input frame is irrevelant, but we need to send
		// a response telling how long configuration data is
		memset(response_buffer, 0x00, buffer_size);

		// get currently used configuration and its size in flash memory
		current_region = configuration_get_current(&conf_size);

		// reset async message counter
		kiss_async_message_counter = 0;

		// construct a response
		response_buffer[0] = FEND;
		response_buffer[1] = NONSTANDARD;
		response_buffer[2] = CALLBACK_GET_RUNNING_CFG_LN;				// message lenght
		response_buffer[3] = KISS_RUNNING_CONFIG;
		response_buffer[4] = (uint8_t)(current_region & 0xFF);
		if ((conf_size % KISS_MAX_CONFIG_PAYLOAD_SIZE) == 0)
			response_buffer[5] = (uint8_t)(conf_size / KISS_MAX_CONFIG_PAYLOAD_SIZE);
		else
			response_buffer[5] = (uint8_t)(conf_size / KISS_MAX_CONFIG_PAYLOAD_SIZE + 1);
		response_buffer[6] = FEND;

		return CALLBACK_GET_RUNNING_CFG_LN;
	}
	else {
		return kiss_nrc_response_fill_security_access_denied(response_buffer);
	}

}

/**
 *
 * @param output_buffer
 * @param buffer_size
 * @return
 */
int16_t kiss_pool_callback_get_running_config(uint8_t * output_buffer, uint16_t buffer_size){

	uint32_t conf_size = 0, offset = 0;

	configuration_handler_region_t current_region;

	uint8_t config_payload_size = 0;

	const uint8_t * config_base_address = 0;

	// terminate if no more packets needs to be send
	if (kiss_async_message_counter == KISS_LAST_ASYNC_MSG) {
		return 0;
	}

	// get currently used configuration and its size in flash memory
	current_region = configuration_get_current(&conf_size);

	// get base address of current config
	config_base_address = (const uint8_t *)configuration_get_address(current_region);

	if (kiss_async_message_counter * KISS_MAX_CONFIG_PAYLOAD_SIZE < conf_size) {
		config_payload_size = KISS_MAX_CONFIG_PAYLOAD_SIZE;

		// offset in source configuration area
		offset = kiss_async_message_counter * KISS_MAX_CONFIG_PAYLOAD_SIZE;

		kiss_async_message_counter++;
	}
	else {
		config_payload_size = conf_size - (kiss_async_message_counter * KISS_MAX_CONFIG_PAYLOAD_SIZE);

		// offset in source configuration area
		offset = kiss_async_message_counter * KISS_MAX_CONFIG_PAYLOAD_SIZE;

		kiss_async_message_counter = KISS_LAST_ASYNC_MSG;
	}

	// a case for 'config_size' being a multiply of 'KISS_MAX_CONFIG_PAYLOAD_SIZE'
	if (config_payload_size == 0) {
		return 0;
	}

	// place KISS header
	output_buffer[0] = FEND;
	output_buffer[1] = NONSTANDARD;
	output_buffer[2] = config_payload_size + 7;
	output_buffer[3] = KISS_RUNNING_CONFIG;
	output_buffer[4] = 0xAB;				// THIS IS A DATA FRAME, not ACK
	output_buffer[5] = kiss_async_message_counter - 1;	// frame sequence number

	memcpy(output_buffer + 6, config_base_address + offset, config_payload_size);

	output_buffer[config_payload_size + 6] = FEND;

	return config_payload_size + 7;
}

/**
 *
 * @param input_frame_from_host
 * @param input_len
 * @param response_buffer
 * @param buffer_size
 * @return
 */
int32_t kiss_callback_get_version_id(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media) {

	uint8_t config_payload_size = 0;

#ifdef PARAMETEO
	config_payload_size = snprintf((char *)response_buffer + 4, buffer_size, "METEO-%s-%s", SW_VER, SW_KISS_PROTO);
#else
	config_payload_size = snprintf((char *)response_buffer + 4, buffer_size, "TNC-%s-%s", SW_VER, SW_KISS_PROTO);
#endif

	// construct a response
	response_buffer[0] = FEND;
	response_buffer[1] = NONSTANDARD;
	response_buffer[2] = config_payload_size + 5;				// message lenght
	response_buffer[3] = KISS_VERSION_AND_ID;
	// string here
	response_buffer[config_payload_size + 4] = FEND;

	return config_payload_size + 5;

}

/**
 * Synchronously erases current startup configuration
 * @param input_frame_from_host
 * @param input_len
 * @param response_buffer
 * @param buffer_size
 * @return
 */
int32_t kiss_callback_erase_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media) {

#define ERASE_STARTUP_LN	6

	if (transport_media == KISS_TRANSPORT_SERIAL_PORT) {

		kiss_communication_nrc_t result = configuration_handler_erase_startup();

		response_buffer[0] = FEND;
		response_buffer[1] = NONSTANDARD;
		response_buffer[2] = ERASE_STARTUP_LN;				// message lenght

		if (result == NRC_POSITIVE) {
			// construct a response
			response_buffer[3] = KISS_ERASE_STARTUP_CFG_RESP;
		}
		else {
			response_buffer[3] = KISS_NEGATIVE_RESPONSE_SERVICE;

		}

		response_buffer[4] = (uint8_t)result;
		response_buffer[5] = FEND;

		return ERASE_STARTUP_LN;
	}
	else {
		return kiss_nrc_response_fill_security_access_denied(response_buffer);
	}

}

/**
 * Callback which program configuration data block received from the Host PC. Please bear in mind that the TNC doesn't really take care
 * what it receives and program. It is up to host PC to provide senseful configuration with properly calculated checksum as this isn't
 * recalculated during programming.
 */
int32_t kiss_callback_program_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media) {

#define PROGRAM_STARTUP_LN	6

	/**
	 * The structure of an input frame goes like that:
	 * FEND, KISS_PROGRAM_STARTUP_CFG, data_PAYLOAD_LN, OFFSET_LSB, OFFSET_MSB, data, data, (...), FEND
	 *
	 * KISS_PROGRAM_STARTUP_CFG is a frame type, in this case 0x34, but might be also 0x00 for regular
	 * frame data to be sent over the air. data_PAYLOAD_LN is a lenght of data in this frame, the
	 * assumption is  that whole config data segment size will be an even multiply of a size
	 * of single frame.
	 */

	// result to be returned to the host PC
	kiss_communication_nrc_t result;

	// offset within input frame where config start begining
	uint8_t * data_ptr = input_frame_from_host + 5;

	// size of data to be programmed into flash memory
	uint8_t data_size =  *(input_frame_from_host + 2);

	uint16_t config_block_offset = *(input_frame_from_host + 3) | (*(input_frame_from_host + 4) << 8);
	
	if (transport_media == KISS_TRANSPORT_SERIAL_PORT) {
		result = configuration_handler_program_startup(data_ptr, data_size, config_block_offset);

		// construct a response
		response_buffer[0] = FEND;
		response_buffer[1] = NONSTANDARD;
		response_buffer[2] = PROGRAM_STARTUP_LN;				// message lenght

		if (result == NRC_POSITIVE) {
			// construct a response
			response_buffer[3] = KISS_PROGRAM_STARTUP_CFG_RESP;
		}
		else {
			response_buffer[3] = KISS_NEGATIVE_RESPONSE_SERVICE;

		}

		response_buffer[4] = (uint8_t)result;
		response_buffer[5] = FEND;

		return PROGRAM_STARTUP_LN;
	}
	else {
		return kiss_nrc_response_fill_security_access_denied(response_buffer);
	}
}

/**
 * Reads data by identifier
 * @param input_frame_from_host
 * @param input_len
 * @param response_buffer
 * @param buffer_size
 * @return
 */
int32_t kiss_callback_read_did(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media) {

	/**
	 * Response frame structure
	 *
	 * FEND, NONSTANDARD, RESPONSE_SIZE, KISS_READ_DID_RESP, DID_lsb, DID_msb, size_byte, DATA (...), FEND
	 */

	int32_t out = 0;

	memset(response_buffer, 0x00, buffer_size);

	// identifier
	uint16_t did = *(input_frame_from_host + 2) | (*(input_frame_from_host + 3) << 8);

	// construct DID response to an output buffer
	const uint8_t response_size = kiss_did_response(did, response_buffer + 4, buffer_size - 5);

	// check if DID has been found and everyting is OK with it.
	if (response_size > 0) {
		// if response is correct fill the buffer with the rest of stuff
		response_buffer[0] = FEND;
		response_buffer[1] = NONSTANDARD;
		response_buffer[2] = response_size + 5;				// message lenght
		response_buffer[3] = KISS_READ_DID_RESP;

		response_buffer[response_size + 4] = FEND;

		out = response_size + 5;
	}
	else {
		out = kiss_nrc_response_fill_request_out_of_range(response_buffer);
	}

	return out;
}

/**
 * Reads raw content of memory from given address
 * @param input_frame_from_host
 * @param input_len
 * @param response_buffer
 * @param buffer_size
 * @return
 */
int32_t kiss_callback_read_memory_by_addr(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media) {
	/**
	 * Response frame structure
	 *
	 * FEND, NONSTANDARD, RESPONSE_SIZE, KISS_READ_MEM_RESP, data, data, data, data (...), FEND
	 */

	int32_t out = 0;

	memset(response_buffer, 0x00, buffer_size);

	// size format
	const uint8_t size_format = *(input_frame_from_host + 2);

	// address
	const uint32_t address = *(input_frame_from_host + 3) | (*(input_frame_from_host + 4) << 8) | (*(input_frame_from_host + 5) << 16) | (*(input_frame_from_host + 6) << 24);

	// size
	const uint8_t size = *(input_frame_from_host + 7);

	// if security access must be unlocked
	const uint8_t require_unlock = kiss_security_check_service_req_unlocking(KISS_READ_MEM_ADDR, transport_media, address);

	// allow only one byte of size and four byte of address
	if (size_format == 0x14) {

		if (size > 0 && size < KISS_CONFIG_DIAGNOSTIC_BUFFER_LN - 4) {
			if ((require_unlock == 1 && kiss_security_access_get_unlocked_per_transport (transport_media) == 1) ||
				(variant_validate_is_within_read_mem_by_addr ((const void *)address) == 1)) {
				// construct DID response to an output buffer
				const uint8_t response_size = kiss_read_memory_response(address, size, response_buffer + 4, buffer_size - 5);

				// check if DID has been found and everyting is OK with it.
				if (response_size > 0) {
					// if response is correct fill the buffer with the rest of stuff
					response_buffer[0] = FEND;
					response_buffer[1] = NONSTANDARD;
					response_buffer[2] = response_size + 5;				// message lenght
					response_buffer[3] = KISS_READ_MEM_ADDR_RESP;

					response_buffer[response_size + 4] = FEND;

					out = response_size + 5;
				}
				else {
					out = kiss_nrc_response_fill_request_out_of_range(response_buffer);
				}
			}
			else {
				out = kiss_nrc_response_fill_security_access_denied(response_buffer);
			}
		}
		else
		{
			out = kiss_nrc_response_fill_request_out_of_range(response_buffer);
		}
	}
	else {
		out = kiss_nrc_response_fill_incorrect_message_lenght_or_format(response_buffer);
	}

	return out;
}

/**
 *
 * @param input_frame_from_host
 * @param input_len
 * @param response_buffer
 * @param buffer_size
 * @return
 */
int32_t kiss_callback_reset(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media) {

	int32_t out = 0;

	const uint8_t reset_type = *(input_frame_from_host + 2);

	// if security access must be unlocked
	const uint8_t require_unlock = kiss_security_check_service_req_unlocking(KISS_RESTART, transport_media, reset_type);

	// as for now only soft reset is allowed
	if (reset_type == KISS_RESET_SOFT) {

		// get last bootup date
		const uint32_t last_restart_date = backup_reg_get_last_restart_date();

		// only one restart per day is allowed, more needs security access unlock
		if ((RTC->DR != last_restart_date) || (require_unlock == 1 && kiss_security_access_get_unlocked_per_transport (transport_media) == 1)) {

			// update last restart date
			backup_reg_set_last_restart_date();

			rte_main_reboot_scheduled_diag = 1;

			// if response is correct fill the buffer with the rest of stuff
			response_buffer[0] = FEND;
			response_buffer[1] = NONSTANDARD;
			response_buffer[2] = 5;				// message lenght
			response_buffer[3] = KISS_RESTART_RESP;
			response_buffer[4] = FEND;
		}
		else {
			out = kiss_nrc_response_fill_conditions_not_correct(response_buffer);
		}
	}
	else {
		out = kiss_nrc_response_fill_request_out_of_range(response_buffer);
	}

	return out;
}

/**
 * 
 */
int32_t kiss_callback_routine_control(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media)
{
	/**
	 * Input frame structure
	 *
	 * FEND, KISS_ROUTINE_CONTROL, SUBFUNCTION_START_STOP_RESULT, ROUTINE_ID_LSB, ROUTINE_ID_MSB, lparam, wparam, FEND
	 */

	int32_t out = 0;

	// this is a result of starting or stopping a routine, but it might be also a result of routine operation
	uint16_t routine_processing_result = 0;

	// what to do with given routine identifier
	const uint8_t subfunction = *(input_frame_from_host + 2);

	// routine identifier
	const uint16_t routineid = *(input_frame_from_host + 3) | (*(input_frame_from_host + 4) << 8);

	const uint32_t lparam = 	*(input_frame_from_host + 5) | (*(input_frame_from_host + 6) << 8) | 
								(*(input_frame_from_host + 7) << 16) | (*(input_frame_from_host + 8) << 24);

	const uint32_t wparam = 	(*(input_frame_from_host + 9)) | (*(input_frame_from_host + 10) << 8);

	// checks if given function ID exists
	const uint8_t routine_type = kiss_routine_control_check_routine_id(routineid);

	// if routine is defined simply call it
	if (routine_type != 0) {
		switch (subfunction) {

			case KISS_ROUTINE_CONTROL_SUBFUNC_START:
				// greater than is used here instead of equals, because of a difference between diagnostics got
				// through serial port and decoded from aprs message. diagnostics from aprs message has only
				// FEND put at the beginning to retain offsets.
				if (input_len > KISS_CALLBACK_ROUTINE_CONTROL_START_MINIMUM_PAYLOAD_LN ) {
					routine_processing_result = kiss_routine_control_start_routine(routineid, wparam, lparam);

					if (routine_processing_result == KISS_ROUTINE_RETVAL_SUCCESSFULLY_STARTED) {
						out = 8;			// size of a response

						response_buffer[0] = FEND;
						response_buffer[1] = NONSTANDARD;
						response_buffer[2] = out;				// message lenght
						response_buffer[3] = KISS_ROUTINE_CONTROL_RESP;
						response_buffer[4] = KISS_ROUTINE_CONTROL_SUBFUNC_START;
						response_buffer[5] = *(input_frame_from_host + 3);
						response_buffer[6] = *(input_frame_from_host + 4);
						response_buffer[7] = FEND;
					}
					else if (routine_processing_result == KISS_ROUTINE_RETVAL_WRONG_PARAMS_VALUES) {
						out = kiss_nrc_response_fill_incorrect_message_lenght_or_format(response_buffer);
					}
					else {	// KISS_ROUTINE_RETVAL_GENERAL_CATASTROPHIC_ERROR
						out = kiss_nrc_response_fill_general_reject(response_buffer);
					}
				}
				else {
					out = kiss_nrc_response_fill_incorrect_message_lenght_or_format(response_buffer);
				}
				break;
			case KISS_ROUTINE_CONTROL_SUBFUNC_STOP:
				routine_processing_result = kiss_routine_control_stop_routine(routineid);

				if (routine_processing_result == KISS_ROUTINE_RETVAL_STOP_FOR_SYNCHRO_ROUTINE) {
					// synchronous routine cannot be stopped, because it block diagnostics I/O on the
					// controller and terminates itself after everything is done
					out = kiss_nrc_response_fill_subfunction_not_supported(response_buffer);
				}
				else if (routine_processing_result == KISS_ROUTINE_RETVAL_STOP_FOR_NOT_RUNNING) {
					// it is not possible to stop a routine which is not running now
					out = kiss_nrc_response_fill_sequence_error(response_buffer);
				}
				else {
					// success
					out = 8;			// size of a response //  out = KISS_ROUTINE_RETVAL_SUCCESS;

					response_buffer[0] = FEND;
					response_buffer[1] = NONSTANDARD;
					response_buffer[2] = out;				// message lenght
					response_buffer[3] = KISS_ROUTINE_CONTROL_RESP;
					response_buffer[4] = KISS_ROUTINE_CONTROL_SUBFUNC_STOP;
					response_buffer[5] = *(input_frame_from_host + 3);
					response_buffer[6] = *(input_frame_from_host + 4);
					response_buffer[7] = FEND;
				}
				break;
			case KISS_ROUTINE_CONTROL_SUBFUNC_RESULT:

				routine_processing_result = kiss_routine_control_get_result_routine(routineid);

				// success
				out = 10;			// size of a response //  out = KISS_ROUTINE_RETVAL_SUCCESS;

				response_buffer[0] = FEND;
				response_buffer[1] = NONSTANDARD;
				response_buffer[2] = out;				// message lenght
				response_buffer[3] = KISS_ROUTINE_CONTROL_RESP;
				response_buffer[4] = KISS_ROUTINE_CONTROL_SUBFUNC_RESULT;
				response_buffer[5] = *(input_frame_from_host + 3);
				response_buffer[6] = *(input_frame_from_host + 4);
				response_buffer[7] = routine_processing_result & 0xFFu;
				response_buffer[8] = (routine_processing_result & 0xFF00u) >> 8;
				response_buffer[9] = FEND;

				break;
		}
	}
	else {
		out = kiss_nrc_response_fill_request_out_of_range(response_buffer);
	}

	return out;
}

