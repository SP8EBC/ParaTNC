/*
 * event_log.c
 *
 *  Created on: May 26, 2024
 *      Author: mateusz
 */

#include "event_log.h"
#include "event_log_strings.h"
#include "./nvm/nvm_event.h"
#include "main_master_time.h"
#include "variant.h"
#include "debug_hardfault.h"

#include "crc_.h"

#include <string.h>
#include <stdio.h>



/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define EVENT_LOG_ASYNC_FIFO_LENGTH 16

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static event_log_t event_log_async_fifo[EVENT_LOG_ASYNC_FIFO_LENGTH];

static int8_t event_log_fifo_current_depth = 0;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 */
void event_log_init (void)
{
	memset (event_log_async_fifo, 0x00, sizeof (event_log_t) * EVENT_LOG_ASYNC_FIFO_LENGTH);
	event_log_fifo_current_depth = 0;
}

/**
 * Stores new event asynchronously. Events are written into all volatile, RAM mapped areas
 * immediately, but FLASH based areas are synchronized periodically.
 * @param severity
 * @param source
 * @param wparam
 * @param lparam
 * @param lparam2
 */
int8_t event_log (event_log_severity_t severity,
				event_log_source_t source,
				uint8_t event_id,
				uint8_t param,
				uint8_t param2,
				uint16_t wparam,
				uint16_t wparam2,
				uint32_t lparam,
				uint32_t lparam2)
{
	return 0;
}

/**
 * Stores an event synchronously to all targer areas
 * @param severity
 * @param source
 * @param event_id
 * @param param
 * @param param2
 * @param wparam
 * @param wparam2
 * @param lparam
 * @param lparam2
 * @return
 */
int8_t event_log_sync (event_log_severity_t severity,
					 event_log_source_t source,
 					 uint8_t event_id,
					 uint8_t param,
					 uint8_t param2,
					 uint16_t wparam,
					 uint16_t wparam2,
					 uint32_t lparam,
					 uint32_t lparam2)
{
	event_log_t new_event = {0u};

	// left this to zero, to be automatically set to appropriate value by
	// pushing function
	new_event.event_counter_id = 0;

	new_event.event_id = event_id;
	new_event.event_master_time = main_get_master_time();
	new_event.event_rtc = main_get_nvm_timestamp();
	new_event.severity = EVENT_LOG_GET_SEVERITY(severity);
	new_event.source = EVENT_LOG_GET_SOURCE(source);

	new_event.param = param;
	new_event.param2 = param2;
	new_event.wparam = wparam;
	new_event.wparam2 = wparam2;
	new_event.lparam = lparam;
	new_event.lparam2 = lparam2;

	const nvm_event_result_t res = nvm_event_log_push_new_event(&new_event);

	if (res == NVM_EVENT_OK) {
		return 0;
	}
	else {
		return -1;
	}
}

const char * event_log_severity_to_str(event_log_severity_t severity) {

	switch (severity) {
		case EVENT_DEBUG:		return event_log_str_severity_debug;	/**< EVENT_DEBUG */
		case EVENT_INFO:		return event_log_str_severity_info;		/**< EVENT_INFO */
		case EVENT_INFO_CYCLIC: return event_log_str_severity_info_cyclic;
		case EVENT_WARNING:		return event_log_str_severity_warning;	/**< EVENT_WARNING */
		case EVENT_ERROR:		return event_log_str_severity_error;	/**< EVENT_ERROR */
		case EVENT_ASSERT:		return event_log_str_severity_assert;	/**< EVENT_ASSERT assert failure, which result in hard reset*/
		case EVENT_BOOTUP:		return event_log_str_severity_bootup;	/**< EVENT_BOOTUP all info events generated during bootup */
		case EVENT_TIMESYNC: 	return event_log_str_severity_timesync;	/**< EVENT_TIMESYNC event generated once at startup and then every 6 hours to
								   keep master_time and RTC date and time sync */
	}

	return 0;
}

/**
 * Returns a pointer to a string representing event source
 * @param src
 * @return
 */
