#include "packet_tx_handler.h"
#include "backup_registers.h"
#include "supervisor.h"
#include "wx_handler.h"

#include "rte_main.h"
#include "rte_pv.h"
#include "rte_wx.h"

#include "station_config.h"
#include "station_config_target_hw.h"

#include "./aprs/beacon.h"
#include "./aprs/status.h"
#include "./aprs/telemetry.h"
#include "./aprs/wx.h"

#include "./drivers/serial.h"

#include "./umb_master/umb_master.h"

#include "../include/etc/rtu_configuration.h"

#include "./nvm/nvm.h"
#include "api/api.h"
#include "aprsis.h"
#include "pwr_save.h"

#include "gsm/sim800c_gprs.h"

#include "event_log.h"
#include "events_definitions/events_packet_tx_handler.h"

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <task.h>

#include "delay.h"
#include "io.h"
#include "main.h"

#define _TELEM_DESCR_INTERVAL 150

uint8_t packet_tx_beacon_interval = 0;
uint8_t packet_tx_beacon_counter = 0;

uint8_t packet_tx_error_status_interval = 2;
uint8_t packet_tx_error_status_counter = 0;

uint8_t packet_tx_meteo_aggresive_interval = 0;
uint8_t packet_tx_meteo_non_aggresive_interval = 0;
uint8_t packet_tx_meteo_interval = 0;
uint8_t packet_tx_meteo_counter = 0;

uint8_t packet_tx_meteo_kiss_interval = 2;
uint8_t packet_tx_meteo_kiss_counter = 0;

uint8_t packet_tx_telemetry_interval = 10;
uint8_t packet_tx_telemetry_counter = 0;

uint8_t packet_tx_telemetry_descr_interval = 155; // 155
uint8_t packet_tx_telemetry_descr_counter = 10;

uint8_t packet_tx_modbus_raw_values =
	(uint8_t)(_TELEM_DESCR_INTERVAL - _WX_INTERVAL * (uint8_t)(_TELEM_DESCR_INTERVAL / 38));
uint8_t packet_tx_modbus_status =
	(uint8_t)(_TELEM_DESCR_INTERVAL - _WX_INTERVAL * (uint8_t)(_TELEM_DESCR_INTERVAL / 5));

uint8_t packet_tx_more_than_one = 0;

static uint8_t packet_tx_trigger_tcp = 0;

uint8_t packet_tx_meteo_gsm_interval = 2;
uint8_t packet_tx_meteo_gsm_counter = 0;

//!< Flag set to one after weather packet has been just send to APRS-IS via GPRS modem and the modem
//!< is probably transmitting now
uint8_t packet_tx_meteo_gsm_has_been_sent = 0;

#define API_TRIGGER_STATUS	 (1 << 1)
#define API_TRIGGER_METEO	 (1 << 2)
#define APRSIS_TRIGGER_METEO (1 << 3)
#define RECONNECT_APRSIS	 (1 << 7)

nvm_measurement_t packet_tx_nvm;

//!< Flag set to one after the gsm status message is sent over radio.
uint8_t packet_tx_gsm_status_sent = 0;

void packet_tx_send_wx_frame (void)
{
	main_wait_for_tx_complete ();

	SendWXFrame (rte_wx_average_windspeed,
				 rte_wx_max_windspeed,
				 rte_wx_average_winddirection,
				 rte_wx_temperature_average_external_valid,
				 rte_wx_pressure_valid,
				 rte_wx_humidity_valid);
}

void packet_tx_init (uint8_t meteo_interval, uint8_t aggressive_meteo_interval,
					 uint8_t beacon_interval, config_data_powersave_mode_t powersave)
{

	// check if values are set reasonable
	if (meteo_interval >= 5 && meteo_interval < 60) {
		packet_tx_meteo_interval = meteo_interval;
	}
	else {
		packet_tx_meteo_interval = 5;
	}

	packet_tx_meteo_non_aggresive_interval = packet_tx_meteo_interval;

	if (aggressive_meteo_interval > packet_tx_meteo_interval) {
		packet_tx_meteo_aggresive_interval = aggressive_meteo_interval;
	}
	else {
		packet_tx_meteo_aggresive_interval = 10;
	}

	// check if values are set reasonable
	if (beacon_interval >= 10 && beacon_interval < 90) {
		packet_tx_beacon_interval = beacon_interval;
	}
	else {
		packet_tx_beacon_interval = 15;
	}

	backup_reg_get_packet_counters (&packet_tx_beacon_counter,
									&packet_tx_meteo_counter,
									&packet_tx_meteo_gsm_counter);

	if (powersave == PWSAVE_AGGRESV) {
		// if user selected aggressive powersave mode the meteo counter must be set back to zero
		// to prevent quirks with waking from sleep mode
		packet_tx_meteo_counter = 0;
	}
}

