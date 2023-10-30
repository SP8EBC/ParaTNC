/*
 * telemetry.h
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_TELEMETRY_H_
#define INCLUDE_APRS_TELEMETRY_H_

#include <stored_configuration_nvm/config_data.h>
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

typedef enum telemetry_descritpion_t {
	TELEMETRY_PV_PARM,
	TELEMETRY_PV_EQNS,
	TELEMETRY_PV_UNIT,
	TELEMETRY_NORMAL_PARAM,
	TELEMETRY_NORMAL_EQNS,
	TELEMETRY_NORMAL_UNIT,
	TELEMETRY_VISCOUS_PARAM,
	TELEMETRY_VISCOUS_EQNS,
	TELEMETRY_VISCOUS_UNIT,
	TELEMETRY_NOTHING
}telemetry_description_t;

extern char telemetry_qf;
extern char telemetry_degr;
extern char telemetry_nav;
extern char telemetry_pressure_qf_navaliable;
extern char telemetry_humidity_qf_navaliable;
extern char telemetry_anemometer_degradated;
extern char telemetry_anemometer_navble;
extern char telemetry_vbatt_low;
extern uint8_t telemetry_scaled_temperature;
extern uint8_t telemetry_scaled_vbatt_voltage;

void telemetry_init(void);

int telemetry_create_description_string(const config_data_basic_t * const config_basic, const telemetry_description_t what, char * out, uint16_t out_ln);

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

#ifdef PARAMETEO
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

#ifdef __cplusplus
}
#endif

uint16_t telemetry_get_counter(void);

#endif /* INCLUDE_APRS_TELEMETRY_H_ */
