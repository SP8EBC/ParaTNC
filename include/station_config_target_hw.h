/*
 * station_config_target_hw.h
 *
 *  Created on: May 30, 2021
 *      Author: mateusz
 */

#ifndef STATION_CONFIG_TARGET_HW_H_
#define STATION_CONFIG_TARGET_HW_H_

//#define PARATNC_HWREV_A
//#define PARATNC_HWREV_B
#define PARATNC_HWREV_C
//#define PARAMETEO

#ifdef PARAMETEO
	// those defines and an undef are only required for shitty Eclipse indexer to see anything from STM32L471xx target
	#define STM32L471xx
	#define USE_FULL_LL_DRIVER
	#undef STM32F10X_MD_VL
#endif

#endif /* STATION_CONFIG_TARGET_HW_H_ */
