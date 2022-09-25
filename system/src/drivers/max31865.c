/*
 * max31865.c
 *
 *  Created on: Sep 25, 2022
 *      Author: mateusz
 */

#include "drivers/max31865.h"

/**
 * 1 - bias on
 * 0 - bias on
 */
uint8_t max31865_vbias = 0;

/**
 *	1 - Auto (continous)
 *	0 - Off (single - shot)
 */
uint8_t max31865_conversion_mode = 0;

/**
 *
 */
uint8_t max31865_start_singleshot = 0;

/**
 * 1 - 3wire
 * 0 - 2 wire or 4 wire
 */
uint8_t max31865_rdt_sensor_type = 0;

/**
 *
 */
uint8_t max31865_fault_detection_config = 0;

/**
 * Set to one to clear
 */
uint8_t max31865_fault_clear = 0;

/**
 * 1 - 50Hz
 * 0 - 60Hz
 */
uint8_t max31865_filter_select = 0;

uint8_t max31865_buffer[3] = {0u};

uint8_t max31865_ok = 0;

/**
 * Function generates a content of configuration register basing on
 */
static uint8_t max31865_get_config_register(void) {

	uint8_t out = 0;

	out |= (max31865_filter_select & 0x01);
	out |= ((max31865_fault_clear & 0x01) << 1);
	out |= ((max31865_fault_detection_config & 0x03) << 2);
	out |= ((max31865_rdt_sensor_type & 0x01) << 4);
	out |= ((max31865_start_singleshot & 0x01) << 5);
	out |= ((max31865_conversion_mode & 0x01) << 6);
	out |= ((max31865_vbias & 0x01) << 7);

	return out;
}

void max31865_init(uint8_t rdt_type) {

	uint8_t bytes[2];

	if (rdt_type == MAX_3WIRE) {
		max31865_rdt_sensor_type = 1;
	}
	else {
		max31865_rdt_sensor_type = 0;
	}

	// set filter to 50Hz
	max31865_filter_select = 1;

	max31865_vbias = 1;

	bytes[0] = 0x80;
	bytes[1] = max31865_get_config_register();

	spi_tx_data(1, bytes, 2);

	spi_wait_for_comms_done();

	// read adres of configuation register
	bytes[0] = 0x00;
	bytes[1] = 0x00;

	// read data for verifiaction
	spi_rx_tx_data(1, max31865_buffer, bytes, 1, 1);

	spi_wait_for_comms_done();

	if (max31865_buffer[0] == max31865_get_config_register()) {
		max31865_ok = 1;
	}

}

void max31865_start_measurement(void) {

}

int32_t max31865_get_result(max31865_qf_t * quality_factor) {

}
