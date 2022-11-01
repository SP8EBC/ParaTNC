/*
 * max31865.c
 *
 *  Created on: Sep 25, 2022
 *      Author: mateusz
 */

#include "rte_wx.h"
#include "drivers/max31865.h"
#include <math.h>

#define DATA_RACE_WORKAROUND

#define REFERENCE_RESISTOR 4300.0f

#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7

int32_t test;

typedef enum max31865_pool_state_t {
	MAX_UNINITIALIZED,
	MAX_IDLE,
	MAX_INITIALIZED,
	MAX_ERROR,
	MAX_MEASUREMENT_STARTED,
	MAX_REGISTER_REQUESTED,
	MAX_SHUTDOWN,
	MAX_POWER_OFF
}max31865_pool_state_t;

static const float max31865_rref_lookup_table[32] =
{
		430.0f,
		432.0f,
		442.0f,
		470.0f,
		499.0f,
		510.0f,
		560.0f,
		620.0f,
		680.0f,
		768.0f,	// 9
		1000.0f,
		1100.0f,
		1200.0f,
		1300.0f,
		1400.0f,
		1500.0f,
		1600.0f,
		1740.0f,
		1800.0f,
		1910.0f,	// 19
		2000.0f,	// 20
		2100.0f,
		2400.0f,
		2700.0f,
		3000.0f,
		3090.0f,
		3400.0f,
		3900.0f,
		4300.0f,	// 28
		4700.0f,	// 29
		4990.0f,	// 30
		5600.0f		// 31

};

max31865_pool_state_t max31865_current_state = MAX_UNINITIALIZED;

/**
 *	reference resistor value
 */
float max31865_rref = 0;

/**
 * This variable is incremented from 0 up to 9 to pause measurement
 * state machine into MAX_SHUTDOWN state. When it reach 9 measurement
 * is triggered
 */
uint8_t max31865_shutdown_ticks = 0;

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
 *	Set to one and send config register to trigger single measurement
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

/**
 * Set to one if MAX has been initialized correctly
 */
uint8_t max31865_ok = 0;

/**
 * Last raw result read from MAX
 */
uint16_t max31865_raw_result = 0;

/**
 * Value of configuration register which should be currently stored in
 * amplifier
 */
uint8_t max31865_current_config_register = 0;

max31865_qf_t max31865_quality_factor = MAX_QF_UNKNOWN;

uint8_t max31865_measurements_counter = 0;

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

static void max31865_request_registers(void) {

	uint8_t result = 0;

	// check if SPI is busy now
	if (spi_get_current_slave() == 0) {
		// read adres of configuation register
		max31865_buffer[0] = 0x00;
		max31865_buffer[1] = 0x00;

		// read data for verifiaction
		result = spi_rx_tx_data(1, SPI_TX_FROM_EXTERNAL, SPI_USE_INTERNAL_RX_BUF, max31865_buffer, 10, 1);
	}
	else {
		max31865_current_state = MAX_ERROR;
	}

	if (result != SPI_OK) {
		max31865_current_state = MAX_ERROR;
	}
}

static void max31865_send_config_register(void) {
	uint8_t result = 0;

	// check if SPI is busy now
	if (spi_get_current_slave() == 0) {
		max31865_current_config_register = max31865_get_config_register();

		// read adres of configuation register
		max31865_buffer[0] = 0x80;
		max31865_buffer[1] = max31865_current_config_register;

		spi_tx_data(1, SPI_TX_FROM_EXTERNAL, max31865_buffer, 2);

	}
	else {
		max31865_current_state = MAX_ERROR;
	}

	if (result != SPI_OK) {
		max31865_current_state = MAX_ERROR;
	}
}

void max31865_init(uint8_t rdt_type, uint8_t reference_resistor) {

	uint8_t * rx_data;

	if (rdt_type == MAX_3WIRE) {
		max31865_rdt_sensor_type = 1;
	}
	else if (rdt_type == MAX_4WIRE) {
		max31865_rdt_sensor_type = 0;
	}
	else {
		max31865_current_state = MAX_UNINITIALIZED;

		return;
	}

	if (reference_resistor > 31) {
		max31865_current_state = MAX_UNINITIALIZED;

		return;
	}
	else {
		max31865_rref = max31865_rref_lookup_table[reference_resistor];
	}

	// set filter to 50Hz
	max31865_filter_select = 1;

	max31865_vbias = 0;

	max31865_conversion_mode = 0;

	max31865_send_config_register();

	spi_wait_for_comms_done();

	max31865_request_registers();

	spi_wait_for_comms_done();

	rx_data = spi_get_rx_data();

	if (rx_data[0] == max31865_get_config_register()) {
		max31865_ok = 1;

		max31865_current_state = MAX_INITIALIZED;
	}
	else {
		max31865_ok = 0;
	}

}

/**
 * This pooler shall be called in two seconds interval
 */
