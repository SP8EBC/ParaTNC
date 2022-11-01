/*
 * nvm.h
 *
 *  Created on: Nov 1, 2022
 *      Author: mateusz
 */

#ifndef NVM_H_
#define NVM_H_

#include <stdint.h>

typedef struct __attribute__((packed)) nvm_measurement_t {
	/**
	 * Date-time timestamp in timezone local for a place where station is installed.
	 * Mixture of BCD and integer format, this is just sligtly processed RTC registers
	 * content.
	 *	bit 0  - bit 12 === number of minutes starting from midnight (max 1440)
	 *	bit 13 - bit 21 === days from new year (max 356)
	 *	bit 25 - bit 31 === years (from 00 to 99, from 2000 up to 2099)
	 */
	uint32_t timestamp;

	/**
	 * Temperature represented as 0.1 degrees increment and
	 * humidity in 2% steps
	 * bit 0  - bit 9  === raw temperature, physical: raw / 10 - 50 (from -50.0 to +52.3)
	 * bit 10 - bit 15 === raw humidity, physical: raw * 2 (from 0% to 100%)
	 */
	uint16_t temperature_humidity;

	/**
	 * Average windspeed and gust windspeed stored in 0.2m/s increments
	 * bit 0  - bit 7 === raw average windspeed, physical: raw / 5 (from 0m/s up to 50m/s)
	 */
	uint16_t wind;

}nvm_measurement_t;

typedef enum nvm_state_result_t {
	NVM_UNINITIALIZED,
	NVM_OK,
	NVM_NO_SPACE_LEFT,
	NVM_INIT_ERROR,
	NVM_PGM_ERROR
}nvm_state_result_t;

void nvm_measurement_init(void);
nvm_state_result_t nvm_measurement_store(nvm_measurement_t * data);

#endif /* NVM_H_ */
