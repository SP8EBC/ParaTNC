/*
 * digi.h
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_DIGI_H_
#define INCLUDE_APRS_DIGI_H_

#include <configuration_nvm/config_data.h>
#include "aprs/ax25.h"

#define DIGI_PACKET_DIGIPEATED 1
#define DIGI_PACKET_DIDNT_DIGIPEATED 0
#define DIGI_PACKET_TOO_LONG -1

typedef enum digi_mode_t {
	DIGI_OFF = 0,
	DIGI_ON_ALL_WIDE1 = 1,
	DIGI_ON_SSID_WIDE1 = 2,
	DIGI_VISCOUS_ALL_WIDE1 = 3,
	DIGI_VISCOUS_SSID_WIDE1 = 4
}digi_mode_t;

#ifdef __cplusplus
extern "C"
{
#endif

void digi_init(const config_data_mode_t* const config_data_mode);
uint8_t digi_process(struct AX25Msg *msg, const config_data_basic_t* const config, const config_data_mode_t* const config_mode);
uint8_t digi_check_with_viscous(struct AX25Msg *msg);
uint8_t digi_pool_viscous(void);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_APRS_DIGI_H_ */
