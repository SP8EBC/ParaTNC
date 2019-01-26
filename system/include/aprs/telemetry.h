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

#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

void telemetry_send_values(	uint8_t rx_pkts,
							uint8_t tx_pkts,
							uint8_t digi_pkts,
							uint8_t kiss_pkts,
							float temperature,
							DallasQF dallas_qf,
							ms5611_qf_t ms_qf,
							dht22QF ds_qf);
void telemetry_send_chns_description(void);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_APRS_TELEMETRY_H_ */
