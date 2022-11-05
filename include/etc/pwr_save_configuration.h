/*
 * pwr_save_configuration.h
 *
 *  Created on: Apr 6, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_PWR_SAVE_CONFIGURATION_H_
#define INCLUDE_PWR_SAVE_CONFIGURATION_H_



/**
 * This is cutoff voltage at which the power saving subsystem will keep ParaMETEO constantly
 * in L7 mode and wakeup once every 20 minutes to check B+ once again
 */
#define PWR_SAVE_CUTOFF_VOLTAGE_DEF 			1100u		// 11.0V

/**
 * This is the restore voltage a battery must be charged to for ParaMETEO to restore it's normal operation
 */
#define PWR_SAVE_STARTUP_RESTORE_VOLTAGE_DEF 	1230u		// 12.3V

/**
 * This is voltage above which controller will switch to PWSAVE_AGGRESV
 */
#define PWR_SAVE_AGGRESIVE_POWERSAVE_VOLTAGE 	1150u		// 11.5V

/**
 * How long in minutes the controller will sleep in L7 state between checking
 * if battery has been charged.
 */
#define PWR_SAVE_CUTOFF_SLEEP_TIME_IN_MINUTES	10

/**
 * Do not uncomment this on production devices
 */
#define INHIBIT_CUTOFF

#endif /* INCLUDE_PWR_SAVE_CONFIGURATION_H_ */
