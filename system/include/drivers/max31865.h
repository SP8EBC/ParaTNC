/*
 * max31865.h
 *
 *  Created on: Sep 25, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_MAX31865_H_
#define INCLUDE_DRIVERS_MAX31865_H_

#include "drivers/spi.h"

#include <stdint.h>
#include <stored_configuration_nvm/config_data.h>

#define MAX_3WIRE	3
#define MAX_4WIRE	1

typedef enum max31865_qf_t {
	MAX_QF_UNKNOWN,
	MAX_QF_FULL,
	MAX_QF_NOT_AVALIABLE
}max31865_qf_t;

extern uint8_t max31865_current_fault_status;

void max31865_init(uint8_t rdt_type, uint8_t reference_resistor_index);
void max31865_init_average(void);
void max31865_pool(void);
int32_t max31865_get_pt100_result();
int32_t max31865_get_result(uint32_t RTDnominal);
max31865_qf_t max31865_get_qf(void);

#endif /* INCLUDE_DRIVERS_MAX31865_H_ */
