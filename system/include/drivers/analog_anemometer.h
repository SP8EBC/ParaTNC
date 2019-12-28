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

extern uint16_t analog_anemometer_windspeed_pulses_time[ANALOG_ANEMOMETER_SPEED_PULSES_N];
extern uint16_t analog_anemometer_pulses_durations[ANALOG_ANEMOMETER_SPEED_PULSES_N];
extern uint16_t analog_anemometer_pulses_per_ms_constant;
extern uint8_t analog_anemometer_timer_has_been_fired;
extern uint8_t analog_anemometer_slew_limit_fired;

void analog_anemometer_init(	uint16_t pulses_per_ms,
								uint16_t mvolts_for_1deg,
								uint16_t mvolts_for_359deg,
								uint8_t reversed);

void analog_anemometer_timer_irq(void);
void analog_anemometer_dma_irq(void);

#endif /* INCLUDE_DRIVERS_ANALOG_ANEMOMETER_H_ */
