#include "packet_tx_handler.h"
#include "station_config.h"
#include "rte_wx.h"
#include "rte_pv.h"

#include "./aprs/beacon.h"
#include "./aprs/wx.h"
#include "./aprs/telemetry.h"

#include "./drivers/tx20.h"
#include "./drivers/serial.h"

#include "./umb_master/umb_master.h"

#include "main.h"

uint8_t packet_tx_beacon_interval = _BCN_INTERVAL;
uint8_t packet_tx_beacon_counter = 0;

uint8_t packet_tx_error_status_interval = 2;
uint8_t packet_tx_error_status_counter = 0;

#ifdef _METEO
uint8_t packet_tx_meteo_interval = _WX_INTERVAL;
uint8_t packet_tx_meteo_counter = 0;

uint8_t packet_tx_meteo_kiss_interval = 2;
uint8_t packet_tx_meteo_kiss_counter = 0;
#endif

uint8_t packet_tx_telemetry_interval = 10;
uint8_t packet_tx_telemetry_counter = 0;

uint8_t packet_tx_telemetry_descr_interval = 150;
uint8_t packet_tx_telemetry_descr_counter = 145;

// this shall be called in 60 seconds periods
void packet_tx_handler(void) {
	dallas_qf_t dallas_qf = DALLAS_QF_UNKNOWN;

	pressure_qf_t pressure_qf = PRESSURE_QF_UNKNOWN;
	humidity_qf_t humidity_qf = HUMIDITY_QF_UNKNOWN;
	wind_qf_t wind_qf = WIND_QF_UNKNOWN;

	int ln = 0;

	packet_tx_beacon_counter++;
	packet_tx_error_status_counter++;
	packet_tx_telemetry_counter++;
	packet_tx_telemetry_descr_counter++;
#ifdef _METEO
	packet_tx_meteo_counter++;
	packet_tx_meteo_kiss_counter++;
#endif

	if (packet_tx_error_status_counter >= packet_tx_error_status_interval) {
	#if defined(_UMB_MASTER)
		umb_construct_status_str(&rte_wx_umb_context, main_own_aprs_msg, sizeof(main_own_aprs_msg), &ln, master_time);

		// if there is anything to send
		if (ln > 0) {
			beacon_send_from_user_content(ln, main_own_aprs_msg);

			main_wait_for_tx_complete();

		}
	#endif

		packet_tx_error_status_counter = 0;
	}

	if (packet_tx_beacon_counter >= packet_tx_beacon_interval) {

		beacon_send_own();

		main_wait_for_tx_complete();

		packet_tx_beacon_counter = 0;
	}

#ifdef _METEO
	if (packet_tx_meteo_counter >= packet_tx_meteo_interval) {

#if defined _DALLAS_AS_TELEM
		// _DALLAS_AS_TELEM wil be set during compilation wx packets will be filled by temperature from MS5611 sensor
		//SendWXFrame(&VNAME, rte_wx_temperature_valid, rte_wx_pressure_valid);
		SendWXFrame(rte_wx_average_windspeed, rte_wx_max_windspeed, rte_wx_average_winddirection, rte_wx_temperature_ms, rte_wx_pressure_valid, rte_wx_humidity_valid);
#else
		SendWXFrame(rte_wx_average_windspeed, rte_wx_max_windspeed, rte_wx_average_winddirection, rte_wx_temperature_average_dallas_valid, rte_wx_pressure_valid, rte_wx_humidity_valid);


#endif

		main_wait_for_tx_complete();

		packet_tx_meteo_counter = 0;
	}

	if (packet_tx_meteo_kiss_counter >= packet_tx_meteo_kiss_interval) {

		srl_wait_for_tx_completion(main_kiss_srl_ctx_ptr);

		SendWXFrameToBuffer(rte_wx_average_windspeed, rte_wx_max_windspeed, rte_wx_average_winddirection, rte_wx_temperature_average_dallas_valid, rte_wx_pressure_valid, rte_wx_humidity_valid, srl_usart1_tx_buffer, TX_BUFFER_1_LN, &ln);

		srl_start_tx(main_kiss_srl_ctx_ptr, ln);


		packet_tx_meteo_kiss_counter = 0;
	}
#endif // #ifdef _METEO

	if (packet_tx_telemetry_counter >= packet_tx_telemetry_interval) {

		// if there weren't any erros related to communication with DS12B20 from previous function call
		if (rte_wx_error_dallas_qf == DALLAS_QF_UNKNOWN) {
			dallas_qf = rte_wx_current_dallas_qf;	// it might be DEGRADATED so we need to copy a value directly

			// reset current QF to check if there will be at least one successfull readout of temperature
			rte_wx_current_dallas_qf = DALLAS_QF_UNKNOWN;
		}

		// if there were any errors
		else {

			// if we had at least one successfull communication with the sensor
			if (rte_wx_current_dallas_qf == DALLAS_QF_FULL || rte_wx_current_dallas_qf == DALLAS_QF_DEGRADATED) {
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

		// pressure sensors quality factors
		if (rte_wx_ms5611_qf == MS5611_QF_UNKNOWN) {
			// use BME280 is used instead
			switch (rte_wx_bme280_qf) {
				case BME280_QF_FULL:
				case BME280_QF_HUMIDITY_DEGRADED: pressure_qf = PRESSURE_QF_FULL; break;
				case BME280_QF_UKNOWN:
				case BME280_QF_NOT_AVAILABLE: pressure_qf = PRESSURE_QF_NOT_AVALIABLE; break;
				case BME280_QF_PRESSURE_DEGRADED:
				case BME280_QF_GEN_DEGRADED: pressure_qf = PRESSURE_QF_DEGRADATED; break;
			}
		}
		else {
			// if not use MS5611
			switch (rte_wx_ms5611_qf) {
				case MS5611_QF_FULL: pressure_qf = PRESSURE_QF_FULL; break;
				case MS5611_QF_NOT_AVALIABLE: pressure_qf = PRESSURE_QF_NOT_AVALIABLE; break;
				case MS5611_QF_DEGRADATED: pressure_qf = PRESSURE_QF_DEGRADATED; break;
				case MS5611_QF_UNKNOWN: pressure_qf = PRESSURE_QF_UNKNOWN; break;
			}
		}

		// humidity sensors quality factors
		if (rte_wx_bme280_qf == BME280_QF_UKNOWN) {
			// use DHT22
			switch(rte_wx_dht_valid.qf) {
				case DHT22_QF_UNKNOWN: humidity_qf = HUMIDITY_QF_UNKNOWN; break;
				case DHT22_QF_FULL: humidity_qf = HUMIDITY_QF_FULL; break;
				case DHT22_QF_DEGRADATED: humidity_qf = HUMIDITY_QF_DEGRADATED; break;
				case DHT22_QF_UNAVALIABLE: humidity_qf = HUMIDITY_QF_NOT_AVALIABLE; break;
			}
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
		}

		// wind quality factor
		if (rte_wx_analog_wind_qf == AN_WIND_QF_UNKNOWN) {

		}
		else {
			switch (rte_wx_analog_wind_qf) {
				case AN_WIND_QF_FULL: wind_qf = WIND_QF_FULL; break;
				case AN_WIND_QF_DEGRADED_DEBOUNCE:
				case AN_WIND_QF_DEGRADED_SLEW:
				case AN_WIND_QF_DEGRADED: wind_qf = WIND_QF_DEGRADATED; break;
				case AN_WIND_QF_NOT_AVALIABLE:
				case AN_WIND_QF_UNKNOWN: wind_qf = WIND_QF_NOT_AVALIABLE; break;
			}
		}

#ifdef _VICTRON
//
		telemetry_send_values_pv(rx10m, digi10m, rte_pv_battery_current, rte_pv_battery_voltage, rte_pv_cell_voltage, dallas_qf, pressure_qf, humidity_qf);
//
#else
//
#if defined _DALLAS_AS_TELEM
		// if _DALLAS_AS_TELEM will be enabled the fifth channel will be set to temperature measured by DS12B20
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, rte_wx_temperature_dallas_valid, dallas_qf, rte_wx_ms5611_qf, rte_wx_dht_valid.qf, rte_wx_umb_qf);
#elif defined _METEO
		// if _METEO will be enabled, but without _DALLAS_AS_TELEM the fifth channel will be used to transmit temperature from MS5611
		// which may be treated then as 'rack/cabinet internal temperature'. Dallas DS12B10 will be used for ragular WX frames
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, rte_wx_temperature_ms_valid, dallas_qf, pressure_qf, humidity_qf, wind_qf);
#else
		// if user will disable both _METEO and _DALLAS_AS_TELEM value will be zeroed internally anyway
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, 0.0f, dallas_qf, pressure_qf, humidity_qf, wind_qf);
#endif
//
#endif
		main_wait_for_tx_complete();

		packet_tx_telemetry_counter = 0;

		rx10m = 0, tx10m = 0, digi10m = 0, kiss10m = 0;

		rte_wx_tx20_excessive_slew_rate = 0;

	}

	if (packet_tx_telemetry_descr_counter >= packet_tx_telemetry_descr_interval) {
#ifdef _VICTRON
		telemetry_send_chns_description_pv();

		main_wait_for_tx_complete();

		telemetry_send_status(&rte_pv_average, &rte_pv_last_error, rte_pv_struct.system_state);

		main_wait_for_tx_complete();
#else
		telemetry_send_chns_description();

		main_wait_for_tx_complete();

		telemetry_send_status();

		main_wait_for_tx_complete();
#endif
#if defined _UMB_MASTER
		umb_clear_error_history(&rte_wx_umb_context);
#endif
		packet_tx_telemetry_descr_counter = 0;
	}

}
