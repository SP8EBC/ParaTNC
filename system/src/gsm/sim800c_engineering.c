/*
 * sim800c_engineering.c
 *
 *  Created on: Jan 26, 2022
 *      Author: mateusz
 */

#include "gsm/sim800c_engineering.h"
#include "gsm/sim800c.h"

#include <event_log.h>
#include <events_definitions/events_gsm_gprs.h>

#include "drivers/serial.h"

#include <stdlib.h>
#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define CENG0_RECORD_LN 77 // including '+CENG' heading

#define ARFCN_OFFSET  10
#define ARFCN_LN	  4
#define CELLID_OFFSET 31
#define CELLID_LN	  4
#define LAC_OFFSET	  42
#define LAC_LN		  4

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

typedef enum gsm_sim800_engineering_state_t {
	GSM_SIM800_ENGINEERING_NOT_ENABLED,
	GSM_SIM800_ENGINEERING_ENABLING,
	GSM_SIM800_ENGINEERING_ENABLED,
	GSM_SIM800_ENGINEERING_REQUESTED,
	GSM_SIM800_ENGINEERING_SUCCEEDED,
	GSM_SIM800_ENGINEERING_SUCCEEDED_DISABLED,
	GSM_SIM800_ENGINEERING_FAILED
} gsm_sim800_engineering_state_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static const char *OK = "OK\r\n\0";
static const char *CENG0 = "+CENG: 0,\0";

/**
 * Set to one externally to request engineering, get one time at startup by default
 */
static uint8_t gsm_sim800_engineering_requested = 1;

static uint8_t gsm_sim800_engineering_delay = 5;

static gsm_sim800_engineering_state_t gsm_sim800_engineering_state =
	GSM_SIM800_ENGINEERING_NOT_ENABLED;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

const char *ENGINEERING_ENABLE = "AT+CENG=4,0\r\n\0";
const char *ENGINEERING_DISABLE = "AT+CENG=0,0\r\0";
const char *ENGINEERING_GET = "AT+CENG?\r\0";

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param srl_rx_buf_pointer
 * @param buffer_ln
 * @param gsm_response_start_idx
 * @return
 */
static uint16_t gsm_sim800_rewind_to_ceng_0 (uint8_t *srl_rx_buf_pointer, uint16_t buffer_ln,
											 uint16_t gsm_response_start_idx)
{

	int comparision_result = 123;

	// iterator over data returned by the modem
	int i = gsm_response_start_idx;

	// calculate the length of CENG0 term to omit recalculation each loop iteration
	int ceng_ln = strlen (CENG0);

	for (; (i < buffer_ln - ceng_ln) && *(srl_rx_buf_pointer + i) != 0; i++) {
		comparision_result = memcmp ((const void *)CENG0, srl_rx_buf_pointer + i, ceng_ln);

		if (comparision_result == 0) {
			return i;
		}
	}

	return 0xFFFF;
}

/**
 *
 * @param srl_rx_buf_pointer
 * @param ceng_0_payload_start
 */
