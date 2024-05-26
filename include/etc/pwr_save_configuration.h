/*
 * pwr_save_configuration.h
 *
 *  Created on: Apr 6, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_PWR_SAVE_CONFIGURATION_H_
#define INCLUDE_PWR_SAVE_CONFIGURATION_H_



/**
 * This is cutoff voltage (momentary / non averaged) at which the power saving
 * subsystem will keep ParaMETEO constantly in L7 mode and wakeup once
 * every PWR_SAVE_CUTOFF_SLEEP_TIME_IN_MINUTES minutes to check B+ once again
 */
#define PWR_SAVE_CUTOFF_VOLTAGE_DEF 			1100u		// 11.0V

/**
 * This is the restore voltage (averaged) a battery must be charged to
 * for ParaMETEO to restore it's normal operation
 */
#define PWR_SAVE_STARTUP_RESTORE_VOLTAGE_DEF 	1200u		// 12.0V

/**
 * This is voltage above which controller will switch to PWSAVE_AGGRESV
 */
#define PWR_SAVE_AGGRESIVE_POWERSAVE_VOLTAGE 	1130u		// 11.3V

/**
 * How much average battery voltage must be greater than
 * current (momentary) voltage to go to cutoff WHEN
 * this current (momentary) voltage is below
 * PWR_SAVE_CUTOFF_VOLTAGE_DEF
 */
#define PWR_SAVE_CUTOFF_AVG_VOLTAGE_MARGIN		100u		// 0.1V

/**
 * How long in minutes the controller will sleep in L7 state between checking
 * if battery has been charged.
 */
#define PWR_SAVE_CUTOFF_SLEEP_TIME_IN_MINUTES	10

/**
 * Do not uncomment this on production devices
 */
//#define INHIBIT_CUTOFF

/**
 * Intermediate STOP2 cycle lenght within L7 or L6 mode.
 */
#define PWR_SAVE_STOP2_CYCLE_LENGHT_SEC			30u

#endif /* INCLUDE_PWR_SAVE_CONFIGURATION_H_ */
