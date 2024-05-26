/*
 * kiss_security_access.c
 *
 *  Created on: May 23, 2024
 *      Author: mateusz
 */

#include "kiss_communication/kiss_security_access.h"
#include "kiss_communication/types/kiss_communication_service_ids.h"
#include "backup_registers.h"
#include "system_stm32l4xx.h"

#include "memory_map.h"
#include "variant.h"

#define KISS_SECURITY_READ_DID						(1 << 0)
#define KISS_SECURITY_READ_MEM_RESTRICTED			(1 << 1)
#define KISS_SECURITY_READ_MEM_UNRESTRICTED			(1 << 2)
#define KISS_SECURITY_RESTART_RESET					(1 << 3)
#define KISS_SECURITY_CONFIG_RESET					(1 << 4)
#define KISS_SECURITY_WRITE_MEM						(1 << 5)
#define KISS_SECURITY_ERASE_PROGRAM_STARTUP_CONF	(1 << 6)
#define KISS_SECURITY_GET_RUNNING_CONF				(1 << 7)
#define KISS_SECURITY_FILE_DATA_TRANSFER			(1 << 8)
#define KISS_SECURITY_ENCRYPTED_DEFAULT_UNLOCK		(1 << 30)
#define KISS_SECURITY_ENCRYPTED_DEFAULT_UNLOCK_NEG	(1 << 31)

#define KISS_SECURITY_CHECK_SERIAL(x)	((kiss_security_access_config & (x)) 		!= 0) 	? 1 : 0
#define KISS_SECURITY_CHECK_MESSAGE(x)	((kiss_security_access_config & (x << 16)) 	!= 0) 	? 1 : 0

/**
 * Configuration of how UDS diagnostics are secured access different
 * mediums. GET_VERSION_AND_ID and SECURITY_ACCESS are never locked.
 * If the service shall not(!!) be locked respective bit should be set to 0.
 * By default, when memory is fully erased everything is locked
 *
 * Serial Port
 *  	0 -	Read DID
 *  	1 - Read Memory by address (RAM2, RAM2_NOINIT, everything > FLASH)
 *  	2 - Read Memory by address (without limit)
 *  	3 - Restart Reset
 *  	4 - Configuration reset
 *		5 - Write memory by address
 *		6 - Erase and program startup config
 *		7 - Get running config
 *		8 - Request file transfer and transfer data
 *
 *  	13 -
 *
 * Validity bits
 * 		14
 * 		15
 * 	these bits are sum of ( (uds_diagnostics_security_access & 0x3FFF) +
 * 	(uds_diagnostics_security_access & 3FFF0000) >> 16) & 0x3. If this value
 * 	doesn't match a configuration from here is discarded completely and
 * 	default settings are applied:
 * 	1. Everything over serial port is unlocked
 * 	2. Read DID and one restart per day
 *
 * APRS Message (Radio network or APRS-IS server)
 *
 *  	16 -
 *  	17 -
 *
 *		29 -
 *
 * Unlock all services by default when accessed via APRSMSG_TRANSPORT_ENCRYPTED_HEXSTRING
 *		30 - this should be zero to enable
 *		31 - this should be one to enable
 */
static uint32_t kiss_security_access_config = 0U;

/**
 * Set to one if configuration allow APRSMSG_TRANSPORT_ENCRYPTED_HEXSTRING
 * to be unlocked by default
 */
static uint8_t kiss_security_access_encrypted_unclock = 0U;

/**
 * Initializes security acees subsystem with current configuration
 * @param config
 */
void kiss_security_access_init(config_data_basic_t * config) {
	kiss_security_access_config = config->uds_diagnostics_security_access;

	if ((kiss_security_access_config & KISS_SECURITY_ENCRYPTED_DEFAULT_UNLOCK) != 0 &&
			(kiss_security_access_config & KISS_SECURITY_ENCRYPTED_DEFAULT_UNLOCK_NEG) == 0	) {

		// every message sent via encrypted HEX string
		kiss_security_access_encrypted_unclock = 1;
	}
}

