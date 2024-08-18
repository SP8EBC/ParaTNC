/*
 * api.h
 *
 *  Created on: Apr 24, 2022
 *      Author: mateusz
 */

#ifndef API_H_
#define API_H_

#include "event_log.h"

void api_init(const char * api_base, const char * station_name);
void api_calculate_mac(void);
void api_send_json_status(void);
void api_send_json_measuremenets(void);
void api_send_json_event(const event_log_exposed_t * event);

#endif /* API_H_ */