static void gsm_sim800_engineering_parse_ceng_0 (uint8_t *srl_rx_buf_pointer,
												 uint16_t ceng_0_payload_start)
{

	/**
	 * 	Details:0x20000828 <srl_usart3_rx_buffer>
	 * 	"AT+CENG?\r\r\n
	 * 	+CENG: 4,0\r\n\r\n
	 * 	+CENG: 0,\"0037,44,00,260,01,43,67f4,05,05,539d,255,-69,145,145,x,x,x,x,x,x,x\"\r\n
	 * 	+CENG: 1,\"0009,25,61,260,01,539d,80,56\"\r\n
	 * 	+CENG: 2,\"0008,25,15,260,01,539d,78,54\"\r\n+CENG: 3,\"0038"...
	 *
	 */

	// temporary buffer for strings
	char string_buffer[5];

	// temporary variable for string -> int conversion
	int integer = 0;

	// zeroing string buffer
	memset (string_buffer, 0x0, sizeof (string_buffer));

	// check if the record has been found
	if (ceng_0_payload_start > (0xFFFF - CENG0_RECORD_LN)) {
		return;
	}

	// copying ARFCN
	memcpy (string_buffer, srl_rx_buf_pointer + ceng_0_payload_start + ARFCN_OFFSET, ARFCN_LN);

	// converting ARFCN to integer
	integer = atoi (string_buffer);

	if (integer < 125) {
		gsm_sim800_bcch_frequency = 890.0f + 0.2f * integer + 45.0f;
	}
	else if (integer > 511 && integer < 886) {
		gsm_sim800_bcch_frequency = 1710.2f + 0.2f * (integer - 512) + 95.0f;
	}
	else if (integer > 974 && integer < 1024) {
		gsm_sim800_bcch_frequency = 890.0f + 0.2f * (integer - 1024) + 45.0f;
	}

	// zeroing string buffer
	memset (string_buffer, 0x0, sizeof (string_buffer));

	// copying CELL-ID string
	memcpy (gsm_sim800_cellid,
			srl_rx_buf_pointer + ceng_0_payload_start + CELLID_OFFSET,
			CELLID_LN);

	// copying LAC
	memcpy (gsm_sim800_lac, srl_rx_buf_pointer + ceng_0_payload_start + LAC_OFFSET, LAC_LN);

	gsm_sim800_cellid_int = (uint16_t)strtol (gsm_sim800_cellid, NULL, 16) & 0xFFFFu;

	gsm_sim800_lac_int = (uint16_t)strtol (gsm_sim800_lac, NULL, 16) & 0xFFFFu;
}

/**
 *
 * @param srl_context
 * @param state
 */
static void gsm_sim800_engineering_enable (srl_context_t *srl_context, gsm_sim800_state_t *state)
{
	// send a command to module
	srl_send_data (srl_context,
				   (const uint8_t *)ENGINEERING_ENABLE,
				   SRL_MODE_ZERO,
				   strlen (ENGINEERING_ENABLE),
				   SRL_INTERNAL);

	// set what has been just send
	gsm_at_command_sent_last = ENGINEERING_ENABLE;

	// switch the internal state
	*state = SIM800_ALIVE_SENDING_TO_MODEM;
}

/**
 *
 * @param srl_context
 * @param state
 */
static void gsm_sim800_engineering_disable (srl_context_t *srl_context, gsm_sim800_state_t *state)
{
	// send a command to module
	srl_send_data (srl_context,
				   (const uint8_t *)ENGINEERING_DISABLE,
				   SRL_MODE_ZERO,
				   strlen (ENGINEERING_DISABLE),
				   SRL_INTERNAL);

	// set what has been just send
	gsm_at_command_sent_last = ENGINEERING_DISABLE;

	// switch the internal state
	*state = SIM800_ALIVE_SENDING_TO_MODEM;
}

/**
 *
 * @param srl_context
 * @param state
 */