const char * event_log_source_to_str(event_log_source_t src) {
	switch (src) {
		case EVENT_SRC_MAIN:				return event_log_str_src_main; break;
		case EVENT_SRC_WX_HANDLER:			return event_log_str_src_wx_handler; break;
		case EVENT_SRC_PWR_SAVE:			return event_log_str_src_pwr_save; break;
		case EVENT_SRC_PACKET_TX_HANDLER:	return event_log_str_src_packet_tx_handler; break;
		case EVENT_SRC_APRSIS:				return event_log_str_src_aprsis; break;
		case EVENT_SRC_KISS:				return event_log_str_src_kiss; break;
		case EVENT_SRC_APRS_RF:				return event_log_str_src_aprs_rf; break;
		case EVENT_SRC_GSM_GPRS:			return event_log_str_src_gsm_gprs; break;
		case EVENT_SRC_TCPIP:				return event_log_str_src_tcpip; break;
		case EVENT_SRC_HTTP_CLIENT:			return event_log_str_src_http_client; break;
		case EVENT_SRC_MODBUS:				return event_log_str_src_modbus; break;
		case EVENT_SRC_UMB:					return event_log_str_src_umb; break;
		case EVENT_SRC_DRV_ANEMOMETER:		return event_log_str_src_drv_anemometer; break;
		case EVENT_SRC_DRV_I2C:				return event_log_str_src_drv_i2c; break;
		case EVENT_SRC_DRV_UART:			return event_log_str_src_drv_uart; break;
		case EVENT_SRC_DRV_SPI:				return event_log_str_src_drv_spi; break;
		default: return event_log_default;
	}
}

/**
 *
 * @param source
 * @param event_id
 * @return
 */
