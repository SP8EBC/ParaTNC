/*
 * config.h
 *
 *  Created on: 03.07.2017
 *      Author: mateusz
 */

#ifndef STATION_CONFIG_H_
#define STATION_CONFIG_H_

// Only for debugging
//#define _DBG_TRACE

// Uncomment to enable all meteo functionality. TX20 anemometer, dallas termometer, MS5611 pressure sens
//#define _METEO
#define _DIGI		// Comment this do disable WIDE1-1 digipeating

// Coordines should be in APRS decimal format DDDMM.SS for Longitude and DDMM.SS for latitude
#define _CALL "NOCALL"
#define _SSID 12
#define _LAT		5000.00
#define _LATNS		'N'
#define _LON		02200.00
#define _LONWE		'E'
#define _COMMENT	"ParaTNC v1.0-19082017 by Mateusz SP8EBC"

// You can use only one of these below defines to choose symbol. Meteo data are are always transmitted with blue WX symbol
//#define _SYMBOL_DIGI			// uncomment if you want digi symbol(green star with D inside)
#define _SYMBOL_WIDE1_DIGI	// uncomment if you want 'little' digi symbol (green star with digit 1 overlaid)
//#define _SYMBOL_HOUSE			// uncomment if you want house symbol
//#define _SYMBOL_RXIGATE		// uncomment if you want rxigate symbol (black diamond with R)
//#define _SYMBOL_IGATE			// uncomment if you want igate symol (black diamond with I)

// Or you can keep commented all symbol defines and choose custom one based on data from APRS symbols table
//#define _SYMBOL_F	'/'
//#define _SYMBOL_S	'#'

// Uncomment one of these two defines to choose what path You want. If you uncommend both of them or
// if you keep both commended path will be completely disabled. CALL-S>AKLPRZ:data
//#define _WIDE1_PATH		// CALL-S>AKLPRZ,WIDE1-1:data
#define _WIDE21_PATH	// CALL-S>AKLPRZ,WIDE2-1:data

// Comment this to disable beacon auto sending during startup (this can be risky if RF feedback occur)
#define _BCN_ON_STARTUP

#define _WX_INTERVAL 3		// WX packet interval in minutes
#define _BCN_INTERVAL 10	// Own beacon interval in minutes

//#define _PTT_PUSHPULL // Uncomment this if you want PTT line to work as Push-pull instead of Open Drain

// Transmitting delay
#define _DELAY_BASE 16	// * 50ms. For example setting 10 gives 500msec delay. Maximum value is 16
#define _RANDOM_DELAY	// adds random delay TO fixed time set by _DELAY_BASE. This additional time can be
						// from 100ms up to 1 sec in 100ms steps. Values are drawn from samples going from ADC
						// so it is better to use Unsquelched output in radio to provide much more randomness
//After waiting time declared above ParaTNC will check DCD (Data Carrier Detect) flag, which works as some
//kind of semaphore. If radio channel is not occupied by any other transmission TX will be keyed up immediately,
//otherwise software will wait for clear conditions.



// Do not touch this
#if defined (_SYMBOL_DIGI) && !defined (_SYMBOL_WIDE1_DIGI) && !defined (_SYMBOL_HOUSE) && !defined (_SYMOL_RXIGATE) &&\
	!defined (_SYMBOL_IGATE)
#define _SYMBOL_F	'/'
#define _SYMBOL_S	'#'
#elif !defined (_SYMBOL_DIGI) && defined (_SYMBOL_WIDE1_DIGI) && !defined (_SYMBOL_HOUSE) && !defined (_SYMOL_RXIGATE) &&\
	!defined (_SYMBOL_IGATE)
#define _SYMBOL_F	'1'
#define _SYMBOL_S	'#'
#elif !defined (_SYMBOL_DIGI) && !defined (_SYMBOL_WIDE1_DIGI) && defined (_SYMBOL_HOUSE) && !defined (_SYMOL_RXIGATE) &&\
	!defined (_SYMBOL_IGATE)
#define _SYMBOL_F	'/'
#define _SYMBOL_S	'-'
#elif !defined (_SYMBOL_DIGI) && !defined (_SYMBOL_WIDE1_DIGI) && !defined (_SYMBOL_HOUSE) && defined (_SYMOL_RXIGATE) &&\
	!defined (_SYMBOL_IGATE)
#define _SYMBOL_F	'I'
#define _SYMBOL_S	'&'
#elif !defined (_SYMBOL_DIGI) && !defined (_SYMBOL_WIDE1_DIGI) && !defined (_SYMBOL_HOUSE) && !defined (_SYMOL_RXIGATE) &&\
	defined (_SYMBOL_IGATE)
#define _SYMBOL_F	'R'
#define _SYMBOL_S	'&'
#elif !defined (_SYMBOL_F) && !defined (_SYMBOL_S)
#error "Missing symbol configuration in station_config.h"
#elif defined (_SYMBOL_F) && defined (_SYMBOL_S)
#else
#error "Wrong symbol configuration in station_config.h"
#endif
#if defined (_METEO) && !defined (_DIGI)
#define _DIGI
#endif



#endif /* STATION_CONFIG_H_ */
