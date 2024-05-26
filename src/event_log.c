/*
 * event_log.c
 *
 *  Created on: May 26, 2024
 *      Author: mateusz
 */

#include "event_log.h"

/**
 * Structure to manage noinit RAM storage and FLASH storage
 */
typedef struct event_log_fifo_t {
	uint16_t 		oldest_event_index;
	uint32_t 		oldest_event_master_time;
	event_log_t * 	oldest_event_pointer;
	uint16_t 		newest_event_index;
	uint32_t 		newest_event_master_time;
	event_log_t * 	newest_event_pointer;
};

void event_log_init(uint8_t flash_enabled_severity, uint8_t ram_enabled_severity) {

}
