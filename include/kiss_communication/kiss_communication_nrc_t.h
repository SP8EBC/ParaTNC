/*
 * kiss_communication_nrc_t.h
 *
 *	Enum with Negative Return Codes for sudoUDS diagnostics via KISS
 *
 *  Created on: Jun 27, 2023
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_KISS_COMMUNICATION_NRC_T_H_
#define KISS_COMMUNICATION_KISS_COMMUNICATION_NRC_T_H_


typedef enum kiss_communication_nrc_t {
	NRC_POSITIVE = 0x00,

	/**
	 * This response code indicates that the requested action has been rejected
	 * by the server. The generalReject response code shall only be implemented
	 * in the server if none of the negative response codes defined in this
	 * document meet the needs of the implementation. At no means shall this
	 * response code be a general replacement for other response codes defined.
	 */
	NRC_GENERAL_REJECT = 0x10,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the server does not support the requested service. The server
	 * shall send this response code in case the client has sent a request
	 * message with a service identifier, which is either unknown or not
	 * supported by the server. Therefore this negative response code is not
	 * shown in the list of negative response codes to be supported for
	 * a diagnostic service, because this negative response code is not
	 * applicable for supported services.
	 */
	NRC_SERVICE_NOT_SUPPORTED = 0x11,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the server does not support the service specific parameters of
	 * the request message. The server shall send this response code in case
	 * the client has sent a request message with a known and supported service
	 * identifier but with "sub function“ which is either unknown or
	 * not supported.
	 */
	NRC_SUBFUNCTION_NOT_SUPPORTED = 0x12,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the length of the received request message does not match
	 * the prescribed length for the specified service or the format of
	 * the parameters do not match the prescribed format for the specified service.
	 */
	NRC_INCORRECT_MESSAGE_LENGTH_OR_FORMAT = 0x13,

	/**
	 * This response code shall be reported by the server if the response to
	 * be generated exceeds the maximum number of bytes available by
	 * the underlying network layer.
	 */
	NRC_RESPONSE_TOO_LONG = 0x14,

	/**
	 * This response code indicates that the server is temporarily too busy
	 * to perform the requested operation. In this circumstance the client
	 * shall perform repetition of the "identical request message" or
	 * "another request message". The repetition of the request shall be delayed
	 * by a time specified in the respective implementation documents.
	 */
	NRC_BUSY_REPEAT_REQUEST = 0x21,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the server prerequisite conditions are not met.
	 */
	NRC_CONDITIONS_NOT_CORRECT = 0x22



}kiss_communication_nrc_t;


#endif /* KISS_COMMUNICATION_KISS_COMMUNICATION_NRC_T_H_ */
