/*
 * KissCommunication.c
 *
 *  Created on: 29.04.2017
 *      Author: mateusz
 */

#include <kiss_communication/kiss_communication.h>
#include <kiss_communication/kiss_callback.h>
#include <kiss_communication/types/kiss_communication_service_ids.h>
#include <stored_configuration_nvm/config_data_externs.h>
#include <stored_configuration_nvm/configuration_handler.h>

#include <crc.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "station_config.h"

#include "variant.h"

extern unsigned short tx10m;

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/**
 * ID of asynchronous message which is currently transmitteed asynchronously to host PC.
 * If it is set to 0xFF then no async message is transmitted
 */
uint8_t kiss_current_async_message = 0xFF;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param output
 * @param output_len
 * @return
 */
uint8_t kiss_async_pooler(uint8_t* output, uint16_t output_len ) {

	int16_t pooling_result = 0;

	uint8_t out = 0;

	if (kiss_current_async_message == 0xFF) {
		return KISS_RETURN_IDLE;
	}

	switch(kiss_current_async_message) {
		case KISS_RUNNING_CONFIG:
			pooling_result = kiss_pool_callback_get_running_config(output, output_len);

			break;
	}

	// positive return value means that there is something to transmit to host
	// and the value itself points how big the response is
	if (pooling_result > 0) {
		out = pooling_result;
	}
	else if (pooling_result == 0) {
		// if result equals to zero it means that there is nothing more to send
		kiss_current_async_message = 0xFF;
	}
	else {
		// an error has occured if pooling result is negative
		out = 0 - abs(pooling_result);
	}

	return out;
}

/**
 *
 * @param input_frame
 * @param input_frame_len
 * @param output
 * @param output_len
 * @return
 */
int32_t kiss_send_ax25_to_host(uint8_t* input_frame, uint16_t input_frame_len, uint8_t* output, uint16_t output_len) {
	#define FEND	(uint8_t)0xC0
	#define FESC	(uint8_t)0xDB
	#define TFEND	(uint8_t)0xDC
	#define TFESC	(uint8_t)0xDD
	short int i /* Zmienna do poruszania siê po frame */, j /* zmienna do poruszani siê po data*/;

	if (input_frame_len >= output_len)
		return KISS_TOO_LONG_FRM;

	uint8_t* data;
	data = output;

	data[0] = FEND;
	data[1] = 0x00;
//	KissFrm.data[2] = HDLC_FLAG;
	for (j = 2, i = 0; i < input_frame_len; j++, i++) {

		// if we reach the maximu size of an output buffer
		if (j >= output_len)
			return KISS_TOO_LONG_FRM;

		if (*(input_frame+i) != FEND && *(input_frame+i) != FESC)
			data[j] = input_frame[i];
		else if (*(input_frame+i) == FEND) {
			data[j] = FESC;
			*(data + (j++)) = TFEND;
		}
		else if(*(input_frame+i) == FESC) {
			data[j] = FESC;
			*(data + (j++)) = TFESC;
		}
		else {

		}
	}
//	*(KissFrm.data + (j++)) = HDLC_FLAG;
	*(data + (j++)) = FEND;
	return j;
}

/**
 * Function responsible for parsing a KISS message send by a host PC to the controller. If the frame contains
 * some diagnostics request or configuration (NOT regular KISS data with frame to be transmitted), this function
 * might generate a response
 */
/**
 *
 * @param input_frame_from_host
 * @param input_len
 * @param ax25
 * @param a
 * @param response_buffer
 * @param resp_buf_ln
 * @return
 */
