/*
 * kiss_security_access.h
 *
 *  Created on: May 23, 2024
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_KISS_SECURITY_ACCESS_H_
#define KISS_COMMUNICATION_KISS_SECURITY_ACCESS_H_

#include <stored_configuration_nvm/config_data.h>
#include "kiss_communication/types/kiss_communication_transport_t.h"

/**
 * Initializes security acees subsystem with current configuration
 * @param config
 */
void kiss_security_access_init(config_data_basic_t * config);

/**
 * Checks if given diagnostics service ID, received through given transport media could be
 * currently used as-is or it required security access to be unlocked.
 * @param service_id identyfier as specified in kiss_communication_service_ids.h
 * @param transport_media how this request was received
 * @param lparam optional, per service specific parameter used for verification.
 * @return
 */
uint8_t kiss_security_check_service_req_unlocking(uint8_t service_id, kiss_communication_transport_t transport_media, uint32_t lparam);


#endif /* KISS_COMMUNICATION_KISS_SECURITY_ACCESS_H_ */
