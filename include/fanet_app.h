/*
 * fanet_app.h
 *
 *  Created on: Feb 28, 2025
 *      Author: mateusz
 */

#ifndef FANET_APP_H_
#define FANET_APP_H_

#include "stored_configuration_nvm/config_data.h"
#include "skytrax_fanet/types/fanet_mac_address_t.h"

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

extern int fanet_success_cnt;
extern int fanet_fail_cnt;
extern int fanet_tx_success_cnt;

/**
 * Source address used to transmit own FANET packets
 */
extern fanet_mac_adress_t fanet_src;

/**
 * Station latitude put in transmitted FANET packets
 */
extern float fanet_latitude;

/**
 * Station longitude put in transmitted FANET packets
 */
extern float fanet_longitude;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void fanet_test_init (config_data_gsm_t const *const config,
					  config_data_basic_t const *const coordinates);

/**
 * Only a test
 */
int fanet_test (void);

#endif /* FANET_APP_H_ */
