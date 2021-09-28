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

typedef enum analog_wind_qf {
	AN_WIND_QF_FULL,
	AN_WIND_QF_DEGRADED_DEBOUNCE,
	AN_WIND_QF_DEGRADED_SLEW,
	AN_WIND_QF_DEGRADED,
	AN_WIND_QF_NOT_AVALIABLE,
	AN_WIND_QF_UNKNOWN
} analog_wind_qf_t;

#define DIRECTION_REGULAR	1
#define DIRECTION_SPARKFUN	2

//#if defined(_ANEMOMETER_ANALOGUE) || defined(_ANEMOMETER_ANALOGUE_SPARKFUN)

extern uint16_t analog_anemometer_windspeed_pulses_time[ANALOG_ANEMOMETER_SPEED_PULSES_N];
extern uint16_t analog_anemometer_time_between_pulses[ANALOG_ANEMOMETER_SPEED_PULSES_N];
extern uint16_t analog_anemometer_pulses_per_m_s_constant;
extern uint8_t analog_anemometer_timer_has_been_fired;
extern uint8_t analog_anemometer_slew_limit_fired;
extern uint8_t analog_anemometer_deboucing_fired;

void analog_anemometer_init(	uint16_t pulses_per_meter_second,
								uint8_t anemometer_lower_boundary,
								uint8_t anemometer_upper_boundary,
								uint8_t direction_polarity);
void analog_anemometer_deinit(void);
void analog_anemometer_timer_irq(void);
void analog_anemometer_dma_irq(void);
uint32_t analog_anemometer_get_ms_from_pulse(uint16_t inter_pulse_time);
int16_t analog_anemometer_direction_handler(void);
int16_t analog_anemometer_direction_sparkfun(uint32_t timer_value);
void analog_anemometer_direction_reset(void);
analog_wind_qf_t analog_anemometer_get_qf(void);

//#endif

#endif /* INCLUDE_DRIVERS_ANALOG_ANEMOMETER_H_ */
