/*
 * sensirion_sht3x.h
 *
 *  Created on: 02.01.2018
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SENSIRION_SHT3X_H_
#define INCLUDE_DRIVERS_SENSIRION_SHT3X_H_

#include "drivers/i2c.h"
#include <stdint.h>

#define SHT3X_RADDR ((0x44 << 1) | 1 )
#define SHT3X_WADDR (0x44 << 1)

#define SHT3X_SS_LOW_NOSTREACH 0x2416

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

void sht3x_start_measurement(void);
void sht3x_read_measurements(uint16_t *temperature, uint16_t *humidity);

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DRIVERS_SENSIRION_SHT3X_H_ */
