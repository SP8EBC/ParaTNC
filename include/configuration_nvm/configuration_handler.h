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

typedef enum configuration_button_function_t {

	BUTTON_DISABLED = 0,
	BUTTON_SEND_WX = 1,
	BUTTON_SEND_WX_INTERNET = 2,
	BUTTON_SEND_BEACON = 3,
	BUTTON_FORCE_UART_KISS = 4,
	BUTTON_FORCE_UART_LOG = 5,
	BUTTON_RESET_GSM_GPRS = 6,
	BUTTON_RECONNECT_APRSIS = 7,
}configuration_button_function_t;

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
int configuration_get_early_tx_assert(void);
int configuration_get_power_cycle_vbat_r(void);
int configuration_get_reboot_after_24_hours(void);
int configuration_get_power_cycle_gsmradio_on_no_communications(void);

uint16_t configuration_get_vbat_a_coeff(void);
uint16_t configuration_get_vbat_b_coeff(void);

configuration_button_function_t configuration_get_left_button(void);
configuration_button_function_t configuration_get_right_button(void);

int configuration_get_is_security_access_required(uint8_t medium, uint8_t routine_type);


#endif /* CONFIGURATION_HANDLER_H_ */
