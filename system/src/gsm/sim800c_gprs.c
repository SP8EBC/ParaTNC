#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c.h"
#include "gsm/sim800c_inline.h"

#include "drivers/serial.h"

#include "float_to_string.h"

#include <stdint.h>
#include <string.h>


const char * START_CONFIG_APN 			= "AT+CSTT=\0";
const char * SHUTDOWN_GPRS 				= "AT+CIPSHUT\r\0";
const char * SHUTDOWN_GRPS_RESPONSE 	= "SHUT OK\0";
const char * ENABLE_EDGE				= "AT+CEGPRS=1,10\r\0";
const char * START_GPRS					= "AT+CIICR\r\0";
const char * GET_IP_ADDRESS				= "AT+CIFSR\r\0";
const char * GET_CONNECTION_STATUS		= "AT+CIPSTATUS\r\0";
const char * CONFIGURE_DTR				= "AT&D1\r\0";


static const char * OK = "OK\r\n\0";
static const char * QUOTATION = "\"\0";
static const char * COMMA = ",\0";
static const char * NEWLINE = "\r\0";
static const char * STATE = "STATE: \0";
static const char * IPSTATUS = "IP STATUS\0";

const config_data_gsm_t * gsm_sim800_gprs_config_gsm;

/**
 * Set to one if GSM radio is connected to a network and
 * GPRS connection is established
 */
int8_t gsm_sim800_gprs_ready = 0;

/**
 * IP Address of GPRS connection retrieved from module
 * using AT commands
 */
char gsm_sim800_ip_address[18];

char gsm_sim800_connection_status_str[24];

inline static void gsm_sim800_replace_non_printable_with_space(char * str) {
	for (int i = 0; *(str + i) != 0 ; i++) {
		char current = *(str + i);

		if (current != 0x00) {
			if (current < 0x21 || current > 0x7A) {
				*(str + i) = ' ';
			}
		}
	}
}

void sim800_gprs_initialize(srl_context_t * srl_context, gsm_sim800_state_t * state, const config_data_gsm_t * config_gsm) {

	if (*state != SIM800_ALIVE) {
		return;
	}

	if (gsm_sim800_get_waiting_for_command_response() == 1) {
		return;
	}

	gsm_sim800_gprs_config_gsm = config_gsm;

	*state = SIM800_INITIALIZING_GPRS;


}

void sim800_gprs_create_apn_config_str(char * buffer, uint16_t buffer_ln) {

	memset(buffer, 0x00, buffer_ln);

	//append prefix
	strcat(buffer, START_CONFIG_APN);

	//append apn
	strcat(buffer, QUOTATION);
	strncat(buffer, gsm_sim800_gprs_config_gsm->apn, 24);
	strcat(buffer, QUOTATION);

	// if username & password was provided
	if (strlen(gsm_sim800_gprs_config_gsm->username) > 0 && strlen(gsm_sim800_gprs_config_gsm->password) > 0) {
		// append comma and quotation mark
		strcat(buffer, COMMA);
		strcat(buffer, QUOTATION);

		// append username
		strncat(buffer, gsm_sim800_gprs_config_gsm->username, 24);

		strcat(buffer, QUOTATION);
		strcat(buffer, COMMA);
		strcat(buffer, QUOTATION);

		// append password
		strncat(buffer, gsm_sim800_gprs_config_gsm->password, 24);

		// append newline
		strcat(buffer, QUOTATION);


	}

	strcat(buffer, NEWLINE);

}

/**
 * This callback
 */
void sim800_gprs_response_callback(srl_context_t * srl_context, gsm_sim800_state_t * state, uint16_t gsm_response_start_idx) {

	int comparision_result = 0;

	int stringln = 0;

	int i = 0;

	if (gsm_at_command_sent_last == SHUTDOWN_GPRS && srl_context->srl_rx_state != SRL_RX_ERROR) {
		comparision_result = strncmp(SHUTDOWN_GRPS_RESPONSE, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 7);
	}
	else if (gsm_at_command_sent_last == START_GPRS && srl_context->srl_rx_state != SRL_RX_ERROR) {
		comparision_result = strncmp(OK, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 2);

	}
	else if (gsm_at_command_sent_last == GET_IP_ADDRESS) {
		memset(gsm_sim800_ip_address, 0, 18);

		strncpy(gsm_sim800_ip_address, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 18);

		gsm_sim800_replace_non_printable_with_space(gsm_sim800_ip_address);
	}
	else if (gsm_at_command_sent_last == CONFIGURE_DTR) {
		comparision_result = strncmp(OK, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 2);
	}
	else if (gsm_at_command_sent_last == GET_CONNECTION_STATUS ) {

		memset(gsm_sim800_connection_status_str, 0x00, 24);

		// check lenght of the response
		stringln = strlen((const char * )srl_context->srl_rx_buf_pointer);

		// check if the reponse has senseful lenght (not to short to be valid)
		if (stringln > 8) {
			// loop backwards from the end
			for (i = stringln; i > 0; i--) {
				comparision_result = strncmp(STATE, (const char * )srl_context->srl_rx_buf_pointer + i, 7);

				if (comparision_result == 0) {
					memcpy(gsm_sim800_connection_status_str, srl_context->srl_rx_buf_pointer + i + 7, stringln - i - 7);

					gsm_sim800_replace_non_printable_with_space(gsm_sim800_connection_status_str);
				}
			}
		}

		// check if connection is alive and kickin'
		stringln = strlen(gsm_sim800_ip_address);

		// check ip address ln is greater than 7 (x.x.x.x)
		if (stringln > 7) {
			// check status
			comparision_result = strncmp(IPSTATUS, gsm_sim800_connection_status_str, 9);

			if (comparision_result == 0) {
				gsm_sim800_gprs_ready = 1;
			}
			else {
				gsm_sim800_gprs_ready = 0;
			}
		}
	}
	else if (srl_context->srl_rx_state == SRL_RX_DONE || srl_context->srl_rx_state == SRL_RX_IDLE){
		comparision_result = strncmp(OK, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 2);
	}


	// check if modem response is the same with what the library actualy expects to get
	if (comparision_result != 0) {
		*state = SIM800_NOT_YET_COMM;	// if not reset the state to start reinitializing
	}

	if (gsm_sim800_gprs_ready == 1) {
		*state = SIM800_ALIVE;
	}
}

void sim800_gprs_reset(void){
	gsm_sim800_gprs_ready = 0;

	memset (gsm_sim800_ip_address, 0x00, 18);
}

/**
 * Create a text status message
 * @param buffer
 * @param ln
 */
void sim800_gprs_create_status(char * buffer, int ln) {
	// check if output buffer has been provided
	if (buffer != 0) {
		snprintf(buffer, ln, "[IP addr: %s]", gsm_sim800_ip_address);
	}
}
