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

#include "modbus_rtu/rtu_getters.h"

#include <main.h>
#include <stdio.h>
#include <string.h>

uint16_t telemetry_counter = 0;

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
	while (main_ax25.dcd == 1);

	afsk_txStart(&main_afsk);

}

void telemetry_send_status_pv(ve_direct_average_struct* avg, ve_direct_error_reason* last_error, ve_direct_system_state state, uint32_t master_time, uint16_t messages_count, uint16_t corrupted_messages_count) {
	char string_buff_err[24], string_buff_state[23];

	ve_direct_state_to_string(state, string_buff_state, 23);
	ve_direct_error_to_string(*last_error, string_buff_err, 24);

	main_own_aprs_msg_len = snprintf(main_own_aprs_msg, sizeof(main_own_aprs_msg), ">MT %X, MC %X, CMC %X, IMIN %d, IMAX %d, ST %s, ERR %s", master_time, (uint32_t)messages_count, (uint32_t)corrupted_messages_count, avg->min_battery_current, avg->max_battery_current, string_buff_state, string_buff_err);
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	afsk_txStart(&main_afsk);

	main_wait_for_tx_complete();

	avg->max_battery_current = 0;
	avg->min_battery_current = 0;
	*last_error = ERR_UNINITIALIZED;
}
////

/**
 * Sends four frames with telemetry description
 */
void telemetry_send_chns_description(void) {

	// a buffer to assembly the 'call-ssid' string at the begining of the frame
	char message_prefix_buffer[9];

	memset(message_prefix_buffer, 0x00, 0x09);

	sprintf(message_prefix_buffer, "%s-%d", _CALL, _SSID);

	// wait for any RF transmission to finish
	main_wait_for_tx_complete();

	// clear the output frame buffer
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

	// prepare a frame with channel names depending on SSID
#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-6s   :PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-9s:PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", message_prefix_buffer);
#endif
#if (_SSID > 9 && _SSID <= 15)
main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-9s:PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", message_prefix_buffer);
#endif

	// place a null terminator at the end
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;

	// prepare transmission
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;

	// key up the transmitter and
	afsk_txStart(&main_afsk);

	main_wait_for_tx_complete();

	delay_fixed(1500);

	while (main_ax25.dcd == 1);

#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-6s   :EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.5,-50", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-9s:EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.5,-50", message_prefix_buffer);
#endif
#if (_SSID > 9 && _SSID <= 15)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-9s:EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.5,-50", message_prefix_buffer);
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	main_wait_for_tx_complete();

	delay_fixed(1500);

	while (main_ax25.dcd == 1);

#if (_SSID == 0)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-6s   :UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-9s:UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi", message_prefix_buffer);
#endif
#if (_SSID > 9 && _SSID <= 15)
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ":%-9s:UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi", message_prefix_buffer);
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);
//
//	main_wait_for_tx_complete();
//
//	delay_fixed(1500);
//
//	while (main_ax25.dcd == 1);

}

/**
 * This function sends telemetry values in 'typical configuration' when VICTRON VE.direct protocol parser is not enabled.
 */
void telemetry_send_values(	uint8_t rx_pkts,
							uint8_t tx_pkts,
							uint8_t digi_pkts,
							uint8_t kiss_pkts,
							float temperature,
							dallas_qf_t dallas_qf,
							pressure_qf_t press_qf,
							humidity_qf_t humid_qf,
							wind_qf_t anemometer_qf) {


	// local variables with characters to be inserted to APRS telemetry frame
	char qf = '0', degr = '0', nav = '0';
	char pressure_qf_navaliable = '0';
	char humidity_qf_navaliable = '0';
	char anemometer_degradated = '0';
	char anemometer_navble = '0';

	// temperature scaled to 0x00-0xFF range for fifth telemetry channel.
	// if _METEO mode is enabled this channel sends the temperaure measure by
	// internal MS5611 or BME280. If _METEO is not enabled this channel
	// could send Dallas DS18B20 masurements if this is enabled in station_config.h
	uint8_t scaled_temperature = 0;

	// get the quality factor for wind measurements
	if (anemometer_qf == WIND_QF_DEGRADATED) {
		anemometer_degradated = '1';
		anemometer_navble = '0';
	}
	else if (anemometer_qf == WIND_QF_NOT_AVALIABLE) {
		anemometer_degradated = '0';
		anemometer_navble = '1';
	}
	else if (anemometer_qf == WIND_QF_UNKNOWN) {
		anemometer_degradated = '1';
		anemometer_navble = '1';
	}

	// scale the physical temperature and limit upper and lower boundary if
	// it is required
	if (temperature < -50.0f) {
		scaled_temperature = (uint8_t)0;
	}
	else if (temperature > 70.0f) {
		scaled_temperature = (uint8_t)255;
	}
	else {
		scaled_temperature = (uint8_t)((temperature + 50.0f) * 2.0f);
	}

	// set the quality factor for dallas DS18B20
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

	// set the quality factor for pressure
	switch (press_qf) {
	case PRESSURE_QF_NOT_AVALIABLE:
	case PRESSURE_QF_UNKNOWN:
		 pressure_qf_navaliable = '1';
		 break;
	default:
		pressure_qf_navaliable = '0';
		break;
	}

	switch (humid_qf) {
	case HUMIDITY_QF_UNKNOWN:
	case HUMIDITY_QF_NOT_AVALIABLE:
		humidity_qf_navaliable = '1';
		break;
	default:
		humidity_qf_navaliable = '0';
	}

	// reset the buffer where the frame will be contructed and stored for transmission
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

	// generate the telemetry frame from values
#ifdef _DALLAS_AS_TELEM
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, kiss_pkts, scaled_temperature, qf, degr, nav, pressure_qf_navaliable, humidity_qf_navaliable, anemometer_degradated, anemometer_navble);
#else
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, kiss_pkts, scaled_temperature, qf, degr, nav, pressure_qf_navaliable, humidity_qf_navaliable, anemometer_degradated, anemometer_navble);
#endif

	// reset the frame counter if it overflowed
	if (telemetry_counter > 999)
		telemetry_counter = 0;

	// put a null terminator at the end of frame (but it should be placed there anyway)
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;

	// wait for completing any previous transmission (afsk_txStart will exit with failure if the modem is transmitting)
	main_wait_for_tx_complete();

	// prepare transmission of the frame
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);

 	// ??
	after_tx_lock = 1;

	// check if RF channel is free from other transmissions and wait for the clearance if it is needed
	while (main_ax25.dcd == 1);

	// key up a transmitter and start transmission
	afsk_txStart(&main_afsk);

}

void telemetry_send_status(void) {
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, ">ParaTNC firmware %s-%s by SP8EBC", SW_VER, SW_DATE);
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	while (main_ax25.dcd == 1);
	afsk_txStart(&main_afsk);

}


void telemetry_send_status_raw_values_modbus(void) {
#ifdef _MODBUS_RTU
	uint8_t status_ln = 0;

	rtu_get_raw_values_string(main_own_aprs_msg, OWN_APRS_MSG_LN, &status_ln);

 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, status_ln);
	while (main_ax25.dcd == 1);
	afsk_txStart(&main_afsk);
	main_wait_for_tx_complete();
#endif
}