void packet_tx_restore_from_backupregs (void)
{
}

/**
 * This function is called from the inside of 'packet_rx_handler' to put an extra wait
 * if more than one packet is sent from the single call to that function. This is required
 * to protect against jamming own frames when any path is configured.
 *
 */
inline void packet_tx_multi_per_call_handler (void)
{
	// if this consecutive frame sent from one call to this function
	if (packet_tx_more_than_one > 0) {
		// wait for previous transmission to complete
		main_wait_for_tx_complete ();

		// wait for any possible retransmission to kick in
		vTaskDelay (2000 / portTICK_PERIOD_MS);

		// reload watchdog counter
		main_reload_internal_wdg ();
	}
	else {
		packet_tx_more_than_one = 1;
	}
}

// this shall be called in 10 seconds interval
void packet_tx_tcp_handler (void)
{

	// TODO: fixme currently there is no way to have APRS-IS and rest api
	// client working at the same time

	if ((packet_tx_trigger_tcp & APRSIS_TRIGGER_METEO) != 0) {
		if (aprsis_connected == 1) {

			aprsis_send_wx_frame (rte_wx_average_windspeed,
								  rte_wx_max_windspeed,
								  rte_wx_average_winddirection,
								  rte_wx_temperature_average_external_valid,
								  rte_wx_pressure_valid,
								  rte_wx_humidity_valid,
								  main_callsign_with_ssid,
								  main_string_latitude,
								  main_string_longitude,
								  main_config_data_basic);

			// clear the flag requesting weather packet transmission
			packet_tx_trigger_tcp ^= APRSIS_TRIGGER_METEO;

			// set this flag to one to inhibit power saving state machine
			// for a while (10 seconds) when GPRS modem is communicating
			// with the GSM radio network and sending the data independently
			// from the controller
			packet_tx_meteo_gsm_has_been_sent = 1;
		}
	}
	else if ((packet_tx_trigger_tcp & API_TRIGGER_STATUS) != 0) {

		// TODO: fixme
		// check if APRS-IS is connected
		if (aprsis_connected != 0) {
			// disconnect it before call to API - this disconnection has blocking IO
			aprsis_disconnect ();

			// remember to reconnect APRSIS after all API comm will be done
			packet_tx_trigger_tcp |= RECONNECT_APRSIS;
		}
		// TODO: fixme

		// send status (async)
		api_send_json_status ();

		// clear the bit
		packet_tx_trigger_tcp ^= API_TRIGGER_STATUS;
	}
	else if ((packet_tx_trigger_tcp & API_TRIGGER_METEO) != 0) {

		// TODO: fixme
		// check if APRS-IS is connected
		if (aprsis_connected != 0) {
			// disconnect it before call to API - this disconnection has blocking IO
			aprsis_disconnect ();

			// remember to reconnect APRSIS after all API comm will be done
			packet_tx_trigger_tcp |= RECONNECT_APRSIS;
		}
		// TODO: fixme

		api_calculate_mac ();
		api_send_json_measuremenets ();

		// clear the bit
		packet_tx_trigger_tcp ^= API_TRIGGER_METEO;
	}
	else if ((packet_tx_trigger_tcp & RECONNECT_APRSIS) != 0) {
		// TODO: fixme
		//		result = aprsis_connect_and_login_default(1);
		//
		//		if (result == APRSIS_OK) {
		packet_tx_trigger_tcp ^= RECONNECT_APRSIS;
		//		}
	}
	else {
		// after 10 second from setting this flag the packet should be
		// sent
		packet_tx_meteo_gsm_has_been_sent = 0;
	}
}