const char * event_id_to_str(event_log_source_t source, uint8_t event_id)
{
	const char * out = event_log_default;

	switch (source) {
		case EVENT_SRC_MAIN: {

			if (event_id == EVENTS_MAIN_BOOTUP_COMPLETE) {
				out = event_log_str_main_bootup_complete;
			}
			else if (event_id == EVENTS_MAIN_TIMESYNC_BOOTUP) {
				out = event_log_str_main_timesync_bootup;
			}
			else if (event_id == EVENTS_MAIN_TIMESYNC_PERIODIC) {
				out = event_log_str_main_timesync_periodic;
			}
			else if (event_id == EVENTS_MAIN_TIMESYNC_NTP) {
				out = event_log_str_main_timesync_ntp;
			}
			else if (event_id == EVENTS_MAIN_CYCLIC) {
				out = event_log_str_main_info_cyclic;
			}
			break;
		}
		case EVENT_SRC_WX_HANDLER:	{

			if (event_id == EVENTS_WX_HANDLER_WARN_TEMPERATURE_INT_FAILED) {
				out = event_log_str_wx_handler_temperature_int_failed;
			}
			else if (event_id == EVENTS_WX_HANDLER_WARN_TEMPERATURE_DALLAS_DEGR) {
				out = event_log_str_wx_handler_temperature_dallas_degraded;
			}
			else if (event_id == EVENTS_WX_HANDLER_WARN_TEMPERATURE_DALLAS_NAV) {
				out = event_log_str_wx_handler_temperature_dallas_not_avble;
			}
			else if (event_id == EVENTS_WX_HANDLER_WARN_TEMPERATURE_EXCESIVE_SLEW) {
				out = event_log_str_wx_handler_temperature_excesive_slew;
			}
			else if (event_id == EVENTS_WX_HANDLER_WARN_PRESSURE_FAILED) {
				out = event_log_str_wx_handler_temperature_pressure_fail;
			}
			else if (event_id == EVENTS_WX_HANDLER_WARN_HUMIDITY_FAILED) {
				out = event_log_str_wx_handler_temperature_humidity_fail;
			}
			///////
			///
			else if (event_id == EVENTS_WX_HANDLER_ERROR_RTE_CHECK_ANEM_TIMER_HAS_BEEN_FIRED) {
				out = event_log_str_wx_handler_error_rte_check_anem_timer_has_been_fired;
			}
			else if (event_id == EVENTS_WX_HANDLER_ERROR_RTE_CHECK_SLEW_LIMIT) {
				out = event_log_str_wx_handler_error_rte_check_slew_limit;
			}
			else if (event_id == EVENTS_WX_HANDLER_ERROR_RTE_CHECK_DEBOUCING) {
				out = event_log_str_wx_handler_error_rte_check_debouncing;
			}
			else if (event_id == EVENTS_WX_HANDLER_ERROR_RTE_CHECK_UF_CONVERTER_FAIL) {
				out = event_log_str_wx_handler_error_rte_check_uf_converter_fail;
			}
			else if (event_id == EVENTS_WX_HANDLER_ERROR_RTE_CHECK_WINDSPEED_BUFFERS) {
				out = event_log_str_wx_handler_error_rte_check_windspeed_buffers;
			}

			break;
		}
		case EVENT_SRC_PWR_SAVE:
			if (event_id == EVENTS_PWR_SAVE_BATT_LOW_GOING_SLEEP) {
				out = event_log_str_pwr_save_going_sleep;
			}
			break;
		case EVENT_SRC_PACKET_TX_HANDLER:	 break;
		case EVENT_SRC_APRSIS: {
			if (event_id == EVENTS_APRSIS_ERROR_IM_NOT_OK_LAST_KEEPALIVE) {
				out = event_log_str_aprsis_im_not_ok_last_keepalive;
			}
			else if (event_id == EVENTS_APRSIS_ERROR_IM_NOT_OK_LAST_TRANSMIT_LONG) {
				out = event_log_str_aprsis_im_not_ok_last_transmit;
			}
			else if (event_id == EVENTS_APRSIS_WARN_AUTH_FAILED) {
				out = event_log_str_aprsis_warn_auth_failed;
			}
			else if (event_id == EVENTS_APRSIS_WARN_TIMEOUT_WAITING_AUTH) {
				out = event_log_str_aprsis_warn_timeout_waiting_auth;
			}
			else if (event_id == EVENTS_APRSIS_WARN_NO_HELLO_MESSAGE) {
				out = event_log_str_aprsis_warn_no_hello_message;
			}
			else if (event_id == EVENTS_APRSIS_WARN_TIMEOUT_WAITING_HELLO_MSG) {
				out = event_log_str_aprsis_warn_timeout_waiting_hello_msg;
			}
			else if (event_id == EVENTS_APRSIS_WARN_CONNECT_FAILED) {
				out = event_log_str_aprsis_warn_connect_failed;
			}
			else if (event_id == EVENTS_APRSIS_WARN_WRONG_STATE) {
				out = event_log_str_aprsis_warn_wrong_state;
			}
			else if (event_id == EVENTS_APRSIS_WARN_DEAD_KEEPALIVE) {
				out = event_log_str_aprsis_warn_dead_keepalive;
			}
			else if (event_id == EVENTS_APRSIS_WARN_DEAD_TRANSMIT) {
				out = event_log_str_aprsis_warn_dead_transmit;
			}
			break;
		}
		case EVENT_SRC_KISS:				 break;
		case EVENT_SRC_APRS_RF:				 break;
		case EVENT_SRC_GSM_GPRS:			 
			if (event_id == EVENTS_GSM_GPRS_ERR_APN_CONFIGURATION_MISSING) {
				out = event_log_str_tcpip_error_apn_config_missing;
			}
			else if (event_id == EVENTS_GSM_GPRS_WARN_ASYNC_MSG_DETECTED) {
				out = event_log_str_tcpip_warn_async_msg_detected;
			}
			else if (event_id == EVENTS_GSM_GPRS_ERR_SIM_CARD_STATUS) {
				out = event_log_str_tcpip_error_sim_card_status;
			}
			else if (event_id == EVENTS_GSM_GPRS_WARN_NOT_REGISTERED_TO_NETWORK) {
				out = event_log_str_tcpip_warn_not_registered_to_nework;
			}
			else if (event_id == EVENTS_GSM_GPRS_REGISTERED_NETWORK) {
				out = event_log_str_tcpip_bootup_registered_network;
			}
			else if (event_id == EVENTS_GSM_GPRS_SIGNAL_LEVEL) {
				out = event_log_str_tcpip_bootup_signal_level;
			}
			else if (event_id == EVENTS_GSM_GPRS_IMSI) {
				out = event_log_str_tcpip_bootup_imsi;
			}
			else if (event_id == EVENTS_GSM_GPRS_IP_ADDRESS) {
				out = event_log_str_tcpip_bootup_ip_address;
			}
			/**
			 * static const char * event_log_str_tcpip_bootup_registered_network = "NETWORK_NAME\0";
static const char * event_log_str_tcpip_bootup_signal_level = "SIGNAL_LEVEL_DBM\0";
static const char * event_log_str_tcpip_bootup_imsi = "IMSI\0";
static const char * event_log_str_tcpip_bootup_ip_address = "IP_ADDR\0";
			 *
			 *
			 */

			break;
		case EVENT_SRC_TCPIP:				 
			if (event_id == EVENTS_TCPIP_ERR_CONNECTING) {
				out = event_log_str_tcpip_error_connecting;
			}
			else if (event_id == EVENTS_TCPIP_ERR_CONNECTING_NO_MODEM_RESPONSE) {
				out = event_log_str_tcpip_error_connecting_no_modem_resp;
			}

			break;
		case EVENT_SRC_HTTP_CLIENT:			 break;
		case EVENT_SRC_MODBUS:				 break;
		case EVENT_SRC_UMB:		{
			if (event_id == EVENTS_UMB_WARN_CRC_FAILED_IN_RECEIVED_FRAME) {
				out = event_log_str_umb_warn_crc_failed_in_received_frame;
			}
			else if (event_id == EVENTS_UMB_WARN_RECEIVED_FRAME_MALFORMED) {
				out = event_log_str_umb_warn_received_frame_malformed;
			}
			else if (event_id == EVENTS_UMB_WARN_NOK_STATUS_IN_GET_STATUS_RESP) {
				out = event_log_str_umb_warn_nok_sensor_status_in_get_status_data;
			}
			else if (event_id == EVENTS_UMB_WARN_NOK_STATUS_IN_OFFLINE_DATA_RESP) {
				out = event_log_str_umb_warn_nok_sensor_status_in_offline_data;
			}
			else if (event_id == EVENTS_UMB_ERROR_RECEIVING) {
				out = event_log_str_umb_error_receiving;
			}
			else if (event_id == EVENTS_UMB_ERROR_UNEXPECTED_ROUTINE_ID) {
				out = event_log_str_umb_error_unexp_routine_id;
			}
			else if (event_id == EVENTS_UMB_ERROR_QF_NOT_AVAILABLE) {
				out = event_log_str_umb_error_quality_factor_not_avail;
			}

			break;
		}
		case EVENT_SRC_DRV_ANEMOMETER: {

			if (event_id == EVENTS_DRV_ANEMOMETER_WARN_NO_PULSES_INT_FIRED) {
				out = event_log_str_drv_anemometer_no_pulses_int_fired;
			}
			else if (event_id == EVENTS_DRV_ANEMOMETER_WARN_EXCESIVE_SLEW) {
				out = event_log_str_drv_anemometer_excesive_slew_rate;
			}
			else if (event_id == EVENTS_DRV_ANEMOMETER_ERROR_UF_CONV_NOT_WORKING) {
				out = event_log_str_drv_anemometer_uf_conv_not_working;
			}
			else if (event_id == EVENTS_DRV_ANEMOMETER_ERROR_UF_FREQ_TOO_HI) {
				out = event_log_str_drv_anemometer_uf_freq_to_hi;
			}
			else if (event_id == EVENTS_DRV_ANEMOMETER_WARN_QF_NOT_FULL) {
				out = event_log_str_drv_anemometer_qf_not_full;
			}

			break;
		}
		case EVENT_SRC_DRV_I2C:				 break;
		case EVENT_SRC_DRV_UART:			 break;
		case EVENT_SRC_DRV_SPI:				 break;
		default: out = event_log_default;
	}

	return out;

}