int32_t kiss_parse_received (uint8_t *input_frame_from_host, 
							uint16_t input_len, 
							AX25Ctx *ax25,
							Afsk *a, 
							uint8_t *response_buffer, 
							uint16_t resp_buf_ln,
							kiss_communication_transport_t transport_media)
{
	int i /* zmienna do poruszania sie po buforze odbiorczym usart */;
	int j/* zmienna do poruszania sie po lokalnej tablicy do przepisywania*/;

	int32_t output = 0;

	if (variant_validate_is_within_ram(input_frame_from_host) == 0x00) {
		output = KISS_COMM_RESULT_WRONG_POINTER;
	}
	else if (input_len >= OWN_APRS_MSG_LN) {
		output = KISS_COMM_RESULT_INPUT_TOO_LONG;
	}
	else if (*(input_frame_from_host) != FEND) {
		output = KISS_COMM_RESULT_MISSING_PREFIX;
	}
	else {

		// copy data to "big" buffer used to exchange packet from/to RF
		uint8_t *FrameBuff = (uint8_t *)main_own_aprs_msg;

		uint8_t frame_type = *(input_frame_from_host+1);

		// check input frame type
		switch (frame_type) {

			case KISS_DATA: {
				if (variant_validate_is_within_ram(ax25) && variant_validate_is_within_ram(a)) {
					memset(FrameBuff, 0x00, OWN_APRS_MSG_LN);

					// if this is data frame
					for (i=2, j=0; (i<input_len && *(input_frame_from_host+i) != FEND); i++, j++) {
						if (*(input_frame_from_host+i) == FESC) {
							if(*(input_frame_from_host+i+1) == TFEND)
								FrameBuff[j]=FEND;
							else if(*(input_frame_from_host+i+1) == TFESC)
								FrameBuff[j]=FESC;
							else {
								;
							}
							i++;
						}
						else
							FrameBuff[j] = *(input_frame_from_host+i);
					}

					tx10m++;

					// keep this commented until reseting the DCD variable will be moved outside main for (;;) loop
					//	while(ax25->dcd == true);
					while(a->sending == true);


					ax25_sendRaw(ax25,FrameBuff,j);
					afsk_txStart(a);
				}
			} break;

			case KISS_GET_RUNNING_CONFIG: {
				output = kiss_callback_get_running_config(input_frame_from_host, input_len, response_buffer, resp_buf_ln, transport_media);
			} break;

			case KISS_GET_VERSION_AND_ID: {
				output = kiss_callback_get_version_id(input_frame_from_host, input_len, response_buffer, resp_buf_ln, transport_media);
				break;
			}

			case KISS_ROUTINE_CONTROL: {
				output = kiss_callback_routine_control(input_frame_from_host, input_len, response_buffer, resp_buf_ln, transport_media);
				break;
			}

			case KISS_PROGRAM_STARTUP_CFG: {
				output = kiss_callback_program_startup(input_frame_from_host, input_len, response_buffer, resp_buf_ln, transport_media);
				break;
			}

			case KISS_ERASE_STARTUP_CFG: {
				output = kiss_callback_erase_startup(input_frame_from_host, input_len, response_buffer, resp_buf_ln, transport_media);
				break;
			}

			case KISS_READ_DID: {
				output = kiss_callback_read_did(input_frame_from_host, input_len, response_buffer, resp_buf_ln, transport_media);
				break;
			}
			case KISS_READ_MEM_ADDR : {
				output = kiss_callback_read_memory_by_addr(input_frame_from_host, input_len, response_buffer, resp_buf_ln, transport_media);
				break;
			}
			case KISS_RESTART: {
				output = kiss_callback_reset(input_frame_from_host, input_len, response_buffer, resp_buf_ln, transport_media);
				break;
			}


			default: {
				// unknown service
				output = KISS_COMM_RESULT_UNKNOWN_DIAG_SERV;
			}
		}
	}


	return output;
}

/**
 *
 * @param output
 * @param output_len
 * @param current_len
 */
void kiss_reset_buffer(uint8_t* output, uint16_t output_len, uint16_t* current_len) {
	memset(output, 0x00, sizeof(output_len));

	output[0] = FEND;
	output[1] = 0x00;

	*current_len = 2;
}

uint8_t kiss_put_char(uint8_t c, uint8_t* output, uint16_t output_len, uint16_t* current_len, uint16_t* crc) {

	uint16_t new_crc = 0;
	uint16_t curr_ln = *current_len;

	if (*current_len >= output_len) {
		return 1;
	}

	if (c == HDLC_FLAG || c == HDLC_RESET || c == AX25_ESC)
	{
		kiss_put_char_nocheck(AX25_ESC, output, output_len, current_len, crc);
	}

	if (c == FEND)
	{
		kiss_put_char_nocheck(FESC, output, output_len, current_len, crc);
		kiss_put_char_nocheck(TFEND, output, output_len, current_len, crc);
	}

	else if (c == FESC)
	{
		kiss_put_char_nocheck(FESC, output, output_len, current_len, crc);
		kiss_put_char_nocheck(TFESC, output, output_len, current_len, crc);
	}

	else {
		output[curr_ln++] = c;
	}

	if (crc == NULL) {
		;
	}
	else {
		new_crc = updcrc_ccitt(c, *crc);

		*crc = new_crc;
	}

	*current_len = curr_ln;

	return 0;
}

/**
 *
 * @param c
 * @param output
 * @param output_len
 * @param current_len
 * @param crc
 * @return
 */
uint8_t kiss_put_char_nocheck(uint8_t c, uint8_t* output, uint16_t output_len, uint16_t* current_len, uint16_t* crc) {
	uint16_t new_crc = 0;
	uint16_t curr_ln = *current_len;

	if (*current_len >= output_len) {
		return 1;
	}

	output[curr_ln++] = c;

	if (crc == NULL) {
		;
	}
	else {
		new_crc = updcrc_ccitt(c, *crc);

		*crc = new_crc;
	}

	*current_len = curr_ln;

	return 0;
}

/**
 *
 * @param addr
 * @param is_last
 * @param output
 * @param output_len
 * @param current_len
 * @param crc
 */
void kiss_put_call(const AX25Call *addr, uint8_t is_last, uint8_t* output, uint16_t output_len, uint16_t* current_len, uint16_t* crc) {

	uint16_t i;
	uint8_t ssid;
	uint16_t len = MIN(sizeof(addr->call), strlen(addr->call));


	for (i = 0; i < len; i++)
	{
		uint8_t c = addr->call[i];
		kiss_put_char(c << 1, output, output_len, current_len, crc);
	}

	if (len < sizeof(addr->call))
	{
		for (i = 0; i < sizeof(addr->call) - len; i++)
		{
			kiss_put_char(' ' << 1, output, output_len, current_len, crc);
		}
	}

	ssid = 0x60 | (addr->ssid << 1) | (is_last ? 0x01 : 0);
	kiss_put_char(ssid, output, output_len, current_len, crc);

}

/**
 *
 * @param output
 * @param output_len
 * @param current_len
 */
void kiss_finalize_buffer(uint8_t* output, uint16_t output_len, uint16_t* current_len) {

	uint16_t ln = *current_len;

	if (*current_len >= output_len) {
		return;
	}

	output[ln++] = FEND;

	*current_len = ln;
}



