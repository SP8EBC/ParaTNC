/*
 * sim800_return_t.h
 *
 *  Created on: May 25, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800_RETURN_T_H_
#define INCLUDE_GSM_SIM800_RETURN_T_H_

/**
 *	Possible return values from any SIM800 GPRS module related function
 */
typedef enum sim800_return_t {

	SIM800_OK					= 0,            /**< SIM800_OK */
	SIM800_RX_TERMINATED		= 1,            /**< SIM800_RX_TERMINATED */
	SIM800_WRONG_STATE_TO_RX		= 100,      /**< SIM800_WRONG_STATE_TO_RX */
	SIM800_WRONG_STATE_TO_TX		= 101,      /**< SIM800_WRONG_STATE_TO_TX */
	SIM800_WRONG_STATE_TO_CONNECT	= 102,  	/**< SIM800_WRONG_STATE_TO_CONNECT */
	SIM800_WRONG_STATE_TO_CLOSE		= 103,   	/**< SIM800_WRONG_STATE_TO_CLOSE */
	SIM800_ADDRESS_AND_PORT_TO_LONG	= 104,		/**< SIM800_ADDRESS_AND_PORT_TO_LONG */
	SIM800_CONNECTING_FAILED		= 105,      /**< SIM800_CONNECTING_FAILED */
	SIM800_RECEIVING_TIMEOUT		= 106,      /**< SIM800_RECEIVING_TIMEOUT	Timeout during waiting for GPRS module response, if TCP connection is established
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	in the transparent mode this might be also caused by no response from remote server */
	SIM800_SEND_FAILED				= 107,      /**< SIM800_SEND_FAILED */
	SIM800_TCP_CLOSE_UNCERTAIN		= 108,    	/**< SIM800_TCP_CLOSE_UNCERTAIN */
	SIM800_TCP_CLOSE_ALREADY		= 109,      /**< SIM800_TCP_CLOSE_ALREADY */

	SIM800_UNSET					= -1        /**< SIM800_UNSET */

} sim800_return_t;

#endif /* INCLUDE_GSM_SIM800_RETURN_T_H_ */
