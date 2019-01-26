#include "packet_tx_handler.h"
#include "station_config.h"
#include "rte_wx.h"

#include "./aprs/beacon.h"
#include "./aprs/wx.h"
#include "./aprs/telemetry.h"

#include "./drivers/tx20.h"

#include "main.h"

uint8_t packet_tx_beacon_interval = _BCN_INTERVAL;
uint8_t packet_tx_beacon_counter = 0;

uint8_t packet_tx_meteo_interval = _WX_INTERVAL;
uint8_t packet_tx_meteo_counter = 0;

uint8_t packet_tx_telemetry_interval = 10;
uint8_t packet_tx_telemetry_counter = 0;

// this shall be called in 60 seconds periods
void packet_tx_handler(void) {
	packet_tx_beacon_counter++;
	packet_tx_meteo_counter++;
	packet_tx_telemetry_counter++;

	if (packet_tx_beacon_counter >= packet_tx_beacon_interval) {

		SendOwnBeacon();

		main_wait_for_tx_complete();

		packet_tx_beacon_counter = 0;
	}

	if (packet_tx_meteo_counter >= packet_tx_meteo_interval) {

		SendWXFrame(&VNAME, rte_wx_temperature_dallas_valid, rte_wx_pressure_valid);

		main_wait_for_tx_complete();

		packet_tx_meteo_counter = 0;
	}

	if (packet_tx_telemetry_counter >= packet_tx_telemetry_interval) {



		main_wait_for_tx_complete();

		packet_tx_telemetry_counter = 0;

	}

}
