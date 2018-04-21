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
#define _METEO
//#define _DIGI		// Comment this do disable WIDE1-1 digipeating

#define _MUTE_RF	// TODO: Not yet implemented - This will make station RXonly and disable all data transmission
#define _MUTE_OWN	// TODO: Not yet implemented - This will disable all self-generated packets (wx, telemetry, beacon)
					// and switch device to "pure" kiss TNC operation. Packets from PC will be transmitted normally.

// Coordines should be in APRS decimal format DDDMM.SS for Longitude and DDMM.SS for latitude
#define _CALL "NOCALL"
#define _SSID 12
#define _LAT		5000.00
#define _LATNS		'N'
#define _LON		02000.00
#define _LONWE		'E'
#define _COMMENT	"ParaTNC v1.0.1-05092017 by Mateusz SP8EBC"

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
//#define _BCN_ON_STARTUP

#define _WX_INTERVAL 4		// WX packet interval in minutes
#define _BCN_INTERVAL 10	// Own beacon interval in minutes

//#define _PTT_PUSHPULL // Uncomment this if you want PTT line to work as Push-pull instead of Open Drain
#define _SERIAL_BAUDRATE 19200

// Transmitting delay
#define _DELAY_BASE 12	// * 50ms. For example setting 10 gives 500msec delay. Maximum value is 16
//#define _RANDOM_DELAY	// adds random delay TO fixed time set by _DELAY_BASE. This additional time can be
						// from 100ms up to 1 sec in 100ms steps. Values are drawn from samples going from ADC
						// so it is better to use Unsquelched output in radio to provide much more randomness
//After waiting time declared above ParaTNC will check DCD (Data Carrier Detect) flag, which works as some
//kind of semaphore. If radio channel is not occupied by any other transmission TX will be keyed up immediately,
//otherwise software will wait for clear conditions.

// Few IMPORTANT hints about setting transmit delay properly.
//
// Transmit delay is key parameter to maintain RF network free from packet losses and collisions. If your station will be
// installed on tall object, without any other digi's close to it, you can set _DELAY_BASE to very low value and disable
// _RANDOM_DELAY. If you wanna rather auxiliary station, witch should only fill gap in RF coverage in small area, then
// _DELAY_BASE parameter should be not less than 12 (600msec), the smallest range the higher _DELAY_BASE should be.
// Additionally for gapfillers (auxiliary stations) _RANDOM_DELAY schould be enabled.
//
// This delay will ensure that while other station will be transmitting repeated packets from mobile, Yours will keep
// always quiet and won't jam RF network. This greatly improve DCD based access to channel. Various controllers uses
// various lenght of preamble, some of them produce signal which might be impossible to decode by ParaTNC, so DCD
// is only one part of effective multiaccess to medium.


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
