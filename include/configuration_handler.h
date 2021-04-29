/*
 * configuration_handler.h
 *
 *  Created on: Apr 28, 2021
 *      Author: mateusz
 */

#ifndef CONFIGURATION_HANDLER_H_
#define CONFIGURATION_HANDLER_H_

#include <stdint.h>

uint32_t configuration_handler_check_crc(void);
uint32_t configuration_handler_restore_default(void);
uint32_t configuration_handler_load_configuration(void);
uint32_t configuration_handler_program(uint8_t* data, uint16_t data_ln, uint8_t config_idx);

#endif /* CONFIGURATION_HANDLER_H_ */
