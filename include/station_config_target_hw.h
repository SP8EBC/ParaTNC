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
//#define PARATNC_HWREV_C
#ifndef PARAMETEO
#define PARAMETEO
#endif

#ifdef PARAMETEO
//	// those defines and an undef are only required for shitty Eclipse indexer to see anything from STM32L471xx target
	#undef STM32F10X_MD_VL
	#ifndef STM32L471xx
		#define STM32L471xx
	#endif
	#ifndef USE_FULL_LL_DRIVER
		#define USE_FULL_LL_DRIVER
	#endif
#endif

#endif /* STATION_CONFIG_TARGET_HW_H_ */
