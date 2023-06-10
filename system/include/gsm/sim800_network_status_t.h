/*
 * sim800_network_status_t.h
 *
 *  Created on: Jun 10, 2023
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800_NETWORK_STATUS_T_H_
#define INCLUDE_GSM_SIM800_NETWORK_STATUS_T_H_

/**
 * Enum with possible statuses of GSM network registration
 */
typedef enum sim800_network_status_t {
	NETWORK_STATUS_UNKNOWN,   /**< NETWORK_STATUS_UNKNOWN */
	NETWORK_NOT_REGISTERED,   /**< NETWORK_NOT_REGISTERED */
	NETWORK_REGISTERED,       /**< NETWORK_REGISTERED */
	NETWORK_REGISTERED_ROAMING/**< NETWORK_REGISTERED_ROAMING */
}sim800_network_status_t;


#endif /* INCLUDE_GSM_SIM800_NETWORK_STATUS_T_H_ */
