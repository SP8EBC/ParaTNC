/*
 * kiss_nrc_response.c
 *
 *  Created on: Jul 2, 2023
 *      Author: mateusz
 */


#include <kiss_communication/kiss_nrc_response.h>
#include "kiss_communication/types/kiss_communication_nrc_t.h"
#include "kiss_communication/kiss_communication.h"
#include "kiss_communication/types/kiss_communication_service_ids.h"
#include "kiss_configuation.h"

#include <string.h>

#define KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN	6
#define KISS_NRC_RESPONSE_OUT_OF_RANGE_LN		6
#define KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN	6
#define KISS_NRC_RESPONSE_SEC_ACCESS_DENIED_LN	6
#define KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN	6


//!< Neagitve response to a request with unknown service id
static const uint8_t kiss_nrc_response_unknown_service[KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_SERVICE_NOT_SUPPORTED,
		FEND};

//!< Negative response to our of range request
static const uint8_t kiss_nrc_response_out_of_range[KISS_NRC_RESPONSE_OUT_OF_RANGE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_REQUEST_OUT_OF_RANGE,
		FEND
};

//!< Negative response to incorrect message lenght or format
static const uint8_t kiss_nrc_response_incorrect_message_ln_or_format[KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_INCORRECT_MESSAGE_LENGTH_OR_FORMAT,
		FEND
};

//!< Negative response to incorrect message lenght or format
static const uint8_t kiss_nrc_response_security_access_denied[KISS_NRC_RESPONSE_SEC_ACCESS_DENIED_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_SEC_ACCESS_DENIED_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_SECURITY_ACCESS_DENIED,
		FEND
};

// NRC_CONDITIONS_NOT_CORRECT
static const uint8_t kiss_nrc_conditions_not_correct[KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_CONDITIONS_NOT_CORRECT,
		FEND
};


int kiss_nrc_response_fill_unknown_service(uint8_t * buffer) {

	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_unknown_service, KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN);

		return KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN;
	}

	return 0;
}

int kiss_nrc_response_fill_request_out_of_range(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_out_of_range, KISS_NRC_RESPONSE_OUT_OF_RANGE_LN);

		return KISS_NRC_RESPONSE_OUT_OF_RANGE_LN;
	}

	return 0;
}

int kiss_nrc_response_fill_incorrect_message_ln(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_incorrect_message_ln_or_format, KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN);

		return KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN;
	}

	return 0;
}

int kiss_nrc_response_fill_security_access_denied(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_security_access_denied, KISS_NRC_RESPONSE_OUT_OF_RANGE_LN);

		return KISS_NRC_RESPONSE_OUT_OF_RANGE_LN;
	}

	return 0;
}

int kiss_nrc_response_fill_conditions_not_correct(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_conditions_not_correct, KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN);

		return KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN;
	}

	return 0;
}