/**
 * Checks if given diagnostics service ID, received through given transport media could be
 * currently used as-is or it required security access to be unlocked.
 * @param service_id identyfier as specified in kiss_communication_service_ids.h
 * @param transport_media how this request was received
 * @param lparam optional, per service specific parameter used for verification.
 * @return zero if service doesn't require unlocking and can be executed right now.
 */
uint8_t kiss_security_check_service_req_unlocking(uint8_t service_id, kiss_communication_transport_t transport_media, uint32_t lparam) {

	uint8_t out = 1;

	if (transport_media == KISS_TRANSPORT_ENCRYPTED_HEXSTRING && kiss_security_access_encrypted_unclock == 1) {
			out = 0;
	}
	else if ((transport_media == KISS_TRANSPORT_ENCRYPTED_HEXSTRING && kiss_security_access_encrypted_unclock == 0) ||
			transport_media == KISS_TRANSPORT_HEXSTRING) {
		switch (service_id) {
			case KISS_RESTART:
				if (lparam == KISS_RESET_HARD) {
					if (system_get_rtc_date() != backup_reg_get_last_restart_date()) {
						// one restart per day is always allowed
						out = 0;
					}
					else {
						out = KISS_SECURITY_CHECK_MESSAGE(KISS_RESTART);
					}
				}
				break;
			case KISS_GET_VERSION_AND_ID:
			case KISS_SECURITY_ACCESS:
				// these two are always allowed to be used
				out = 0;
				break;
			case KISS_READ_DID:
				out = KISS_SECURITY_CHECK_MESSAGE(KISS_READ_DID);
				break;
			case KISS_READ_MEM_ADDR:
				if (variant_validate_is_within_flash_logger_events((void*)lparam) == 1) {
					out = KISS_SECURITY_CHECK_MESSAGE(KISS_SECURITY_READ_MEM_RESTRICTED);
				}
				else {
					// it is assumed that DID handler checks if address to be read
					// is located within area
					out = KISS_SECURITY_CHECK_MESSAGE(KISS_SECURITY_READ_MEM_UNRESTRICTED);
				}
				break;
			case KISS_ERASE_STARTUP_CFG:
			case KISS_PROGRAM_STARTUP_CFG:
				out = KISS_SECURITY_CHECK_MESSAGE(KISS_SECURITY_ERASE_PROGRAM_STARTUP_CONF);
				break;
			case KISS_GET_RUNNING_CONFIG:
				out = KISS_SECURITY_CHECK_MESSAGE(KISS_SECURITY_GET_RUNNING_CONF);
				break;
			default:
				break;
			}

	}
	else if (transport_media == KISS_TRANSPORT_SERIAL_PORT) {
		switch (service_id) {
			case KISS_RESTART:
				if (lparam == KISS_RESET_HARD) {
					if (system_get_rtc_date() != backup_reg_get_last_restart_date()) {
						// one restart per day is always allowed
						out = 0;
					}
					else {
						out = KISS_SECURITY_CHECK_SERIAL(KISS_RESTART);
					}
				}
				break;
			case KISS_GET_VERSION_AND_ID:
			case KISS_SECURITY_ACCESS:
				// these two are always allowed to be used
				out = 0;
				break;
			case KISS_READ_DID:
				out = KISS_SECURITY_CHECK_SERIAL(KISS_READ_DID);
				break;
			case KISS_READ_MEM_ADDR:
				if (variant_validate_is_within_flash_logger_events((void*)lparam) == 1) {
					out = KISS_SECURITY_CHECK_SERIAL(KISS_SECURITY_READ_MEM_RESTRICTED);
				}
				else {
					// it is assumed that DID handler checks if address to be read
					// is located within area
					out = KISS_SECURITY_CHECK_SERIAL(KISS_SECURITY_READ_MEM_UNRESTRICTED);
				}
				break;
			case KISS_ERASE_STARTUP_CFG:
			case KISS_PROGRAM_STARTUP_CFG:
				out = KISS_SECURITY_CHECK_SERIAL(KISS_SECURITY_ERASE_PROGRAM_STARTUP_CONF);
				break;
			case KISS_GET_RUNNING_CONFIG:
				out = KISS_SECURITY_CHECK_SERIAL(KISS_SECURITY_GET_RUNNING_CONF);
				break;
			default:
				break;
			}

	}

	return out;

}
