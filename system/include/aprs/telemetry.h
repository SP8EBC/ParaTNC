/*
 * telemetry.h
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_TELEMETRY_H_
#define INCLUDE_APRS_TELEMETRY_H_

#include "config_data.h"

#include "./drivers/dallas.h"
#include "./drivers/ms5611.h"
#include "./umb_master/umb_qf_t.h"
#include "./ve_direct_protocol/raw_struct.h"
#include "./ve_direct_protocol/average_struct.h"

#include "./station_config.h"

#define TELEMETRY_MIN_DALLAS	-25.0f
#define TELEMETRY_MAX_DALLAS	38.75f

#include "ve_direct_protocol/parser.h"

#include "stdint.h"

typedef enum pressure_qf {
	PRESSURE_QF_UNKNOWN = 0,
	PRESSURE_QF_FULL = 1,
	PRESSURE_QF_NOT_AVALIABLE = 2,
	PRESSURE_QF_DEGRADATED = 3
}pressure_qf_t;

typedef enum humidity_qf {
	HUMIDITY_QF_UNKNOWN = 0,
	HUMIDITY_QF_FULL = 1,
	HUMIDITY_QF_NOT_AVALIABLE = 2,
	HUMIDITY_QF_DEGRADATED = 3
}humidity_qf_t;

typedef enum wind_qf {
	WIND_QF_UNKNOWN = 0,
	WIND_QF_FULL = 1,
	WIND_QF_NOT_AVALIABLE = 2,
	WIND_QF_DEGRADATED = 3
}wind_qf_t;

#ifdef __cplusplus
extern "C"
{
#endif

void telemetry_send_values_pv (	uint8_t rx_pkts,
								uint8_t digi_pkts,
								int16_t raw_battery_current,
								uint16_t raw_battery_voltage,
								uint16_t raw_pv_cell_voltage,
								dallas_qf_t dallas_qf,
								pressure_qf_t press_qf,
								humidity_qf_t humid_qf,
								wind_qf_t anemometer_q);
void telemetry_send_chns_description_pv(const config_data_basic_t * const config_basic);
void telemetry_send_status_pv(ve_direct_average_struct* avg, ve_direct_error_reason* last_error, ve_direct_system_state state, uint32_t master_time, uint16_t messages_count, uint16_t corrupted_messages_count);

#ifdef STM32L471xx
void telemetry_send_values(		uint8_t rx_pkts,
								uint8_t tx_pkts,
								uint8_t digi_pkts,
								uint16_t vbatt_voltage,
								uint8_t viscous_drop_pkts,
								float temperature,
								dallas_qf_t dallas_qf,
								pressure_qf_t press_qf,
								humidity_qf_t humid_qf,
								wind_qf_t anemometer_qf,
								int8_t cutoff_and_vbat_low,
								const config_data_mode_t * const config_mode);
#else
void telemetry_send_values(	uint8_t rx_pkts,
							uint8_t tx_pkts,
							uint8_t digi_pkts,
							uint8_t kiss_pkts,
							uint8_t viscous_drop_pkts,
							float temperature,
							dallas_qf_t dallas_qf,
							pressure_qf_t press_qf,
							humidity_qf_t humid_qf,
							wind_qf_t anemometer_qf,
							const config_data_mode_t * const config_mode);
#endif
void telemetry_send_chns_description(const config_data_basic_t * const config_basic, const config_data_mode_t * const config_mode);
void telemetry_send_status(void);

void telemetry_send_status_raw_values_modbus(void);

void telemetry_send_status_powersave_cutoff(uint16_t battery_voltage, int8_t vbatt_low, int8_t cutoff);
void telemetry_send_status_powersave_registers(uint32_t register_last_sleep, uint32_t register_last_wakeup, uint32_t register_counters);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_APRS_TELEMETRY_H_ */
