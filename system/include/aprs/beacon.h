/*
 * beacon.h
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_BEACON_H_
#define INCLUDE_APRS_BEACON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void beacon_send_own(void);
void beacon_send_on_startup(void);
void beacon_send_from_user_content(uint16_t content_ln, char* content_ptr);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_APRS_BEACON_H_ */
