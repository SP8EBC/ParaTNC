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

typedef enum configuration_erase_startup_t {
	ERASE_STARTUP_IDLE		= 0xAA,
	ERASE_STARTUP_PENDING	= 0xAB,
	ERASE_STARTUP_DONE		= 0xAC,
	ERASE_STARTUP_ERROR		= 0xAD
}configuration_erase_startup_t;

uint32_t configuration_handler_check_crc(void);
uint32_t configuration_handler_restore_default_first(void);
uint32_t configuration_handler_restore_default_second(void);
void configuration_handler_load_configuration(configuration_handler_region_t region);
configuration_erase_startup_t configuration_handler_erase_startup(void);
configuration_erase_startup_t configuration_handler_program_startup(uint8_t * data, uint8_t dataln, uint16_t offset);

uint32_t configuration_get_register(void);
void configuration_set_register(uint32_t value);
void configuration_set_bits_register(uint32_t value);
void configuration_clear_bits_register(uint32_t value);

configuration_handler_region_t configuration_get_current(uint32_t * size);
const uint32_t * configuration_get_address(configuration_handler_region_t region);

int configuration_get_inhibit_wx_pwr_handle(void);

#endif /* CONFIGURATION_HANDLER_H_ */
