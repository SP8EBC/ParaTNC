#ifndef CB306C87_36C1_4094_BF30_6A8819A8336C
#define CB306C87_36C1_4094_BF30_6A8819A8336C

/**
 * This file contains a state machine, which controls currently used communication protocol
 * going through GSM modem. For simplicity, currently only one concurrent TCP/UDP connection
 * can be established at once. So to talk with APRS/NTP server/API a state machine must be
 * used to protect against concurrent access to the modem. It is used mainly during startup.
 * After RTC date and time is synchronized from NTP server and events sent to the API,
 * GSM modem is then used only for APRS-IS communication
 */

#include "gsm/sim800_state_t.h"
#include <stdint.h>

/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL TYPES
/// ==================================================================================================

typedef enum gsm_comm_state_machine_t {
	GSM_COMM_NO_GPRS, //!< GPRS hasn't been started, so no TCP/UDP comm is currently possible
	GSM_COMM_IDLE,	  //!< GPRS is started but not used for anything
	GSM_COMM_NTP,	  //!< Currently used by NTP connection
	GSM_COMM_API,
	GSM_COMM_APRSIS //!< Currently used by APRS-IS

	/**
	 * Please remember that a state of this machine doesn't automatically mean
	 * that any connection is currently established
	 */
} gsm_comm_state_machine_t;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

gsm_comm_state_machine_t gsm_comm_state_get_current (void);

void gsm_comm_state_handler (uint8_t ntp_done,
							 uint8_t api_log_events_remaining,
							 gsm_sim800_state_t gsm_state);

#endif /* CB306C87_36C1_4094_BF30_6A8819A8336C */
