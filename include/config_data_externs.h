/*
 * config_data_externs.h
 *
 *  Created on: Apr 4, 2021
 *      Author: mateusz
 */

#ifndef CONFIG_DATA_EXTERNS_H_
#define CONFIG_DATA_EXTERNS_H_

#include "config_data.h"

extern const int __config_section_first_crc;
extern const int __config_section_second_crc;

extern const int __config_section_first_pgm_counter;
extern const int __config_section_second_pgm_counter;


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


#endif /* CONFIG_DATA_EXTERNS_H_ */
