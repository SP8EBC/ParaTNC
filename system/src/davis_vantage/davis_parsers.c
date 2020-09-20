/*
 * davis_parsers.c
 *
 *  Created on: 10.08.2020
 *      Author: mateusz
 */

#include "davis_vantage/davis_parsers.h"

#include <string.h>
#include <stdio.h>

#define DAVIS_QUERY_ABOUT_LOOP2

const uint16_t crc_table [] = {

	0x0,	0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x210, 	0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x420, 	0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x630, 	0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x840, 	0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0xa50, 	0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0xc60, 	0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0xe70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0xa1, 	0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x2b1, 	0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x481, 	0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x691, 	0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x8e1, 	0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0xaf1,  0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0xcc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0xed1, 0x1ef0
};

#define LOOP_PACAKET_LN 99
#define RX_CHECK_MIN_LN_WITH_ACK	16

#define LOOP_CRC_OFFSET 				0x61
#define LOOP_BAROMETER_OFFSET 			7
#define LOOP_INSIDE_TEMPERATURE_OFFSET	9
#define LOOP_OUTSIDE_TEMPERATURE_OFFSET		12
#define LOOP_WINDSPEED_OFFSET			14
#define LOOP_TEN_MIN_WINDSPEED_OFFSET	15
#define LOOP_WIND_DIRECTION_OFFSET		16
#define LOOP_OUTSIDE_HUMIDITY			33
#define LOOP_DAY_RAIN_OFFSET			50

#define LOOP2_CRC_OFFSET					0x61
#define LOOP2_WIND_DIRECTION				16
#define LOOP2_TEN_MINUTES_AVG_WINDSPEED		18
#define LOOP2_TWO_MINUTES_AVG_WINDSPEED		20
#define LOOP2_TEN_MINUTES_GUST				22

uint32_t davis_parsers_loop(uint8_t* input, uint16_t input_ln, davis_loop_t* output) {

	uint32_t retval = DAVIS_PARSERS_OK;

#ifdef DAVIS_QUERY_ABOUT_LOOP2
	retval = davis_parsers_loop2(input, input_ln, output);
#else
	// crc stored in the frame
	uint16_t crc_from_frame = 0;

	// calculated crc checksum
	uint16_t calculated_crc = 0;

	// check if input buffer consist at least as many characters as the LOOP packet has
	if (input_ln < LOOP_PACAKET_LN) {
		// return with an error state, don't consider consecutive content of a frame
		// as it is for sure incorrect
		retval = DAVIS_PARSERS_TOO_SHORT_FRAME;
	}
	else {
		// check if first character in the buffer is ACK (0x06) send for the LOOP packet
		if (*input == 0x06) {
			// if yes rewind the buffer to the next character
			input += 1;
		}
		else if (*input == 'L') {
			retval = DAVIS_PARSERS_OK;
		}
		else {
			// correct LOOP packet shall start from either 0x06 (if it is glued
			// to ACK) or 'L'. If it starts from something else it means that
			retval = DAVIS_PARSERS_WRONG_CONTENT;
		}

		if (retval == DAVIS_PARSERS_OK) {
			// retrieve the CRC value from the frame content (sent in MSB, totally opposite from the rest of the frame)
			crc_from_frame = *(input + LOOP_CRC_OFFSET + 1) | (*(input + LOOP_CRC_OFFSET)) << 8;

			// calculate the CRC locally excluding the last 2 bytes which consists the CRC value.
			// the another way of performing this calculation is to include the last 2 bytes (CRC itself)
			// and check if the result equals '0' what indicates that data is not corrupted.
			calculated_crc = davis_parsers_check_crc(input, LOOP_PACAKET_LN - 2);

			// check if calculated CRC is the same as recevied from the sation
			if (calculated_crc == crc_from_frame) {
				// continue only if both CRC matches

				// fetch the pressure
				output->barometer = *(input + LOOP_BAROMETER_OFFSET) | *(input + LOOP_BAROMETER_OFFSET + 1) << 8;

				output->inside_temperature = *(input + LOOP_INSIDE_TEMPERATURE_OFFSET) | *(input + LOOP_INSIDE_TEMPERATURE_OFFSET + 1) << 8;

				output->outside_temperature = *(input + LOOP_OUTSIDE_TEMPERATURE_OFFSET) | *(input + LOOP_OUTSIDE_TEMPERATURE_OFFSET + 1) << 8;

				output->wind_speed = *(input + LOOP_WINDSPEED_OFFSET);

				output->wind_speed_10min_average = *(input + LOOP_TEN_MIN_WINDSPEED_OFFSET);

				output->wind_direction = *(input + LOOP_WIND_DIRECTION_OFFSET) | *(input + LOOP_WIND_DIRECTION_OFFSET + 1) << 8;

				output->outside_humidity = *(input + LOOP_OUTSIDE_HUMIDITY);

				output->day_rain = *(input + LOOP_DAY_RAIN_OFFSET);

			}
			else {
				// if CRCs differs treat the frame as corrupted and
				retval = DAVIS_PARSERS_CORRUPTED_CRC;
			}
		}
	}
#endif



	return retval;
}