static void gsm_sim800_engineering_request_data (srl_context_t *srl_context,
												 gsm_sim800_state_t *state)
{

	// send a command to module
	srl_send_data (srl_context,
				   (const uint8_t *)ENGINEERING_GET,
				   SRL_MODE_ZERO,
				   strlen (ENGINEERING_GET),
				   SRL_INTERNAL);

	// set what has been just send
	gsm_at_command_sent_last = ENGINEERING_GET;

	// switch the internal state
	*state = SIM800_ALIVE_SENDING_TO_MODEM;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

uint8_t gsm_sim800_engineering_get_is_done (void)
{
	return ((gsm_sim800_engineering_state == GSM_SIM800_ENGINEERING_SUCCEEDED) ||
			(gsm_sim800_engineering_state == GSM_SIM800_ENGINEERING_FAILED) ||
			(gsm_sim800_engineering_state == GSM_SIM800_ENGINEERING_SUCCEEDED_DISABLED));
}

/**
 *
 * @param srl_context
 * @param state
 */
void gsm_sim800_engineering_pool (srl_context_t *srl_context, gsm_sim800_state_t *state)
{

	switch (gsm_sim800_engineering_state) {
	case GSM_SIM800_ENGINEERING_NOT_ENABLED: {

		if (--gsm_sim800_engineering_delay > 0) {
			;
		}
		else {

			if (gsm_sim800_engineering_requested == 1) {
				gsm_sim800_engineering_enable (srl_context, state);

				gsm_sim800_engineering_requested = 0;

				gsm_sim800_engineering_state = GSM_SIM800_ENGINEERING_ENABLING;
			}
			else {
				;
			}
		}
		break;
	}
	case GSM_SIM800_ENGINEERING_ENABLING: {
		break;
	}
	case GSM_SIM800_ENGINEERING_ENABLED: {
		gsm_sim800_engineering_request_data (srl_context, state);

		gsm_sim800_engineering_state = GSM_SIM800_ENGINEERING_REQUESTED;
		break;
	}
	case GSM_SIM800_ENGINEERING_REQUESTED: {
		break;
	}
	case GSM_SIM800_ENGINEERING_SUCCEEDED: {
		gsm_sim800_engineering_disable (srl_context, state);

		event_log_sync (EVENT_BOOTUP,
						EVENT_SRC_GSM_GPRS,
						EVENTS_GSM_GPRS_SIGNAL_LEVEL,
						gsm_sim800_signal_level_dbm,
						0,
						gsm_sim800_cellid_int,
						gsm_sim800_lac_int,
						0,
						(uint32_t)(gsm_sim800_bcch_frequency * 100.0f));
		break;
	}
	case GSM_SIM800_ENGINEERING_SUCCEEDED_DISABLED: {
		packet_tx_force_gsm_status ();
		break;
	}
	case GSM_SIM800_ENGINEERING_FAILED: {

		event_log_sync (EVENT_ERROR,
						EVENT_SRC_GSM_GPRS,
						EVENTS_GSM_GPRS_SIGNAL_LEVEL,
						gsm_sim800_signal_level_dbm,
						0,
						0,
						0,
						0,
						0);
		break;
	}
	}
}

void gsm_sim800_engineering_response_callback (srl_context_t *srl_context,
											   gsm_sim800_state_t *state,
											   uint16_t gsm_response_start_idx)
{

	if (gsm_at_command_sent_last == ENGINEERING_ENABLE) {
		int comparision_result =
			strcmp (OK, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx));

		if (comparision_result == 0) {
			gsm_sim800_engineering_state = GSM_SIM800_ENGINEERING_ENABLED;
		}
		else {
			gsm_sim800_engineering_state = GSM_SIM800_ENGINEERING_FAILED;
		}
	}
	else if (gsm_at_command_sent_last == ENGINEERING_GET) {

		// look for the start of '+CENG: 0,' record
		uint16_t ceng_start = gsm_sim800_rewind_to_ceng_0 (srl_context->srl_rx_buf_pointer,
														   srl_context->srl_rx_buf_ln,
														   gsm_response_start_idx);

		if (ceng_start != 0xFFFF) {
			// if it has been found
			gsm_sim800_engineering_parse_ceng_0 (srl_context->srl_rx_buf_pointer, ceng_start);

			gsm_sim800_engineering_state = GSM_SIM800_ENGINEERING_SUCCEEDED;
		}
		else {
			gsm_sim800_engineering_state = GSM_SIM800_ENGINEERING_FAILED;
		}
	}
	else if (gsm_at_command_sent_last == ENGINEERING_DISABLE) {
		gsm_sim800_engineering_state = GSM_SIM800_ENGINEERING_SUCCEEDED_DISABLED;
	}
}
