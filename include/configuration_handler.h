/*
 * configuration_handler.h
 *
 *  Created on: Apr 28, 2021
 *      Author: mateusz
 */

#ifndef CONFIGURATION_HANDLER_H_
#define CONFIGURATION_HANDLER_H_

#include <stdint.h>

typedef enum configuration_handler_region_t {
	REGION_DEFAULT,
	REGION_FIRST,
	REGION_SECOND
} configuration_handler_region_t;

uint32_t configuration_handler_check_crc(void);
uint32_t configuration_handler_restore_default_first(void);
uint32_t configuration_handler_restore_default_second(void);
void configuration_handler_load_configuration(configuration_handler_region_t region);
uint32_t configuration_handler_program(uint8_t* data, uint16_t data_ln, uint8_t config_idx);

uint32_t configuration_get_register(void);
void configuration_set_register(uint32_t value);
void configuration_set_bits_register(uint32_t value);
void configuration_clear_bits_register(uint32_t value);

#endif /* CONFIGURATION_HANDLER_H_ */