uint32_t davis_parsers_loop2(uint8_t* input, uint16_t input_ln, davis_loop_t* output) {

	uint32_t retval = DAVIS_PARSERS_OK;

	// crc stored in the frame
	uint16_t crc_from_frame = 0;

	// calculated crc checksum
	uint16_t calculated_crc = 0;

	// check if input buffer consist at least as many characters as the LOOP packet has
	if (input_ln < LOOP_PACAKET_LN) {
		// return with an error state, don't consider consecutive content of a frame
		// as it is for sure incorrect
		retval = DAVIS_PARSERS_TOO_SHORT_FRAME;
	}
	else {

		if (*input == 0x06 && *(input + 1) == 'L') {
			// if yes rewind the buffer to the next character
			input += 1;

			retval = DAVIS_PARSERS_OK;
		}
		else if (*input == 'L') {
			retval = DAVIS_PARSERS_OK;
		}
		else {
			retval = DAVIS_PARSERS_WRONG_CONTENT;
		}

		if (retval == DAVIS_PARSERS_OK) {

			// retrieve the CRC value from the frame content (sent in MSB, totally opposite from the rest of the frame)
			crc_from_frame = *(input + LOOP_CRC_OFFSET + 1) | (*(input + LOOP_CRC_OFFSET)) << 8;

			// calculate the CRC locally excluding the last 2 bytes which consists the CRC value.
			// the another way of performing this calculation is to include the last 2 bytes (CRC itself)
			// and check if the result equals '0' what indicates that data is not corrupted.
			calculated_crc = davis_parsers_check_crc(input, LOOP_PACAKET_LN - 2);

			if (calculated_crc == crc_from_frame) {

				// fetch the pressure
				output->barometer = *(input + LOOP_BAROMETER_OFFSET) | *(input + LOOP_BAROMETER_OFFSET + 1) << 8;

				output->inside_temperature = *(input + LOOP_INSIDE_TEMPERATURE_OFFSET) | *(input + LOOP_INSIDE_TEMPERATURE_OFFSET + 1) << 8;

				output->outside_temperature = *(input + LOOP_OUTSIDE_TEMPERATURE_OFFSET) | *(input + LOOP_OUTSIDE_TEMPERATURE_OFFSET + 1) << 8;

				output->wind_speed = *(input + LOOP2_TWO_MINUTES_AVG_WINDSPEED) | *(input + LOOP2_TWO_MINUTES_AVG_WINDSPEED + 1) << 8;

				output->wind_speed_10min_average = *(input + LOOP2_TEN_MINUTES_AVG_WINDSPEED) | *(input + LOOP2_TEN_MINUTES_AVG_WINDSPEED + 1) << 8;

				output->wind_gusts_10min = *(input + LOOP2_TEN_MINUTES_GUST) | *(input + LOOP2_TEN_MINUTES_GUST + 1) << 8;

				output->wind_direction = *(input + LOOP_WIND_DIRECTION_OFFSET) | *(input + LOOP_WIND_DIRECTION_OFFSET + 1) << 8;

				output->outside_humidity = *(input + LOOP_OUTSIDE_HUMIDITY);

				output->day_rain = *(input + LOOP_DAY_RAIN_OFFSET);
			}
			else {
				// if CRCs differs treat the frame as corrupted and
				retval = DAVIS_PARSERS_CORRUPTED_CRC;
			}
		}
	}

	return retval;
}


uint32_t davis_parsers_check_crc(uint8_t* input, uint16_t input_ln) {

	//uint32_t retval = 0;

	uint16_t crc = 0;

	for (int i = 0; i < input_ln; i++) {
		crc = crc_table [(crc >> 8) ^ *(input + i)] ^ (crc << 8);
	}

	return crc;
}

uint32_t davis_parsers_rxcheck(	uint8_t* input,
									uint16_t input_ln,
									uint16_t* total_packet_received,
									uint16_t* total_packet_missed,
									uint16_t* resynchronizations,
									uint16_t* packets_in_the_row,
									uint16_t* crc_errors)
{
	// 	Default:0x2000030c <srl_usart1_rx_buffer> "\n\rOK\n\r0 24840 61 0 0\n\r"
	uint32_t retval = DAVIS_PARSERS_OK;

	int i = 0;

	int itnermediate_val = 0;

	// return value of sscanf
	int position = 0;

	volatile char * chr_ptr = 0;

	// check if a string given as an input begins with a newline
	if ((*input = '\n') && (*(input + 1) == '\r')) {

		// check the lenght of an input string
		if (strnlen(input, input_ln) > RX_CHECK_MIN_LN_WITH_ACK) {
			// if it seems that this is valid RX check frame FF it
			// to the first ASCII digit
			for (i = 0; i < input_ln; i++) {
				if (is_digit(*(input +i)) != 0) {
					break;
				}
			}

			// check if the fast forward loop reach the end of an string
			if (i < input_ln) {
				// if not try to get all values

				chr_ptr = strtok(input + i, " ");

				// zero the 'i' value which now be used to count elements in input string
				i = 0;

				// iterate through all subsequent elements
				while (chr_ptr != 0) {

					// convert from string to integer
					position = sscanf(chr_ptr, "%d", &itnermediate_val);

					chr_ptr = strtok(0, " ");

					switch (i) {
						case 0:
							*total_packet_received = (uint16_t)itnermediate_val;
							break;
						case 1:
							*total_packet_missed = (uint16_t)itnermediate_val;
							break;
						case 2:
							*resynchronizations = (uint16_t)itnermediate_val;
							break;
						case 3:
							*packets_in_the_row = (uint16_t)itnermediate_val;
							break;
						case 4:
							*crc_errors = (uint16_t)itnermediate_val;
							break;
						default: break;
					}

					// increment the elements counter
					i++;

					// exit if we reach the last element from the string
					if (i > 4)
						break;

				}

			}
			else {
				// if yes this isn't a valid
				retval = DAVIS_PARSERS_WRONG_CONTENT;
			}

		}
		else {
			retval = DAVIS_PARSERS_WRONG_CONTENT;
		}
	}
	else {

	}

	return retval;

}