// this shall be called in 60 seconds intervals
void packet_tx_handler (const config_data_basic_t *const config_basic,
						const config_data_mode_t *const config_mode)
{
	dallas_qf_t dallas_qf = DALLAS_QF_UNKNOWN;

	pressure_qf_t pressure_qf = PRESSURE_QF_UNKNOWN;
	humidity_qf_t humidity_qf = HUMIDITY_QF_UNKNOWN;
	wind_qf_t wind_qf = WIND_QF_UNKNOWN;

	uint16_t ln = 0;

	// set to one if more than one packet will be send from this function at once (like beacon +
	// telemetry)
	packet_tx_more_than_one = 0;

	// check if there is a time to send own beacon
	if (packet_tx_beacon_counter >= packet_tx_beacon_interval && packet_tx_beacon_interval != 0) {

		packet_tx_multi_per_call_handler ();

		beacon_send_own (0, 0);

		packet_tx_beacon_counter = 0;
	}

	// if WX is enabled
	if ((main_config_data_mode->wx & WX_ENABLED) == WX_ENABLED) {

		// check if there is a time to send meteo packet through RF
		if (packet_tx_meteo_counter >= packet_tx_meteo_interval && packet_tx_meteo_interval != 0) {

			if (config_mode->nvm_logger != 0) {
				packet_tx_nvm.temperature_humidity = wx_get_nvm_record_temperature ();
				packet_tx_nvm.wind = wx_get_nvm_record_wind ();
				packet_tx_nvm.timestamp = main_get_nvm_timestamp ();

				taskENTER_CRITICAL ();
				// write to NVM if it is enabled
				nvm_measurement_store (&packet_tx_nvm);
				taskEXIT_CRITICAL ();
			}

			// this function is required if more than one RF frame will be send from this function
			// at once it waits for transmission completion and add some delay to let digipeaters do
			// theris job
			packet_tx_multi_per_call_handler ();

			// send WX frame through RF
			SendWXFrame (rte_wx_average_windspeed,
						 rte_wx_max_windspeed,
						 rte_wx_average_winddirection,
						 rte_wx_temperature_average_external_valid,
						 rte_wx_pressure_valid,
						 rte_wx_humidity_valid);

			/**
			 * debug
			 *
			 *	#define REGISTER_LAST_SLEEP	RTC->BKP1R
				#define REGISTER_LAST_WKUP	RTC->BKP2R
				#define REGISTER_COUNTERS	RTC->BKP4R
			 *
			 */

			// service external watchdog while sending weather frame
			io_ext_watchdog_service ();

			event_log_sync (EVENT_INFO,
							EVENT_SRC_PACKET_TX_HANDLER,
							EVENTS_TX_HANDLER_INFO_SENDING_WXFRAME,
							packet_tx_meteo_counter,
							packet_tx_meteo_interval,
							rte_wx_average_windspeed,
							rte_wx_max_windspeed,
							(uint32_t)rte_wx_average_winddirection,
							(uint32_t) * ((uint32_t *)&rte_wx_temperature_average_external_valid));

			// check if user want's to send two wx packets one after another
			if (main_config_data_basic->wx_double_transmit == 1) {
				rte_main_trigger_wx_packet = 1;
			}

			packet_tx_meteo_counter = 0;
		}

		// if modus RTU client is enabled with debugging
		if ((main_config_data_mode->wx_modbus & WX_MODBUS_DEBUG) == WX_MODBUS_DEBUG) {
			// send the status packet with raw values of all requested modbus-RTU registers
			if (packet_tx_meteo_counter == (packet_tx_meteo_interval - 1) &&
				packet_tx_telemetry_descr_counter >= packet_tx_modbus_raw_values) {

				packet_tx_multi_per_call_handler ();

				status_send_raw_values_modbus ();
			}
		}

		// there is no sense to include support for Victron VE.Direct in parameteo
		// which has its own charging controller
		// check if Victron VE.Direct serial protocol client is enabled and it is
		// a time to send the status message
		if (config_mode->victron == 1 &&
			packet_tx_meteo_counter == (packet_tx_meteo_interval - 1) &&
			packet_tx_telemetry_descr_counter >= packet_tx_modbus_raw_values) {
			packet_tx_multi_per_call_handler ();

			telemetry_send_status_pv (&rte_pv_average,
									  &rte_pv_last_error,
									  rte_pv_struct.system_state,
									  master_time,
									  rte_pv_messages_count,
									  rte_pv_corrupted_messages_count);
		}

		if (packet_tx_meteo_gsm_counter >= packet_tx_meteo_gsm_interval &&
			gsm_sim800_gprs_ready == 1) {
			if (main_config_data_gsm->aprsis_enable == 0 && main_config_data_gsm->api_enable == 1) {
				// and trigger API wx packet transmission
				packet_tx_trigger_tcp |= API_TRIGGER_METEO;
			}
			else if (main_config_data_gsm->aprsis_enable == 1 &&
					 main_config_data_gsm->api_enable == 0) {
				packet_tx_trigger_tcp |= APRSIS_TRIGGER_METEO;
			}
			else {
				packet_tx_trigger_tcp = 0;
			}

			packet_tx_meteo_gsm_counter = 0;
		}

		// send wx frame to KISS host once every two minutes
		if (packet_tx_meteo_kiss_counter >= packet_tx_meteo_kiss_interval &&
			main_kiss_enabled == 1) {

			// wait if serial port is currently used
			srl_wait_for_tx_completion (main_kiss_srl_ctx_ptr);

			// create wx data packet into specified buffer
			SendWXFrameToKissBuffer (rte_wx_average_windspeed,
									 rte_wx_max_windspeed,
									 rte_wx_average_winddirection,
									 rte_wx_temperature_average_external_valid,
									 rte_wx_pressure_valid,
									 rte_wx_humidity_valid,
									 main_kiss_srl_ctx_ptr->srl_tx_buf_pointer,
									 main_kiss_srl_ctx_ptr->srl_tx_buf_ln,
									 &ln);

			srl_start_tx (main_kiss_srl_ctx_ptr, ln);

			packet_tx_meteo_kiss_counter = 0;
		}
	}

	if (packet_tx_telemetry_counter >= packet_tx_telemetry_interval) {

		packet_tx_multi_per_call_handler ();

		// ASSEMBLY QUALITY FACTORS

		// if there weren't any errors related to the communication with DS12B20 from previous
		// function call
		if (rte_wx_error_dallas_qf == DALLAS_QF_UNKNOWN) {
			dallas_qf = rte_wx_current_dallas_qf; // it might be DEGRADATED so we need to copy a
												  // value directly

			// reset current QF to check if there will be at least one successfull readout of
			// temperature
			rte_wx_current_dallas_qf = DALLAS_QF_UNKNOWN;
		}

		// if there were any errors
		else {

			// if we had at least one successfull communication with the sensor
			if (rte_wx_current_dallas_qf == DALLAS_QF_FULL ||
				rte_wx_current_dallas_qf == DALLAS_QF_DEGRADATED) {
				// set the error reason
				dallas_qf = DALLAS_QF_DEGRADATED;
			}
			// if they wasn't any successfull comm
			else {
				// set that it is totally dead and not avaliable
				dallas_qf = DALLAS_QF_NOT_AVALIABLE;
			}

			// and reset the error
			rte_wx_error_dallas_qf = DALLAS_QF_UNKNOWN;

			rte_wx_current_dallas_qf = DALLAS_QF_UNKNOWN;
		}

		// get quality factors for internal pressure and humidity sensors
		if (config_mode->wx_ms5611_or_bme == 0) {
			// pressure sensors quality factors

			switch (rte_wx_ms5611_qf) {
			case MS5611_QF_FULL: pressure_qf = PRESSURE_QF_FULL; break;
			case MS5611_QF_NOT_AVALIABLE: pressure_qf = PRESSURE_QF_NOT_AVALIABLE; break;
			case MS5611_QF_DEGRADATED: pressure_qf = PRESSURE_QF_DEGRADATED; break;
			case MS5611_QF_UNKNOWN: pressure_qf = PRESSURE_QF_UNKNOWN; break;
			}
		}
		else if (config_mode->wx_ms5611_or_bme == 1) {
			// #elif defined(_SENSOR_BME280)
			//  humidity sensors quality factors
			if (rte_wx_bme280_qf == BME280_QF_UKNOWN) {
				;
			}
			else {
				// use BME280
				switch (rte_wx_bme280_qf) {
				case BME280_QF_FULL:
				case BME280_QF_PRESSURE_DEGRADED: humidity_qf = HUMIDITY_QF_FULL; break;
				case BME280_QF_UKNOWN:
				case BME280_QF_NOT_AVAILABLE: humidity_qf = HUMIDITY_QF_NOT_AVALIABLE; break;
				case BME280_QF_HUMIDITY_DEGRADED:
				case BME280_QF_GEN_DEGRADED: humidity_qf = HUMIDITY_QF_DEGRADATED; break;
				}

				switch (rte_wx_bme280_qf) {
				case BME280_QF_FULL:
				case BME280_QF_HUMIDITY_DEGRADED: pressure_qf = PRESSURE_QF_FULL; break;
				case BME280_QF_UKNOWN:
				case BME280_QF_NOT_AVAILABLE: pressure_qf = PRESSURE_QF_NOT_AVALIABLE; break;
				case BME280_QF_PRESSURE_DEGRADED:
				case BME280_QF_GEN_DEGRADED: pressure_qf = PRESSURE_QF_DEGRADATED; break;
				}
			}
		}
		else {
			pressure_qf = PRESSURE_QF_NOT_AVALIABLE; // TODO:
			humidity_qf = HUMIDITY_QF_NOT_AVALIABLE;
		}

		// additional check for modbus RTU humidity sensor
		if (config_mode->wx_modbus == 1) {
			// for sake of simplicity there is another variable
			// which holds common information if valid humidity
			// is available or not. This is kinda-sorta duplication of
			// quality factors for each sensor types, although it
			// simplifies few things and omits handling separate
			// Modbus-RTU configuration here
			if (rte_wx_humidity_available == 0) {
				humidity_qf = HUMIDITY_QF_NOT_AVALIABLE;
			}
			else if (rte_wx_humidity_available == 1) {
				humidity_qf = HUMIDITY_QF_FULL;
			}
		}

		// wind quality factor
		if (rte_wx_wind_qf == AN_WIND_QF_UNKNOWN) {
			wind_qf = WIND_QF_FULL;
		}
		else {
			switch (rte_wx_wind_qf) {
			case AN_WIND_QF_FULL: wind_qf = WIND_QF_FULL; break;
			case AN_WIND_QF_DEGRADED_DEBOUNCE:
			case AN_WIND_QF_DEGRADED_SLEW:
			case AN_WIND_QF_DEGRADED: wind_qf = WIND_QF_DEGRADATED; break;
			case AN_WIND_QF_NOT_AVALIABLE:
			case AN_WIND_QF_UNKNOWN: wind_qf = WIND_QF_NOT_AVALIABLE; break;
			}

			rte_wx_wind_qf = AN_WIND_QF_UNKNOWN;
		}

		if (config_mode->victron == 1) {
			telemetry_send_values_pv (rx10m,
									  digi10m,
									  rte_pv_battery_current,
									  rte_pv_battery_voltage,
									  rte_pv_cell_voltage,
									  dallas_qf,
									  pressure_qf,
									  humidity_qf,
									  wind_qf);
		}
		else {

			// if _DALLAS_AS_TELEM will be enabled the fifth channel will be set to temperature
			// measured by DS12B20
			// telemetry_send_values(rx10m, tx10m, digi10m, kiss10m,
			// rte_wx_temperature_dallas_valid, dallas_qf, rte_wx_ms5611_qf, rte_wx_dht_valid.qf,
			// rte_wx_umb_qf);
			if (config_mode->wx == 1) {

				// if _METEO will be enabled, but without _DALLAS_AS_TELEM the fifth channel will be
				// used to transmit temperature from MS5611 which may be treated then as
				// 'rack/cabinet internal temperature'. Dallas DS12B10 will be used for ragular WX
				// frames
				telemetry_send_values (rx10m,
									   tx10m,
									   digi10m,
									   rte_main_average_battery_voltage,
									   digidrop10m,
									   rte_wx_temperature_internal_valid,
									   dallas_qf,
									   pressure_qf,
									   humidity_qf,
									   wind_qf,
									   pwr_save_currently_cutoff,
									   config_mode);
			}
			else {
				// if user will disable both _METEO and _DALLAS_AS_TELEM value will be zeroed
				// internally anyway
				telemetry_send_values (rx10m,
									   tx10m,
									   digi10m,
									   rte_main_average_battery_voltage,
									   digidrop10m,
									   0.0f,
									   dallas_qf,
									   pressure_qf,
									   humidity_qf,
									   wind_qf,
									   pwr_save_currently_cutoff,
									   config_mode);
			}
		}
		packet_tx_telemetry_counter = 0;

		// service external watchdog while sending telemetry
		io_ext_watchdog_service ();

		aprsis_prepare_telemetry (telemetry_get_counter (),
								  rx10m,
								  tx10m,
								  digi10m,
								  telemetry_scaled_vbatt_voltage,
								  digidrop10m,
								  telemetry_scaled_temperature,
								  telemetry_qf,
								  telemetry_degr,
								  telemetry_nav,
								  telemetry_pressure_qf_navaliable,
								  telemetry_humidity_qf_navaliable,
								  telemetry_anemometer_degradated,
								  telemetry_anemometer_navble,
								  telemetry_vbatt_low,
								  config_mode);

		if (rx10m == 0) {
			main_wait_for_tx_complete ();
			rte_main_reboot_req = io_reset_vbat_r ();
		}

		rx10m = 0, tx10m = 0, digi10m = 0, kiss10m = 0, digidrop10m = 0;
	}

	if (packet_tx_telemetry_descr_counter >= packet_tx_telemetry_descr_interval) {

		rte_main_trigger_gsm_telemetry_descriptions = 1;
		status_send_powersave_registers ();

		packet_tx_multi_per_call_handler ();

		if (config_mode->victron == 1) {
			telemetry_send_chns_description_pv (config_basic);

			if (rte_pv_battery_voltage == 0) {
				rte_main_reboot_req = 1;
			}
		}
		else {
			supervisor_iam_alive (SUPERVISOR_THREAD_MAIN_LOOP);

			telemetry_send_chns_description (config_basic, config_mode);

			supervisor_iam_alive (SUPERVISOR_THREAD_MAIN_LOOP);

			packet_tx_multi_per_call_handler ();
		}

		status_send ();

		if (config_mode->wx_umb == 1) {

			umb_clear_error_history (&rte_wx_umb_context);
		}

		if (main_config_data_gsm->aprsis_enable == 0 && main_config_data_gsm->api_enable == 1) {
			// and trigger API wx packet transmission
			packet_tx_trigger_tcp |= API_TRIGGER_STATUS;
		}
		else {
			packet_tx_trigger_tcp = 0;
		}

		packet_tx_telemetry_descr_counter = 0;
	}

	// store counters in backup registers
	backup_reg_set_packet_counters (packet_tx_beacon_counter,
									packet_tx_meteo_counter,
									packet_tx_meteo_gsm_counter);
}

