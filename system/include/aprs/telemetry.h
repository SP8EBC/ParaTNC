/*
 * telemetry.h
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_TELEMETRY_H_
#define INCLUDE_APRS_TELEMETRY_H_

#include "./drivers/dallas.h"
#include "./drivers/ms5611.h"
#include "./drivers/_dht22.h"
#include "./umb_master/umb_qf_t.h"

#include "./station_config.h"

#define TELEMETRY_MIN_DALLAS	-25.0f
#define TELEMETRY_MAX_DALLAS	38.75f

#ifdef _VICTRON
#include "ve_direct_protocol/parser.h"
#endif

#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _VICTRON
void telemetry_send_values_pv (	uint8_t rx_pkts,
								uint8_t digi_pkts,
								int16_t raw_battery_current,
								uint16_t raw_battery_voltage,
								uint16_t raw_pv_cell_voltage,
								dallas_qf_t dallas_qf,
								ms5611_qf_t ms_qf,
								dht22QF ds_qf);
void telemetry_send_chns_description_pv(void);
void telemetry_send_status(ve_direct_average_struct* avg, ve_direct_error_reason* last_error, ve_direct_system_state state);

#else
void telemetry_send_values(	uint8_t rx_pkts,
							uint8_t tx_pkts,
							uint8_t digi_pkts,
							uint8_t kiss_pkts,
							float temperature,
							dallas_qf_t dallas_qf,
							ms5611_qf_t ms_qf,
							dht22QF ds_qf,
							umb_qf_t anemometer_qf);
void telemetry_send_chns_description(void);
void telemetry_send_status(void);

#endif

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_APRS_TELEMETRY_H_ */
