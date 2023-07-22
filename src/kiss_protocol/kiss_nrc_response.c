/*
 * kiss_nrc_response.c
 *
 *  Created on: Jul 2, 2023
 *      Author: mateusz
 */


#include <kiss_communication/kiss_nrc_response.h>
#include "kiss_communication/kiss_communication_nrc_t.h"
#include "kiss_communication/kiss_communication.h"
#include "kiss_communication/kiss_communication_service_ids.h"
#include "kiss_configuation.h"

#include <string.h>

#define KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN	6
#define KISS_NRC_RESPONSE_OUT_OF_RANGE_LN		6

//!< Neagitve response to a request with unknown service id
static const uint8_t kiss_nrc_response_unknown_service[KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_SERVICE_NOT_SUPPORTED,
		FEND};

static const uint8_t kiss_nrc_response_out_of_range[KISS_NRC_RESPONSE_OUT_OF_RANGE_LN] = {
		FEND,
		NONSTANDARD,
		KISS_NRC_RESPONSE_UNKNOWN_SERVICE_LN,
		KISS_NEGATIVE_RESPONSE_SERVICE,
		NRC_REQUEST_OUT_OF_RANGE,
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

