/*
 * sds011.h
 *
 *  Created on: Sep 16, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_DUST_SENSOR_SDS011_H_
#define INCLUDE_DUST_SENSOR_SDS011_H_

#include <stdint.h>

int sds011_get_pms(uint8_t * data, uint16_t data_ln, uint16_t * pm_10, uint16_t * pm_2_5);

#endif /* INCLUDE_DUST_SENSOR_SDS011_H_ */
