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

extern configuration_handler_region_t configuration_handler_loaded;

uint32_t configuration_handler_check_crc(void);
uint32_t configuration_handler_restore_default_first(void);
uint32_t configuration_handler_restore_default_second(void);
void configuration_handler_load_configuration(configuration_handler_region_t region);
uint32_t configuration_handler_program(uint8_t* data, uint16_t data_ln, uint8_t config_idx);

uint32_t configuration_get_register(void);
void configuration_set_register(uint32_t value);
void configuration_set_bits_register(uint32_t value);
void configuration_clear_bits_register(uint32_t value);

int32_t configuration_kiss_parse_get_running_config(uint8_t* input_frame_from_host, uint16_t input_len);

#endif /* CONFIGURATION_HANDLER_H_ */
