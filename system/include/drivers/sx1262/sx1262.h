/*
 * sx1262.h
 *
 *  Created on: May 19, 2025
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_SX1262_H_
#define INCLUDE_DRIVERS_SX1262_SX1262_H_


void sx1262_init(void);

void sx1262_busy_released_callback(void);

void sx1262_interrupt_callback(void);

void sx1262_spi_transmission_done_callback(void);

#endif /* INCLUDE_DRIVERS_SX1262_SX1262_H_ */
