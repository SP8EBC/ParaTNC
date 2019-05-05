#include "packet_tx_handler.h"
#include "station_config.h"
#include "rte_wx.h"
#include "rte_pv.h"

#include "./aprs/beacon.h"
#include "./aprs/wx.h"
#include "./aprs/telemetry.h"

#include "./drivers/tx20.h"
#include "./drivers/serial.h"

#include "main.h"

uint8_t packet_tx_beacon_interval = _BCN_INTERVAL;
uint8_t packet_tx_beacon_counter = 0;

#ifdef _METEO
uint8_t packet_tx_meteo_interval = _WX_INTERVAL;
uint8_t packet_tx_meteo_counter = 0;

uint8_t packet_tx_meteo_kiss_interval = 2;
uint8_t packet_tx_meteo_kiss_counter = 0;
#endif

uint8_t packet_tx_telemetry_interval = 10;
uint8_t packet_tx_telemetry_counter = 0;

uint8_t packet_tx_telemetry_descr_interval = 40;
uint8_t packet_tx_telemetry_descr_counter = 35;

// this shall be called in 60 seconds periods
void packet_tx_handler(void) {
	DallasQF dallas_qf = DALLAS_QF_UNKNOWN;

	uint16_t ln = 0;

	packet_tx_beacon_counter++;
	packet_tx_telemetry_counter++;
	packet_tx_telemetry_descr_counter++;
#ifdef _METEO
	packet_tx_meteo_counter++;
	packet_tx_meteo_kiss_counter++;
#endif

	if (packet_tx_beacon_counter >= packet_tx_beacon_interval) {

		SendOwnBeacon();

		main_wait_for_tx_complete();

		packet_tx_beacon_counter = 0;
	}

#ifdef _METEO
	if (packet_tx_meteo_counter >= packet_tx_meteo_interval) {

#if defined _DALLAS_AS_TELEM
		// _DALLAS_AS_TELEM wil be set during compilation wx packets will be filled by temperature from MS5611 sensor
		SendWXFrame(&VNAME, rte_wx_temperature_valid, rte_wx_pressure_valid);
#else
		SendWXFrame(&VNAME, rte_wx_temperature_dallas_valid, rte_wx_pressure_valid);
#endif
		main_wait_for_tx_complete();

		packet_tx_meteo_counter = 0;
	}

	if (packet_tx_meteo_kiss_counter >= packet_tx_meteo_kiss_interval) {

		srl_wait_for_tx_completion();

		SendWXFrameToBuffer(&VNAME, rte_wx_temperature_dallas_valid, rte_wx_pressure_valid, srl_tx_buffer, TX_BUFFER_LN, &ln);

		srl_start_tx(ln);


		packet_tx_meteo_kiss_counter = 0;
	}
#endif

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

#ifdef _VICTRON
//
		telemetry_send_values_pv(rx10m, digi10m, rte_pv_battery_current, rte_pv_battery_voltage, rte_pv_cell_voltage, dallas_qf, rte_wx_ms5611_qf, rte_wx_dht_valid.qf);
//
#else
//
#if defined _DALLAS_AS_TELEM
		// if _DALLAS_AS_TELEM will be enabled the fifth channel will be set to temperature measured by DS12B20
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, rte_wx_temperature_dallas_valid, dallas_qf, rte_wx_ms5611_qf, rte_wx_dht_valid.qf, rte_wx_tx20_excessive_slew_rate);
#elif defined _METEO
		// if _METEO will be enabled, but without _DALLAS_AS_TELEM the fifth channel will be used to transmit temperature from MS5611
		// which may be treated then as 'rack/cabinet internal temperature'. Dallas DS12B10 will be used for ragular WX frames
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, rte_wx_temperature_valid, dallas_qf, rte_wx_ms5611_qf, rte_wx_dht.qf, rte_wx_tx20_excessive_slew_rate);
#else
		// if user will disable both _METEO and _DALLAS_AS_TELEM value will be zeroed internally anyway
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, 0.0f, dallas_qf, rte_wx_ms5611_qf, rte_wx_dht.qf);
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


		packet_tx_telemetry_descr_counter = 0;
	}

}
