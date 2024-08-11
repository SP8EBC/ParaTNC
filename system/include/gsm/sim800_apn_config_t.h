/*
 * sim800_apn_config.h
 *
 *  Created on: Aug 6, 2024
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800_APN_CONFIG_T_H_
#define INCLUDE_GSM_SIM800_APN_CONFIG_T_H_

#include <stdint.h>
#include "sim800_mcc_t.h"

typedef struct sim800_apn_config_t {
	const sim800_mcc_t country_code;
	const uint8_t mobile_network_code;
	const char * apn_name;
	const char * username;
	const char * password;
}sim800_apn_config_t;


#endif /* INCLUDE_GSM_SIM800_APN_CONFIG_T_H_ */
