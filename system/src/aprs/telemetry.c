/*
 * telemetry.c
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */

#include "aprs/telemetry.h"
#include "main.h"
#include "station_config.h"
#include "delay.h"

#include "ve_direct_protocol/parser.h"

#include <main.h>
#include <stdio.h>

uint16_t telemetry_counter = 0;

#ifdef _VICTRON
void telemetry_send_chns_description_pv(void) {
	while (main_afsk.sending == 1);

#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :PARM.Rx10min,Digi10min,BatAmps,BatVolt,PvVolt,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,MS_QF_NAVBLE,DHT_QF_NAVBLE,TX20_SLEW", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :PARM.Rx10min,Digi10min,BatAmps,BatVolt,PvVolt,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,MS_QF_NAVBLE,DHT_QF_NAVBLE,TX20_SLEW", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:PARM.Rx10min,Digi10min,BatAmps,BatVolt,PvVolt,DS_QF_FULL,DS_QF_DEGRADA,DS_QF_NAVBLE,MS_QF_NAVBLE,DHT_QF_NAVBLE,TX20_SLEW", _CALL, _SSID);
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	while (main_afsk.sending == 1);

	delay_fixed(1200);

#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :EQNS.0,1,0,0,1,0,0,0.07,-8,0,0.07,4,0,0.07,4", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :EQNS.0,1,0,0,1,0,0,0.07,-8,0,0.07,4,0,0.07,4", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:EQNS.0,1,0,0,1,0,0,0.07,-8,0,0.07,4,0,0.07,4", _CALL, _SSID);
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	while (main_afsk.sending == 1);

	delay_fixed(1200);

#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :UNIT.Pkt,Pkt,A,V,V,Hi,Hi,Hi,Hi,Hi,Hi", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :UNIT.Pkt,Pkt,A,V,V,Hi,Hi,Hi,Hi,Hi,Hi", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:UNIT.Pkt,Pkt,A,V,V,Hi,Hi,Hi,Hi,Hi,Hi", _CALL, _SSID);
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	delay_fixed(1200);
}

void telemetry_send_values_pv (	uint8_t rx_pkts,
								uint8_t digi_pkts,
								int16_t raw_battery_current,
								uint16_t raw_battery_voltage,
								uint16_t raw_pv_cell_voltage,
								dallas_qf_t dallas_qf,
								ms5611_qf_t ms_qf,
								dht22QF ds_qf)
{
	// local variables with characters to be inserted to APRS telemetry frame
	char qf = '0', degr = '0', nav = '0';
	char ms_qf_navaliable = '0';
	char dht_qf_navaliable = '0';

	uint8_t scaled_battery_current = 0;
	uint8_t scaled_battery_voltage = 0;
	uint8_t scaled_pvcell_volage = 0;

	float phy_battery_current = 0.0f;
	float phy_battery_voltage = 0.0f;
	float phy_pvcell_voltage = 0.0f;

	phy_battery_current = (float)raw_battery_current / 1000.0f;
	phy_battery_voltage = (float)raw_battery_voltage / 1000.0f;
	phy_pvcell_voltage = (float)raw_pv_cell_voltage / 1000.0f;

	scaled_battery_current = (uint8_t) roundf((phy_battery_current + 8.0f) * 14.2857f);
	scaled_battery_voltage = (uint8_t) roundf((phy_battery_voltage - 4.0f) * 14.2857f);
	scaled_pvcell_volage = (uint8_t) roundf((phy_pvcell_voltage - 4.0f) * 14.2857f);

	switch (dallas_qf) {
	case DALLAS_QF_FULL:
		qf = '1', degr = '0', nav = '0';
		break;
	case DALLAS_QF_DEGRADATED:
		qf = '0', degr = '1', nav = '0';
		break;

	case DALLAS_QF_NOT_AVALIABLE:
		qf = '0', degr = '0', nav = '1';
		break;

	default:
		qf = '0', degr = '0', nav = '0';
		break;
	}

	switch (ms_qf) {
	case MS5611_QF_NOT_AVALIABLE:
	case MS5611_QF_UNKNOWN:
		 ms_qf_navaliable = '1';
		 break;
	default:
		ms_qf_navaliable = '0';
		break;
	}

	switch (ds_qf) {
	case DHT22_QF_UNAVALIABLE:
	case DHT22_QF_UNKNOWN:
		dht_qf_navaliable = '1';
		break;
	default:
		dht_qf_navaliable = '0';
	}

	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c000", telemetry_counter++, rx_pkts, digi_pkts, scaled_battery_current, scaled_battery_voltage, scaled_pvcell_volage, qf, degr, nav, ms_qf_navaliable, dht_qf_navaliable);


	if (telemetry_counter > 999)
		telemetry_counter = 0;
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
//	while(ax25.dcd == true);


	afsk_txStart(&main_afsk);


}

