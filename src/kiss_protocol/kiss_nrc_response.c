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

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN	6
#define KISS_NRC_RESPONSE_OUT_OF_RANGE_LN		6
#define KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN	6
#define KISS_NRC_RESPONSE_SEC_ACCESS_DENIED_LN	6
#define KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN	6
#define KISS_NRC_RESPONSE_SEQUENCE_ERR_LN		6
#define KISS_NRC_RESPONSE_ACCESS_DENIED_LN		6

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

//!< Neagitve response to a request with unknown service id
static const uint8_t kiss_nrc_response_unknown_service[KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_SERVICE_NOT_SUPPORTED,
		FEND};

		//!< Neagitve response to a request with unknown service id
static const uint8_t kiss_nrc_response_subfunction_not_supported[KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_SUBFUNCTION_NOT_SUPPORTED,
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

//!< NRC_INCORRECT_MESSAGE_LENGTH_OR_FORMAT
static const uint8_t kiss_nrc_response_incorrect_message_ln_or_format[KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_INCORRECT_MESSAGE_LENGTH_OR_FORMAT,
		FEND
};

//!< NRC_SECURITY_ACCESS_DENIED
static const uint8_t kiss_nrc_response_security_access_denied[KISS_NRC_RESPONSE_SEC_ACCESS_DENIED_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_SEC_ACCESS_DENIED_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_SECURITY_ACCESS_DENIED,
		FEND
};

//!< NRC_CONDITIONS_NOT_CORRECT
static const uint8_t kiss_nrc_conditions_not_correct[KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_CONDITIONS_NOT_CORRECT,
		FEND
};

//!< NRC_SEQUENCE_ERROR
static const uint8_t kiss_nrc_sequence_error[KISS_NRC_RESPONSE_SEQUENCE_ERR_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_SEQUENCE_ERR_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_REQUEST_SEQUENCE_ERROR,
		FEND
};

//!< NRC_GENERAL_REJECT
static const uint8_t kiss_nrc_general_reject[KISS_NRC_RESPONSE_SEQUENCE_ERR_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_SEQUENCE_ERR_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_GENERAL_REJECT,
		FEND
};


/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param buffer
 * @return
 */
int kiss_nrc_response_fill_unknown_service(uint8_t * buffer) {

	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_unknown_service, KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN);

		return KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN;
	}

	return 0;
}

/**
 *
 * @param buffer
 * @return
 */
int kiss_nrc_response_fill_subfunction_not_supported(uint8_t * buffer) {

	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_subfunction_not_supported, KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN);

		return KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN;
	}

	return 0;
}

/**
 *
 * @param buffer
 * @return
 */
int kiss_nrc_response_fill_request_out_of_range(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_out_of_range, KISS_NRC_RESPONSE_OUT_OF_RANGE_LN);

		return KISS_NRC_RESPONSE_OUT_OF_RANGE_LN;
	}

	return 0;
}

/**
 *
 * @param buffer
 * @return a lenght of a response with a nrc
 */
int kiss_nrc_response_fill_incorrect_message_lenght_or_format(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_incorrect_message_ln_or_format, KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN);

		return KISS_NRC_RESPONSE_INCORRECT_MESSAGE_LN;
	}

	return 0;
}

/**
 *
 * @param buffer
 * @return
 */
int kiss_nrc_response_fill_security_access_denied(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_response_security_access_denied, KISS_NRC_RESPONSE_OUT_OF_RANGE_LN);

		return KISS_NRC_RESPONSE_OUT_OF_RANGE_LN;
	}

	return 0;
}

/**
 *
 * @param buffer
 * @return
 */
int kiss_nrc_response_fill_conditions_not_correct(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_conditions_not_correct, KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN);

		return KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN;
	}

	return 0;
}

/**
 *
 * @param buffer
 * @return
 */
int kiss_nrc_response_fill_sequence_error(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_sequence_error, KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN);

		return KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN;
	}

	return 0;
}

/**
 *
 * @param buffer
 * @return
 */
int kiss_nrc_response_fill_general_reject(uint8_t * buffer) {
	if (buffer != 0x00) {
		memcpy(buffer, kiss_nrc_general_reject, KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN);

		return KISS_NRC_RESPONSE_COND_NOT_CORRECT_LN;
	}

	return 0;
}