/**
 * Generates string representation of given event log in exposed form
 * @param exposed pointer to an event to be converted
 * @param output char buffer to place a string into
 * @param output_ln maximum length of output string
 * @return length of assembled string
 */
uint16_t event_exposed_to_string(const event_log_exposed_t * exposed, char * output, uint16_t output_ln)
{
	uint16_t out = 0;

	char severity = ' ';

	switch (exposed->severity) {
		case EVENT_DEBUG:		severity = 'D'; break;
		case EVENT_INFO:		severity = 'I'; break;
		case EVENT_INFO_CYCLIC:	severity = 'C'; break;
		case EVENT_WARNING:		severity = 'W'; break;
		case EVENT_ERROR:		severity = 'E'; break;
		case EVENT_ASSERT:		severity = 'A'; break;
		case EVENT_BOOTUP:		severity = 'B'; break;
		case EVENT_TIMESYNC:	severity = 'T'; break;
	}

	if (exposed->severity == EVENT_TIMESYNC)
	{
		out = snprintf(output, output_ln,
						"[%c][CNT:%lu][MT:%lu][RTC_TIMESYNC][DATE:%d/%d/%d][TIME:%d:%d:%d]",
						severity,
						exposed->event_counter_id,
						exposed->event_master_time,
						(uint16_t)exposed->param2,
						(uint16_t)exposed->param,
						(uint16_t)exposed->wparam,
						(uint16_t)exposed->wparam2,
						(uint16_t)exposed->lparam,
						(uint16_t)exposed->lparam2);
	}
	else if (exposed->severity == EVENT_ERROR &&
					exposed->event_id == EVENTS_MAIN_POSTMORTEM_HARDFAULT
			)
	{
		// special format for hardfault postmortem logs

		char * first		= (char *)event_log_default;
		char * second		= (char *)event_log_default;

		// param value defines which stackframe registers are stored as lparams
		switch (exposed->param)
		{
			case 1: first = (char *)event_log_str_hardfault_lr; second = (char *)event_log_str_hardfault_pc; break;
			case 2: first = (char *)event_log_str_hardfault_r0; second = (char *)event_log_str_hardfault_r1; break;
			case 3: first = (char *)event_log_str_hardfault_r2; second = (char *)event_log_str_hardfault_r3; break;
			case 4: first = (char *)event_log_str_hardfault_r12; second = (char *)event_log_str_hardfault_cfsr; break;
			case 5: first = (char *)event_log_str_hardfault_src; second = (char *)event_log_str_hardfault_xpsr; break;
			default: first = (char *)event_log_default; second = (char *)event_log_default; break;
		}

		// value of 5 contain hard fault source, which might be decoded from numeric value to dedicated string
		if (exposed->param == 5)
		{
			char * source = (char *)event_log_default;

			switch (exposed->lparam)
			{
				case DEBUG_HARDFAULT_SOURCE_HFLT:		source = (char *)event_log_str_hardfault_hf; break;
				case DEBUG_HARDFAULT_SOURCE_USAGEFLT:	source = (char *)event_log_str_hardfault_usage; break;
				case DEBUG_HARDFAULT_SOURCE_BUSFLT:		source = (char *)event_log_str_hardfault_bus; break;
				case DEBUG_HARDFAULT_SOURCE_MMUFLT:		source = (char *)event_log_str_hardfault_mmu; break;
			}
			// special case if the fault source can be printed
			out = snprintf(output, output_ln,
							"[ERR-HF][CNT:%lu][%s: %s][%s: 0x%lX]",
							exposed->event_counter_id,
							first,
							source,
							second,
							exposed->lparam2);
		}
		else
		{
			// everything else which has only registers values
			out = snprintf(output, output_ln,
							"[ERR-HF][CNT:%lu][%s: 0x%lX][%s: 0x%lX]",
							exposed->event_counter_id,
							first,
							exposed->lparam,
							second,
							exposed->lparam2);
		}
	}
	else if (exposed->severity == EVENT_ERROR &&
					exposed->event_id == EVENTS_MAIN_POSTMORTEM_SUPERVISOR
			)
	{
		// special case for a log with a master time value on which
		// supervisor failure has been detected
		if (exposed->param == 0)
		{
			out = snprintf(output, output_ln,
							"[ERR-SUP][CNT:%lu][0][MT-FLT: %lu][%d: %lu]",
							exposed->event_counter_id,
							exposed->lparam,
							exposed->param + 1,
							exposed->lparam2);
		}
		else
		{
			if (exposed->lparam != 0 && exposed->lparam2 != 0)
			{
				// everything else which has only registers values
				out = snprintf(output, output_ln,
								"[ERR-SUP][CNT:%lu][0][%d: %lu][%d: %lu]",
								exposed->event_counter_id,
								exposed->param,
								exposed->lparam,
								exposed->param + 1,
								exposed->lparam2);
			}
		}
	}
	else
	{
		// if pointer to event string is set
		if (variant_validate_is_within_flash((const void*)exposed->event_str_name) == 1)
		{
			out = snprintf(output, output_ln,
							"[%c][CNT:%lu][MT:%lu][%s][%s][P: %X %X %X %X %lX %lX]",
							severity,
							exposed->event_counter_id,
							exposed->event_master_time,
							exposed->source_str_name,
							exposed->event_str_name,
							exposed->param,
							exposed->param2,
							exposed->wparam,
							exposed->wparam2,
							exposed->lparam,
							exposed->lparam2);
		}
		else
		{
			out = snprintf(output, output_ln,
							"[%c][CNT:%lu][MT:%lu][%s][EV:%d][P: %X %X %X %X %lX %lX]",
							severity,
							exposed->event_counter_id,
							exposed->event_master_time,
							exposed->source_str_name,
							exposed->event_id,
							exposed->param,
							exposed->param2,
							exposed->wparam,
							exposed->wparam2,
							exposed->lparam,
							exposed->lparam2);
		}
	}


	return out;
}