void max31865_pool(void) {

	uint8_t * result_ptr;

	switch (max31865_current_state) {
		case MAX_IDLE:
			// MAX31865 is powered up but not initialized
			if (max31865_rdt_sensor_type == 1) {
				max31865_init(MAX_3WIRE, max31865_rref);
			}
			else {
				max31865_init(MAX_4WIRE, max31865_rref);
			}

			if (max31865_ok == 1) {
				max31865_current_state = MAX_INITIALIZED;
			}

			break;
		case MAX_INITIALIZED:
			// initialized and ready to start measurement
			max31865_current_state = MAX_MEASUREMENT_STARTED;

			max31865_start_singleshot = 1;

			max31865_vbias = 1;

			// this function may change 'max31865_current_state' internally due to errors
			max31865_send_config_register();

			// disable VBIAS to reduce power consumption
			max31865_vbias = 0;

			max31865_start_singleshot = 0;

			break;
		case MAX_ERROR:
			// go back to idle in case of any error
			max31865_current_state = MAX_IDLE;

			max31865_quality_factor = MAX_QF_NOT_AVALIABLE;

			break;
		case MAX_MEASUREMENT_STARTED:
			// measurement has been started before, so now it's time to request for results
			max31865_request_registers();

			max31865_current_state = MAX_REGISTER_REQUESTED;

			break;
		case MAX_REGISTER_REQUESTED:
			// results shall be available
			max31865_current_state = MAX_SHUTDOWN;

			// check a SPI status
			if (spi_get_rx_state() != SPI_RX_DONE) {
				// if SPI is not done
				max31865_current_state = MAX_ERROR;
			}
			else {
				// get a pointer to results
				result_ptr = spi_get_rx_data();

#ifdef DATA_RACE_WORKAROUND
				if ((max31865_current_config_register & 0xDF) == *(result_ptr + 1)) {
					result_ptr++;
				}
#endif

				// check communication results by comparing a value of config register
				if ((max31865_current_config_register & 0xDF) == *result_ptr) {	// fifth bit read always zero
					// save raw results
					max31865_raw_result = *(result_ptr + 2) | (*(result_ptr + 1) << 8);

					max31865_raw_result = max31865_raw_result >> 1;

//					rte_wx_temperature_average_pt = max31865_get_pt100_result(0);
					if (max31865_rdt_sensor_type == 0) {
						rte_wx_temperature_average_pt = max31865_get_result(1000);
					}
					else {
						rte_wx_temperature_average_pt = max31865_get_result(100);
					}

					max31865_measurements_counter++;

					max31865_quality_factor = MAX_QF_FULL;
				}
				else {
					max31865_current_state = MAX_ERROR;

					max31865_quality_factor = MAX_QF_NOT_AVALIABLE;
				}

				// disable VBIAS to reduce power consumption
				max31865_vbias = 0;

				// this function may change 'max31865_current_state' internally due to errors
				max31865_send_config_register();

				max31865_shutdown_ticks = 0;
			}


			break;
		case MAX_SHUTDOWN:
			// MAX31865 is powered up and initialized but PT bias is disabled
			// and no measurement is ongoing
			if (max31865_shutdown_ticks++ > 9) {
				max31865_current_state = MAX_INITIALIZED;

				max31865_shutdown_ticks = 0;
			}

			break;
		case MAX_UNINITIALIZED:
		case MAX_POWER_OFF:
			// supply voltage for MAX31865 is powered off and no communication
			// is currently possible
			break;
	}
}

int32_t max31865_get_pt100_result(void) {

	int32_t temperature_scaled = 0;

	float R_ohms = (max31865_raw_result * max31865_rref) / 32768.0f;

	float num, denom, T ;

	float c0= -245.19 ;
	float c1 = 2.5293 ;
	float c2 = -0.066046 ;
	float c3 = 4.0422E-3 ;
	float c4 = -2.0697E-6 ;
	float c5 = -0.025422 ;
	float c6 = 1.6883E-3 ;
	float c7 = -1.3601E-6 ;

	num = R_ohms * (c1 + R_ohms * (c2 + R_ohms * (c3 + R_ohms * c4))) ;
	denom = 1.0 + R_ohms * (c5 + R_ohms * (c6 + R_ohms * c7)) ;
	T = c0 + (num / denom) ;

	temperature_scaled = (int32_t) (T * 10.0f);

	return temperature_scaled;
}

int32_t max31865_get_result(uint32_t RTDnominal) {

	  float Z1, Z2, Z3, Z4, Rt, temp;

	  //float R_ohms = (max31865_raw_result * REFERENCE_RESISTOR) / 32768.0f;

	  Rt = max31865_raw_result;
	  Rt /= 32768.0f;
	  Rt *= max31865_rref;

	  // Serial.print("\nResistance: "); Serial.println(Rt, 8);

	  Z1 = -RTD_A;
	  Z2 = RTD_A * RTD_A - (4 * RTD_B);
	  Z3 = (4 * RTD_B) / RTDnominal;
	  Z4 = 2 * RTD_B;

	  temp = Z2 + (Z3 * Rt);
	  temp = (sqrt(temp) + Z1) / Z4;

	  if (temp >= 0)
		  return (int32_t) (temp * 10.0f);

	  // ugh.
	  Rt /= RTDnominal;
	  Rt *= 100; // normalize to 100 ohm

	  float rpoly = Rt;

	  temp = -242.02;
	  temp += 2.2228 * rpoly;
	  rpoly *= Rt; // square
	  temp += 2.5859e-3 * rpoly;
	  rpoly *= Rt; // ^3
	  temp -= 4.8260e-6 * rpoly;
	  rpoly *= Rt; // ^4
	  temp -= 2.8183e-8 * rpoly;
	  rpoly *= Rt; // ^5
	  temp += 1.5243e-10 * rpoly;

	  return (int32_t) (temp * 10.0f);

	//return 0;
}

max31865_qf_t max31865_get_qf(void) {
	return max31865_quality_factor;
}
