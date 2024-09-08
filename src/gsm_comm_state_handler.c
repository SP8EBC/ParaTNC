/**
 * This file contains a state machine, which controls currently used communication protocol
 * going through GSM modem. For simplicity, currently only one concurrent TCP/UDP connection
 * can be established at once. So to talk with APRS/NTP server/API a state machine must be
 * used to protect against concurrent access to the modem. It is used mainly during startup. 
 * After RTC date and time is synchronized from NTP server and events sent to the API, 
 * GSM modem is then used only for APRS-IS communication
 */

#include "gsm_comm_state_handler.h"

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static gsm_comm_state_machine_t gsm_comm_state_machine = GSM_COMM_NO_GPRS;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

const gsm_comm_state_machine_t gsm_comm_state_get_current (void) 
{
    return gsm_comm_state_machine;
}

void gsm_comm_state_handler (uint8_t ntp_done,
							 uint8_t api_log_events_remaining,
							 gsm_sim800_state_t gsm_state) 
{
    if (gsm_state == SIM800_ALIVE)
    {
        if (ntp_done == 0) 
        {
            gsm_comm_state_machine = GSM_COMM_NTP;
        }
        else if (ntp_done > 0 && api_log_events_remaining > 0)
        {
            gsm_comm_state_machine = GSM_COMM_API;
        }
        else if (ntp_done > 0 && api_log_events_remaining == 0)
        {
            gsm_comm_state_machine = GSM_COMM_APRSIS;
        }
        else 
        {
            gsm_comm_state_machine = GSM_COMM_APRSIS; // ????? TODO
        }
    }
    else if (gsm_state < SIM800_ALIVE) 
    {
        gsm_comm_state_machine = GSM_COMM_NO_GPRS;       
    }
}
