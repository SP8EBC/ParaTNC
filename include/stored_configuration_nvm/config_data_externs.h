/*
 * config_data_externs.h
 *
 *  Created on: Apr 4, 2021
 *      Author: mateusz
 */

#ifndef CONFIG_DATA_EXTERNS_H_
#define CONFIG_DATA_EXTERNS_H_

#include <stored_configuration_nvm/config_data.h>

extern const int __config_section_first_crc;
extern const int __config_section_second_crc;

extern const uint16_t * config_data_pgm_cntr_first_ptr;
extern const uint16_t * config_data_pgm_cntr_second_ptr;

extern const uint16_t 	config_data_pgm_cntr_first;
extern const uint16_t  	config_data_pgm_cntr_second;

extern const config_data_mode_t * config_data_mode_first_ptr;
extern const config_data_basic_t * config_data_basic_first_ptr;
extern const config_data_wx_sources_t * config_data_wx_sources_first_ptr;
extern const config_data_umb_t * config_data_umb_first_ptr;
extern const config_data_rtu_t * config_data_rtu_first_ptr;

extern const config_data_mode_t * config_data_mode_second_ptr;
extern const config_data_basic_t * config_data_basic_second_ptr;
extern const config_data_wx_sources_t * config_data_wx_sources_second_ptr;
extern const config_data_umb_t * config_data_umb_second_ptr;
extern const config_data_rtu_t * config_data_rtu_second_ptr;

#ifdef PARAMETEO
#define config_data_pgm_cntr_first 		*(config_data_pgm_cntr_first_ptr)
#define config_data_pgm_cntr_second 	*(config_data_pgm_cntr_second_ptr)

extern const config_data_gsm_t * config_data_gsm_first_ptr;
extern const config_data_gsm_t * config_data_gsm_second_ptr;
extern const config_data_gsm_t * config_data_gsm_default_ptr;

extern const config_data_gsm_t config_data_gsm_default;

#else

extern const config_data_basic_t config_data_basic_first;
extern const config_data_mode_t config_data_mode_first;
extern const config_data_umb_t config_data_umb_first;
extern const config_data_rtu_t config_data_rtu_first;
extern const config_data_wx_sources_t config_data_wx_sources_first;

extern const config_data_basic_t config_data_basic_second;
extern const config_data_mode_t config_data_mode_second;
extern const config_data_umb_t config_data_umb_second;
extern const config_data_rtu_t config_data_rtu_second;
extern const config_data_wx_sources_t config_data_wx_sources_second;

#endif




#endif /* CONFIG_DATA_EXTERNS_H_ */
