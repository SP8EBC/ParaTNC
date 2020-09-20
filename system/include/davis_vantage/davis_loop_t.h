/*
 * davis_loop_t.h
 *
 *  Created on: 10.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_DAVIS_VANTAGE_DAVIS_LOOP_T_H_
#define INCLUDE_DAVIS_VANTAGE_DAVIS_LOOP_T_H_

/**
 * This type holds information parsed from incoming LOOP packet. Due to memory constraints
 * it doesn't hold all information from original packet sent from the WX station base. It
 * is limited to those values which could be transmitted over APRS network using standard
 * weather packet.
 *
 * All fields name and types corresponds to naming scheme used in official documentation
 * provided by Davis (VantageSerialProtocol_v261.pdf)
 */
typedef struct davis_loop {
	// pressure stored as 1/1000 of mmHg. Valid ranges is between 20000 (20mmHg) and
	// 32500 (32.5mmHg)
	uint16_t barometer;

	// inside temperature stored as 1/100 of one F.
	uint16_t inside_temperature;

	// scaling the same as for inside temperature
	uint16_t outside_temperature;

	// wind speed in US land miles per hour. Value od '1' means 1mph
	uint16_t wind_speed;

	// the same scaling as for wind_speed. There is no clear explanation
	// what are the differences between wind_speed and it's 10 minutes
	// average. Presumably wind_speed is just a current value
	uint16_t wind_speed_10min_average;

	// only in loop2
	uint16_t wind_gusts_10min;

	// Wind direction in degrees. It is scaled +1 from real value.
	// Value of 0 means that no data is available
	uint16_t wind_direction;

	uint8_t outside_humidity;

	uint16_t day_rain;

	// crc is sent after newline '\n\r'
	uint16_t crc;

}davis_loop_t;

#endif /* INCLUDE_DAVIS_VANTAGE_DAVIS_LOOP_T_H_ */