void telemetry_send_status(ve_direct_average_struct* avg, ve_direct_error_reason* last_error, ve_direct_system_state state) {
	char string_buff_err[24], string_buff_state[23];

	ve_direct_state_to_string(state, string_buff_state, 23);
	ve_direct_error_to_string(*last_error, string_buff_err, 24);

	main_own_aprs_msg_len = snprintf(main_own_aprs_msg, sizeof(main_own_aprs_msg), "> FwVersion %s BatAmpsMin %d BatAmpsMax %d %s %s", SW_VER, avg->min_battery_current, avg->max_battery_current, string_buff_state, string_buff_err);
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	afsk_txStart(&main_afsk);

	avg->max_battery_current = 0;
	avg->min_battery_current = 0;
	*last_error = ERR_UNINITIALIZED;
}

#else

void telemetry_send_chns_description(void) {
	while (main_afsk.sending == 1);

	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,MS_QF_NAVBLE,DHT_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,MS_QF_NAVBLE,DHT_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRADA,DS_QF_NAVBLE,MS_QF_NAVBLE,DHT_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", _CALL, _SSID);
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	while (main_afsk.sending == 1);

	delay_fixed(1200);

#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.25,-25", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.25,-25", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.25,-25", _CALL, _SSID);
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	while (main_afsk.sending == 1);

	delay_fixed(1200);

#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi", _CALL, _SSID);
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	delay_fixed(1200);

}

void telemetry_send_values(	uint8_t rx_pkts,
							uint8_t tx_pkts,
							uint8_t digi_pkts,
							uint8_t kiss_pkts,
							float temperature,
							dallas_qf_t dallas_qf,
							ms5611_qf_t ms_qf,
							dht22QF ds_qf,
							umb_qf_t anemometer_qf) {


	// local variables with characters to be inserted to APRS telemetry frame
	char qf = '0', degr = '0', nav = '0';
	char ms_qf_navaliable = '0';
	char dht_qf_navaliable = '0';
	char anemometer_degradated = '0';
	char anemometer_navble = '0';

	uint8_t scaled_temperature = 0;

	if (anemometer_qf == UMB_QF_DEGRADED) {
		anemometer_degradated = '1';
		anemometer_navble = '0';
	}
	else if (anemometer_qf == UMB_QF_NOT_AVALIABLE) {
		anemometer_degradated = '0';
		anemometer_navble = '1';
	}
	else if (anemometer_qf == UMB_QF_UNITIALIZED || anemometer_qf == UMB_QF_UNKNOWN) {
		anemometer_degradated = '1';
		anemometer_navble = '1';
	}

	if (temperature < -25.0f) {
		scaled_temperature = (uint8_t)0;
	}
	else if (temperature > 38.75f) {
		scaled_temperature = (uint8_t)255;
	}
	else {
		scaled_temperature = (uint8_t)((temperature + 25.0f) * 4.0f);
	}

	switch (dallas_qf) {
	case DALLAS_QF_FULL:
		qf = '1', degr = '0', nav = '0';
		break;
	case DALLAS_QF_DEGRADATED:
		qf = '0', degr = '1', nav = '0';
		break;

	case DALLAS_QF_NOT_AVALIABLE:
		qf = '0', degr = '0', nav = '1';
		break;

	default:
		qf = '0', degr = '0', nav = '0';
		break;
	}

	switch (ms_qf) {
	case MS5611_QF_NOT_AVALIABLE:
	case MS5611_QF_UNKNOWN:
		 ms_qf_navaliable = '1';
		 break;
	default:
		ms_qf_navaliable = '0';
		break;
	}

	switch (ds_qf) {
	case DHT22_QF_UNAVALIABLE:
	case DHT22_QF_UNKNOWN:
		dht_qf_navaliable = '1';
		break;
	default:
		dht_qf_navaliable = '0';
	}

	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

#ifdef _DALLAS_AS_TELEM
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, kiss_pkts, scaled_temperature, qf, degr, nav, ms_qf_navaliable, dht_qf_navaliable, anemometer_degradated, anemometer_navble);
#else
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, kiss_pkts, scaled_temperature, qf, degr, nav, ms_qf_navaliable, dht_qf_navaliable, anemometer_degradated, anemometer_navble);
#endif

	if (telemetry_counter > 999)
		telemetry_counter = 0;
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
//	while(ax25.dcd == true);


	afsk_txStart(&main_afsk);
}

void telemetry_send_status(void) {
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ">ParaTNC firmware %s-%s by SP8EBC", SW_VER, SW_DATE);
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	afsk_txStart(&main_afsk);

}

#endif



