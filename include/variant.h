/*
 * variant.h
 *
 *  Created on: Oct 24, 2023
 *      Author: mateusz
 */

#ifndef VARIANT_H_
#define VARIANT_H_

#include "stdint.h"

int variant_validate_is_within_ram(const void * address);
int variant_validate_is_within_sram2(const void * address);
int variant_validate_is_within_flash(const void * address);
int variant_validate_is_within_flash_logger_events(const void * address);
int variant_validate_is_within_read_mem_by_addr(const void * address);

#endif /* VARIANT_H_ */