void packet_tx_handler_increment_counters(void)
{
	// increase beacon transmit counters
	packet_tx_beacon_counter++;
	packet_tx_error_status_counter++;
	packet_tx_telemetry_counter++;
	packet_tx_telemetry_descr_counter++;
	if ((main_config_data_mode->wx & WX_ENABLED) == WX_ENABLED) {
		// increase these counters only when WX is enabled
		packet_tx_meteo_counter++;
		packet_tx_meteo_kiss_counter++;
		packet_tx_meteo_gsm_counter++;
	}
}

void packet_tx_get_current_counters (packet_tx_counter_values_t *out)
{

	if (out != 0x00) {
		out->beacon_counter = packet_tx_beacon_counter;
		out->wx_counter = packet_tx_meteo_counter;
		out->gsm_wx_counter = packet_tx_meteo_gsm_counter;
		out->telemetry_counter = packet_tx_telemetry_counter;
		out->telemetry_desc_counter = packet_tx_telemetry_descr_counter;
		out->kiss_counter = packet_tx_meteo_kiss_counter;
	}
}

void packet_tx_set_current_counters (packet_tx_counter_values_t *in)
{
	if (in != 0x00) {

		if (in->beacon_counter != 0)
			packet_tx_beacon_counter = in->beacon_counter;

		if (in->wx_counter != 0)
			packet_tx_meteo_counter = in->wx_counter;

		if (in->telemetry_counter != 0)
			packet_tx_telemetry_counter = in->telemetry_counter;

		if (in->telemetry_desc_counter != 0)
			packet_tx_telemetry_descr_counter = in->telemetry_desc_counter;

		if (in->kiss_counter != 0)
			packet_tx_meteo_kiss_counter = in->kiss_counter;

		if (in->gsm_wx_counter != 0)
			packet_tx_meteo_gsm_counter = in->gsm_wx_counter;
	}
	else {
		packet_tx_beacon_counter = 0;
		packet_tx_meteo_counter = 2;
		packet_tx_meteo_gsm_counter = 0;
		packet_tx_telemetry_counter = 0;
		packet_tx_telemetry_descr_counter = 10;
		packet_tx_meteo_kiss_counter = 0;
	}

	event_log_sync (EVENT_INFO,
					EVENT_SRC_PACKET_TX_HANDLER,
					EVENTS_TX_HANDLER_INFO_SET_CURRENT_COUNTERS,
					packet_tx_beacon_counter,
					packet_tx_meteo_counter,
					packet_tx_meteo_gsm_counter,
					packet_tx_telemetry_counter,
					packet_tx_telemetry_descr_counter,
					packet_tx_meteo_kiss_counter);
}

