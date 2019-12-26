/*
 * analog_anemometer.h
 *
 *  Created on: 25.12.2019
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_ANALOG_ANEMOMETER_H_
#define INCLUDE_DRIVERS_ANALOG_ANEMOMETER_H_

#define ANALOG_ANEMOMETER_SPEED_PULSES_N 10

#include <stdint.h>

void analog_anemometer_init(	uint16_t pulses_per_ms,
								uint16_t mvolts_for_1deg,
								uint16_t mvolts_for_359deg,
								uint8_t reversed);

void analog_anemometer_timer_irq(void);

#endif /* INCLUDE_DRIVERS_ANALOG_ANEMOMETER_H_ */
