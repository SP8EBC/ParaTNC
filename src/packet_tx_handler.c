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

#ifdef _METEO
uint8_t packet_tx_meteo_interval = _WX_INTERVAL;
uint8_t packet_tx_meteo_counter = 0;
#endif

uint8_t packet_tx_telemetry_interval = 10;
uint8_t packet_tx_telemetry_counter = 0;

// this shall be called in 60 seconds periods
void packet_tx_handler(void) {
	packet_tx_beacon_counter++;
	packet_tx_telemetry_counter++;
#ifdef _METEO
	packet_tx_meteo_counter++;
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
#endif

	if (packet_tx_telemetry_counter >= packet_tx_telemetry_interval) {

#if defined _DALLAS_AS_TELEM
		// if _DALLAS_AS_TELEM will be enabled the fifth channel will be set to temperature measured by DS12B20
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, rte_wx_temperature_dallas_valid, rte_wx_dallas_qf, rte_wx_ms5611_qf, rte_wx_dht_valid.qf);
#elif defined _METEO
		// if _METEO will be enabled, but without _DALLAS_AS_TELEM the fifth channel will be used to transmit temperature from MS5611
		// which may be treated then as 'rack/cabinet internal temperature'. Dallas DS12B10 will be used for ragular WX frames
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, rte_wx_temperature_valid, rte_wx_dallas_qf, rte_wx_ms5611_qf, rte_wx_dht.qf);
#else
		// if user will disable both _METEO and _DALLAS_AS_TELEM value will be zeroed internally anyway
		telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, 0.0f, rte_wx_dallas_qf, rte_wx_ms5611_qf, rte_wx_dht.qf);
#endif

		main_wait_for_tx_complete();

		packet_tx_telemetry_counter = 0;

		rx10m = 0, tx10m = 0, digi10m = 0, kiss10m = 0;

	}

}