/**
 * Returns how many minutes is left to next weather packet (on radio network!!).
 * Used in power saving state machine and to periodically reset VHF radio
 * if this feature is enabled in configuration
 * @return
 */
int16_t packet_tx_get_minutes_to_next_wx (void)
{
	if (packet_tx_meteo_interval != 0) {
		return packet_tx_meteo_interval - packet_tx_meteo_counter;
	}
	else {
		return -1;
	}
}

/**
 * This function checks if at the moment any weather packet is scheduled to be sent
 * to APRS-IS server, or this packet has been just sent and GPRS module probably
 * communicate with GSM network now. The result of this check is used to
 * inhibit power saving state machine temporary, not to turn off or disable
 * GSM modem while it is talking with GSM radio network.
 * @return
 */
uint8_t packet_tx_is_gsm_meteo_pending (void)
{
	uint8_t out = 0;

	if (gsm_sim800_gprs_ready == 1 && (packet_tx_trigger_tcp & APRSIS_TRIGGER_METEO) != 0) {
		out = 1;
	}

	if (packet_tx_meteo_gsm_has_been_sent != 0) {
		out = 1;
	}

	return out;
}

void packet_tx_force_gsm_status (void)
{
	packet_tx_gsm_status_sent = 0;
}

/**
 * Callback used by @link{pwr_save_pooling_handler} in case of change in current
 * powersave mode due to schedule or battery voltage
 * @param non_aggressive_or_aggressive
 * @return new value of packet_tx_meteo_interval
 */
uint8_t packet_tx_changed_powersave_callback (uint8_t non_aggressive_or_aggressive)
{
	if (non_aggressive_or_aggressive == 0) {
		packet_tx_meteo_interval = packet_tx_meteo_non_aggresive_interval;
		packet_tx_telemetry_interval = 10;
	}
	else {
		packet_tx_meteo_interval = packet_tx_meteo_aggresive_interval;

		if (packet_tx_meteo_aggresive_interval < 10) {
			packet_tx_telemetry_interval = 2 * packet_tx_meteo_aggresive_interval;
		}
		else {
			packet_tx_telemetry_interval = packet_tx_meteo_aggresive_interval;
		}
	}

	packet_tx_meteo_counter = 0;
	packet_tx_telemetry_counter = 0;

	return packet_tx_meteo_interval;
}

/**
 * As name should explain this returns current value of meteo packet counter
 * @return
 */
uint8_t packet_tx_get_meteo_counter (void)
{
	return packet_tx_meteo_counter;
}

uint8_t packet_tx_get_trigger_tcp (void)
{
	return packet_tx_trigger_tcp;
}
