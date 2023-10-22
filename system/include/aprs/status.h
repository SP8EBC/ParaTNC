/*
 * status.h
 *
 *  Created on: Jul 22, 2023
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_STATUS_H_
#define INCLUDE_APRS_STATUS_H_

#include <stdint.h>

void status_send(void);

void status_send_raw_values_modbus(void);

void status_send_powersave_cutoff(uint16_t battery_voltage, int8_t vbatt_low, int8_t cutoff);
void status_send_powersave_registers(void);

void status_send_gsm(void);
void status_send_aprsis_timeout(uint8_t unsuccessfull_conn_cntr);


#endif /* INCLUDE_APRS_STATUS_H_ */
