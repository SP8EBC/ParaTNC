/*
 * aprsis.h
 *
 *  Created on: Feb 20, 2022
 *      Author: mateusz
 */

#ifndef APRSIS_H_
#define APRSIS_H_

#include <stored_configuration_nvm/config_data.h>
#include "drivers/serial.h"
#include "gsm/sim800c_tcpip.h"
#include "ax25.h"
#include "telemetry.h"
#include "message.h"


typedef enum aprsis_return {
	APRSIS_OK					= 0,
	APRSIS_NOT_CONFIGURED		= 1,
	APRSIS_WRONG_STATE			= 2,
	APRSIS_ALREADY_CONNECTED	= 3,
	APRSIS_UNKNOWN				= -1
}aprsis_return_t;

extern uint8_t aprsis_connected;

/**
 * Initialize APRS-IS client
 * @param context
 * @param gsm_modem_state
 * @param callsign
 * @param ssid
 * @param passcode
 * @param default_server
 * @param default_port
 * @param reset_on_timeout	Set to one to reset GSM module in case of APRS-IS
 * 							instead of only reconnecting
 */
void aprsis_init(
		srl_context_t * context,
		gsm_sim800_state_t * gsm_modem_state,
		const char * callsign,
		uint8_t ssid,
		uint32_t passcode,
		const char * default_server,
		const uint16_t default_port,
		uint8_t reset_on_timeout,
		const char * callsign_with_ssid);
aprsis_return_t aprsis_connect_and_login(const char * address, uint8_t address_ln, uint16_t port, uint8_t auto_send_beacon);
aprsis_return_t aprsis_connect_and_login_default(uint8_t auto_send_beacon);
sim800_return_t aprsis_disconnect(void);
//void aprsis_receive_callback(srl_context_t* srl_context);
void aprsis_check_alive(void);
int aprsis_check_connection_attempt_alive(void);

void aprsis_send_wx_frame(uint16_t windspeed,
		uint16_t windgusts,
		uint16_t winddirection,
		float temperatura,
		float cisnienie,
		uint8_t humidity,
		const char * callsign_with_ssid,
		const char * string_latitude,
		const char * string_longitude,
		const config_data_basic_t * config_data_basic);
void aprsis_send_beacon(uint8_t async,
		const char * callsign_with_ssid,
		const char * string_latitude,
		char symbol_f,
		const char * string_longitude,
		char symbol_s,
		const config_data_basic_t * config_data_basic);
void aprsis_prepare_telemetry(
		uint16_t _telemetry_counter,
		uint8_t _rx_pkts,
		uint8_t _tx_pkts,
		uint8_t _digi_pkts,
		uint8_t _scaled_vbatt_voltage,
		uint8_t _viscous_drop_pkts,
		uint8_t _scaled_temperature,
		char _telemetry_qf,
		char _telemetry_degr,
		char _telemetry_nav,
		char _telemetry_pressure_qf_navaliable,
		char _telemetry_humidity_qf_navaliable,
		char _telemetry_anemometer_degradated,
		char _telemetry_anemometer_navble,
		char _telemetry_vbatt_low,
		const config_data_mode_t * const _config_mode);
void aprsis_send_telemetry(uint8_t async, const char * callsign_with_ssid);
telemetry_description_t aprsis_send_description_telemetry(uint8_t async,
														const telemetry_description_t what,
														const config_data_basic_t * const config_basic,
														const config_data_mode_t * const config_mode,
														const char * callsign_with_ssid);

void aprsis_igate_to_aprsis(AX25Msg *msg, const char * callsign_with_ssid);
void aprsis_send_status_send_battery_voltages(const char * callsign_with_ssid, uint16_t current, uint16_t average);
void aprsis_send_server_comm_counters(const char * callsign_with_ssid);
void aprsis_send_loginstring(const char * callsign_with_ssid, uint8_t rtc_ok, uint16_t voltage);
void aprsis_send_gsm_status(const char * callsign_with_ssid);
void aprsis_send_ack_for_message(const message_t * const message);

void aprsis_send_any_string_buffer(const char * const message, const uint16_t ln);

#ifdef UNIT_TEST
char * aprsis_get_tx_buffer(void);
#endif
uint8_t aprsis_get_aprsis_logged(void);

void aprsis_debug_set_simulate_timeout(void);

#endif /* APRSIS_H_ */
