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

//#define KISS_COMMUNICATION_NRC_SERVICE		KISS_NEGATIVE_RESPONSE_SERVICE

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
	NRC_CONDITIONS_NOT_CORRECT = 0x22,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the server expects a different sequence of request messages or
	 * message as sent by the client. This may occur when sequence sensitive
	 * requests are issued in the wrong order.
	 */
	NRC_REQUEST_SEQUENCE_ERROR = 0x24,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the server has detected that the request message contains
	 * a parameter which attempts to substitute a value beyond its range of
	 * authority (e.g. attempting to substitute a data byte of 111 when the data
	 *  is only defined to 100), or which attempts to access a
	 *  dataIdentifier/routineIdentifer that is not supported or not supported
	 *  in active session. This response code shall be implemented for
	 *  all services, which allow the client to read data, write data or
	 *  adjust functions by data in the server.
	 */
	NRC_REQUEST_OUT_OF_RANGE = 0x31,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the server's security strategy has not been satisfied by
	 * the client. The server shall send this response code if one of the
	 * following cases occur:
	 *    the test conditions of the server are not met,
	 *    the required message sequence
	 *    			 e.g. DiagnosticSessionControl, securityAccess is not met,
	 *    the client has sent a request message which requires an unlocked server.
	 */
	NRC_SECURITY_ACCESS_DENIED = 0x33,

	/**
	 * This response code indicates that the server has not given security
	 * access because the key sent by the client did not match with the key
	 * in the server's memory. This counts as an attempt to gain security.
	 * The server shall remain locked and increment its internal
	 * securityAccessFailed counter.
	 */
	NRC_INVALID_KEY = 0x35,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the client has unsuccessfully attempted to gain security access
	 * more times than the server's security strategy will allow.
	 */
	NRC_EXCEED_NUMBER_OF_ATTEMPTS = 0x36,

	/**
	 * This response code indicates that an attempt to upload/download to
	 * a server's memory cannot be accomplished due to some fault conditions.
	 */
	NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70,

	/**
	 * This response code indicates that the server detected an error when
	 * erasing or programming a memory location in the permanent memory
	 * device (e.g. Flash Memory).
	 */
	NRC_GENERAL_PROGRAMMING_FAIL = 0x72,

	/**
	 * This response code indicates that the request message was received
	 * correctly, and that all parameters in the request message were valid,
	 * but the action to be performed is not yet completed and the server is
	 * not yet ready to receive another request. As soon as the requested
	 * service has been completed, the server shall send a positive response
	 * message or negative response message with a response code different
	 * from this. The negative response message with this response code may be
	 * repeated by the server until the requested service is completed and
	 * the final response message is sent. This response code might impact
	 * the application layer timing parameter values. The detailed specification
	 * shall be included in the data link specific implementation document.
	 */
	NRC_REQUEST_CORRECT_RESPONSE_PENDING = 0x78,

	/**
	 * This response code indicates that the requested action will not be taken
	 * because the server does not support the requested service in the session
	 * currently active. This response code shall only be used when
	 * the requested service is known to be supported in another session,
	 * otherwise response code 0x11 (serviceNotSupported) shall be used.
	 * This response code is in general supported by each diagnostic service,
	 * as not otherwise stated in the data link specific implementation
	 * document, therefore it is not listed in the list of applicable response
	 * codes of the diagnostic services.
	 */
	NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7F



}kiss_communication_nrc_t;


#endif /* KISS_COMMUNICATION_KISS_COMMUNICATION_NRC_T_H_ */
