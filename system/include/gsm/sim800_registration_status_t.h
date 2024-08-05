/*
 * sim800_registration_status_t.h
 *
 *  Created on: Aug 5, 2024
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800_REGISTRATION_STATUS_T_H_
#define INCLUDE_GSM_SIM800_REGISTRATION_STATUS_T_H_


typedef enum sim800_registration_status_t {
	//!< Not registered, MT is not currently searching a new operator to register to
	SIM800_REGISTRATION_STATUS_NOT_REGISTERED_NOT_SEARCHING =		(0u),

	//!< Registered, home network
	SIM800_REGISTRATION_STATUS_HOME_NETWORK =						(1u),

	//!< Not registered, but MT is currently searching a new operator to register to
	SIM800_REGISTRATION_STATUS_NOT_REGISTERED_SEARCHING =			(2u),

	//!< Registration denied
	SIM800_REGISTRATION_STATUS_DENIED = 							(3u),

	//!< Unknown
	SIM800_REGISTRATION_STATUS_UNKNOWN =							(4u),

	//!< Registered, home network
	SIM800_REGISTRATION_STATUS_ROAMING =							(5u)
}sim800_registration_status_t;


#endif /* INCLUDE_GSM_SIM800_REGISTRATION_STATUS_T_H_ */
