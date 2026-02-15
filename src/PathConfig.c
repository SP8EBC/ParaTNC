/*
 * PathConfig.c
 *
 *  Created on: 02.08.2017
 *      Author: mateusz
 */

#include "PathConfig.h"
#include "station_config.h"
#include <string.h>

uint8_t ConfigPath (AX25Call *p, const config_data_basic_t *conf)
{

	memcpy (p[0].call, "AKLPRZ", 6), p[0].ssid = 0;

	switch (conf->path_type) {
	case 1:
		strncpy (p[1].call, conf->callsign, 6), p[1].ssid = conf->ssid;
		memcpy (p[2].call, "WIDE1", 6), p[2].ssid = 1;
		return 3;
	case 2:
		strncpy (p[1].call, conf->callsign, 6), p[1].ssid = conf->ssid;
		memcpy (p[2].call, "WIDE2", 6), p[2].ssid = 1;
		return 3;
	case 0:
	default: strncpy (p[1].call, conf->callsign, 6), p[1].ssid = conf->ssid; return 2;
	}

	// #if (defined(_WIDE1_PATH) && !defined(_WIDE21_PATH))
	//	memcpy(p[1].call, _CALL, 6), p[1].ssid = _SSID;
	//	memcpy(p[2].call, "WIDE1", 6), p[2].ssid = 1;
	//	return 3;
	// #elif (!defined(_WIDE1_PATH) && defined(_WIDE21_PATH))
	//	memcpy(p[1].call, _CALL, 6), p[1].ssid = _SSID;
	//	memcpy(p[2].call, "WIDE2", 6), p[2].ssid = 1;
	//	return 3;
	// #else
	//	memcpy(p[1].call, _CALL, 6), p[1].ssid = _SSID;
	//	return 2;
	//	#endif
	//
	//	return 0;
}
