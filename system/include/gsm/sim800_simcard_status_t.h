/*
 * sim800_simcard_status_t.h
 *
 *  Created on: Jun 10, 2023
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800_SIMCARD_STATUS_T_H_
#define INCLUDE_GSM_SIM800_SIMCARD_STATUS_T_H_

/**
 * Enum which holds a possible values of current SIM card status
 */
typedef enum sim800_simcard_status_t {
	SIMCARD_READY,       /**< SIMCARD_READY Simcard is unlocked and ready */
	SIMCARD_PIN_REQUIRED,/**< SIMCARD_PIN_REQUIRED Simcard is present but requires a PIN code*/
	SIMCARD_ERROR,       /**< SIMCARD_ERROR No communication with card */
	SIMCARD_UNKNOWN      /**< SIMCARD_UNKNOWN */
}sim800_simcard_status_t;


#endif /* INCLUDE_GSM_SIM800_SIMCARD_STATUS_T_H_ */
