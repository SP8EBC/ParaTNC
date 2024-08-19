/*
 * events_main.h
 *
 *  Created on: Jul 28, 2024
 *      Author: mateusz
 */
#ifndef EVENTS_DEFINITIONS_EVENTS_MAIN_H_
#define EVENTS_DEFINITIONS_EVENTS_MAIN_H_

/**
 * 	EVENT_BOOTUP	< EVENT_BOOTUP all info events generated during bootup
 *
 *
 */
#define EVENTS_MAIN_BOOTUP_COMPLETE	0xEEU

/**
 * 	EVENT_TIMESYNC	< EVENT_TIMESYNC event generated once at startup and then every 6 hours to
					 		   keep master_time and RTC date and time sync
 *
 *
 */
#define EVENTS_MAIN_TIMESYNC_BOOTUP		0x77u
#define EVENTS_MAIN_TIMESYNC_PERIODIC	0x88u
#define EVENTS_MAIN_TIMESYNC_NTP		0x99u

#define EVENTS_MAIN_CYCLIC				0xAAu

#endif
