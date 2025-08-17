/*
 * main_freertos_externs.h
 *
 *  Created on: Aug 17, 2025
 *      Author: mateusz
 */

#ifndef MAIN_FREERTOS_EXTERNS_H_
#define MAIN_FREERTOS_EXTERNS_H_

#include <FreeRTOS.h>
#include <event_groups.h>

//! Declare a variable to hold the handle of the created event group.
extern EventGroupHandle_t main_eventgroup_handle_powersave;


#endif /* MAIN_FREERTOS_EXTERNS_H_ */
