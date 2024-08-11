/*
 *
 *  Created on: Aug 6, 2024
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800C_IMSI_H_
#define INCLUDE_GSM_SIM800C_IMSI_H_


#include <stdint.h>
#include "sim800_mcc_t.h"

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Decode provided IMSI as string, to get Country Code (MCC) and Network Code (MNC) separately
 * @param imsi_str pointer to a string with IMSI returned by GSM module
 * @param imsi_str_ln lenght of a string
 * @param out_mcc pointer to a variable where a MCC will be decoded into
 * @param out_mnc the same as above, but for MNC
 */
int sim800c_imsi_decode(char* imsi_str, uint8_t imsi_str_ln, sim800_mcc_t * out_mcc, uint8_t * out_mnc);

/**
 * Getter, which looks in @link{sim800c_imsi_preprogrammed_apns} and gets APN name
 * @param out_mcc
 * @param out_mnc
 * @return pointer to a string with APN name or null if such cannot be found
 */
const char * sim800c_get_apn(const sim800_mcc_t out_mcc, const uint8_t out_mnc);

/**
 * Getter, which looks in @link{sim800c_imsi_preprogrammed_apns} and gets username
 * @param out_mcc
 * @param out_mnc
 * @return pointer to a string with APN name or null if such cannot be found
 */
const char * sim800c_get_username(const sim800_mcc_t out_mcc, const uint8_t out_mnc);

/**
 * Getter, which looks in @link{sim800c_imsi_preprogrammed_apns} and gets password
 * @param out_mcc
 * @param out_mnc
 * @return pointer to a string with APN name or null if such cannot be found
 */
const char * sim800c_get_password(const sim800_mcc_t out_mcc, const uint8_t out_mnc);


#endif /* INCLUDE_GSM_SIM800C_IMSI_H_ */
