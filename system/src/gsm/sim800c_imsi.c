/*
 * IMSI decoding stuff and a list of APN/user/pass for MCC and MNC pair
 *
 * sim800c_imsi.c
 *
 *  Created on: Aug 5, 2024
 *      Author: mateusz
 */

#include "./gsm/sim800c_imsi.h"
#include "./gsm/sim800_apn_config_t.h"
#include "preprogrammed_gprs_apn_list.h"
#include "variant.h"

#include <stdlib.h>

// clang-format off
/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================
#define SIM800C_IMSI_GENERATE_PREPROGMD_APN_LIST(_mcc, _mnc, _apn, _user, _pass) \
	{.country_code = _mcc,                                                       \
	 .mobile_network_code = _mnc,                                                \
	 .apn_name = _apn,                                                           \
	 .username = _user,                                                          \
	 .password = _pass},

#define SIM800C_IMSI_GENERATE_PREPROGMD_MCC_CODES_ENUM(_mcc, _mnc, _apn, _user, _pass) \
	SIM800C_PREPROGMD_MCC_##_mcc_##_mnc,

/// ==================================================================================================
///	LOCAL TYPEDEFS
/// ==================================================================================================

/**
 * This enum is used only to count how many entries is defined in PREPROGRAMMED_GPRS_APN_LIST
 */
typedef enum sim800c_imsi_preprogrammed_mcc_codes_t {
		PREPROGRAMMED_GPRS_APN_LIST (SIM800C_IMSI_GENERATE_PREPROGMD_MCC_CODES_ENUM)
		SIM800C_PREPROGMD_MCC_LAST
} sim800c_imsi_preprogrammed_mcc_codes_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/**
 * Global list of all preprogrammed APNs
 */
static const sim800_apn_config_t sim800c_imsi_preprogrammed_apns[] = {
	PREPROGRAMMED_GPRS_APN_LIST (SIM800C_IMSI_GENERATE_PREPROGMD_APN_LIST)};
// clang-format on
/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

static sim800_mcc_t sim800c_imsi_mcc_value_of (const char *mcc_buffer)
{

	sim800_mcc_t output = MCC_UNKNOWN;

	int mcc = atoi (mcc_buffer);

	output = (sim800_mcc_t)mcc;

	return output;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param imsi_str
 * @param imsi_str_ln
 * @param out_mcc
 * @param out_mnc
 */
int sim800c_imsi_decode (char *imsi_str, uint8_t imsi_str_ln, sim800_mcc_t *out_mcc,
						 uint8_t *out_mnc)
{

	int result = 0;

	// local buffer for MCC string
	char mcc_buffer[4] = {0u};
	char mnc_buffer[4] = {0u};

	// IMSI string could be no longer than 15 characters long. It also must contain at least
	// three digit MCC (country code) and two digit MNC (mobile network code). Then some digits
	// for sim card number itself
	if (imsi_str_ln <= 15 && imsi_str_ln > 5) {

		// if pointers are set as they should
		if (variant_validate_is_within_ram (out_mcc) && variant_validate_is_within_ram (out_mnc)) {

			// copy first three digits
			mcc_buffer[0] = imsi_str[0];
			mcc_buffer[1] = imsi_str[1];
			mcc_buffer[2] = imsi_str[2]; // yeah, maybe there is a better way to do that copy.

			*out_mcc = sim800c_imsi_mcc_value_of (mcc_buffer);

			mnc_buffer[0] = imsi_str[3];
			mnc_buffer[1] = imsi_str[4];

			*out_mnc = atoi ((const char *)&mnc_buffer);
		}
		else {
			result = -1;
		}
	}
	else {
		result = -1;
	}
	// 260011204264113

	return result;
}

/**
 * Getter, which looks in @link{sim800c_imsi_preprogrammed_apns} and gets APN name
 * @param out_mcc
 * @param out_mnc
 * @return pointer to a string with APN name or null if such cannot be found
 */
const char *sim800c_get_apn (const sim800_mcc_t out_mcc, const uint8_t out_mnc)
{
	const char *output = 0;

	for (int i = 0; i < SIM800C_PREPROGMD_MCC_LAST; i++) {

		const sim800_mcc_t mcc_from_list = sim800c_imsi_preprogrammed_apns[i].country_code;
		const uint8_t network_code_from_list =
			sim800c_imsi_preprogrammed_apns[i].mobile_network_code;

		if ((mcc_from_list == out_mcc) && (network_code_from_list == out_mnc)) {

			output = sim800c_imsi_preprogrammed_apns[i].apn_name;
		}
	}

	return output;
}

/**
 * Getter, which looks in @link{sim800c_imsi_preprogrammed_apns} and gets username
 * @param out_mcc
 * @param out_mnc
 * @return pointer to a string with APN name or null if such cannot be found
 */
const char *sim800c_get_username (const sim800_mcc_t out_mcc, const uint8_t out_mnc)
{
	const char *output = 0;

	for (int i = 0; i < SIM800C_PREPROGMD_MCC_LAST; i++) {

		const sim800_mcc_t mcc_from_list = sim800c_imsi_preprogrammed_apns[i].country_code;
		const uint8_t network_code_from_list =
			sim800c_imsi_preprogrammed_apns[i].mobile_network_code;

		if ((mcc_from_list == out_mcc) && (network_code_from_list == out_mnc)) {

			output = sim800c_imsi_preprogrammed_apns[i].username;
		}
	}

	return output;
}

/**
 * Getter, which looks in @link{sim800c_imsi_preprogrammed_apns} and gets password
 * @param out_mcc
 * @param out_mnc
 * @return pointer to a string with APN name or null if such cannot be found
 */
const char *sim800c_get_password (const sim800_mcc_t out_mcc, const uint8_t out_mnc)
{
	const char *output = 0;

	for (int i = 0; i < SIM800C_PREPROGMD_MCC_LAST; i++) {

		const sim800_mcc_t mcc_from_list = sim800c_imsi_preprogrammed_apns[i].country_code;
		const uint8_t network_code_from_list =
			sim800c_imsi_preprogrammed_apns[i].mobile_network_code;

		if ((mcc_from_list == out_mcc) && (network_code_from_list == out_mnc)) {

			output = sim800c_imsi_preprogrammed_apns[i].password;
		}
	}

	return output;
}
