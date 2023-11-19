/*
 * telemetry.c
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */

#include "aprs/telemetry.h"
#include "main.h"
#include "delay.h"
#include "backup_registers.h"
#include "variant.h"
#include "rte_main.h"

#include "ve_direct_protocol/parser.h"

#include "modbus_rtu/rtu_getters.h"

#include <main.h>
#include <stdio.h>
#include <string.h>

static uint16_t telemetry_counter = 0;

// variables with characters to be inserted to APRS telemetry frame
char telemetry_qf = '0';
char telemetry_degr = '0';
char telemetry_nav = '0';
char telemetry_pressure_qf_navaliable = '0';
char telemetry_humidity_qf_navaliable = '0';
char telemetry_anemometer_degradated = '0';
char telemetry_anemometer_navble = '0';
char telemetry_vbatt_low = '0';

// temperature scaled to 0x00-0xFF range for fifth telemetry channel.
// if _METEO mode is enabled this channel sends the temperaure measure by
// internal MS5611 or BME280. If _METEO is not enabled this channel
// could send Dallas DS18B20 masurements if this is enabled in station_config.h
uint8_t telemetry_scaled_temperature = 0;

uint8_t telemetry_scaled_vbatt_voltage = 0;

void telemetry_init(void) {
	telemetry_counter = backup_reg_get_telemetry();
}

int telemetry_create_description_string(const config_data_basic_t * const config_basic, const telemetry_description_t what, char * out, uint16_t out_ln) {

	// real size of a string put into a buffer pointed by 'out'
	int out_size = 0;

	// a buffer to assembly the 'call-ssid' string at the begining of the frame
	char message_prefix_buffer[9];

	memset(message_prefix_buffer, 0x00, 0x09);

	sprintf(message_prefix_buffer, "%s-%d", config_basic->callsign, config_basic->ssid);

	if (variant_validate_is_within_ram((uint32_t)out) == 0) {
		return out_size;
	}

	switch (what) {
		case TELEMETRY_PV_PARM: {

			if (config_basic->ssid == 0)
				out_size = snprintf(out, out_ln, ":%-6s   :PARM.Rx10min,Digi10min,BatAmps,BatVolt,PvVolt,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", config_basic->callsign);
			else if (config_basic->ssid > 0 && config_basic->ssid < 10)
				out_size = snprintf(out, out_ln, ":%-9s:PARM.Rx10min,Digi10min,BatAmps,BatVolt,PvVolt,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", message_prefix_buffer);
			else if (config_basic->ssid >= 10 && config_basic->ssid < 16)
				out_size = snprintf(out, out_ln, ":%-9s:PARM.Rx10min,Digi10min,BatAmps,BatVolt,PvVolt,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB", message_prefix_buffer);
			else
				return out_size;

			break;
		}

		case TELEMETRY_PV_EQNS: {

			if (config_basic->ssid == 0)
				out_size = snprintf(out, out_ln, ":%-6s   :EQNS.0,1,0,0,1,0,0,0.07,-8,0,0.07,4,0,0.07,4", config_basic->callsign);
			else if (config_basic->ssid > 0 && config_basic->ssid < 10)
				out_size = snprintf(out, out_ln, ":%-9s:EQNS.0,1,0,0,1,0,0,0.07,-8,0,0.07,4,0,0.07,4", message_prefix_buffer);
			else if (config_basic->ssid >= 10 && config_basic->ssid < 16)
				out_size = snprintf(out, out_ln, ":%-9s:EQNS.0,1,0,0,1,0,0,0.07,-8,0,0.07,4,0,0.07,4", message_prefix_buffer);
			else
				return out_size;

			break;
		}

		case TELEMETRY_PV_UNIT: {

			if (config_basic->ssid == 0)
				out_size = snprintf(out, out_ln, ":%-6s   :UNIT.Pkt,Pkt,A,V,V,Hi,Hi,Hi,Hi,Hi,Hi,Hi", config_basic->callsign);
			else if (config_basic->ssid > 0 && config_basic->ssid < 10)
				out_size = snprintf(out, out_ln, ":%-9s:UNIT.Pkt,Pkt,A,V,V,Hi,Hi,Hi,Hi,Hi,Hi,Hi", message_prefix_buffer);
			else if (config_basic->ssid >= 10 && config_basic->ssid < 16)
				out_size = snprintf(out, out_ln, ":%-9s:UNIT.Pkt,Pkt,A,V,V,Hi,Hi,Hi,Hi,Hi,Hi,Hi", message_prefix_buffer);
			else
				return out_size;

			break;
		}

		case TELEMETRY_NORMAL_PARAM : {

			// prepare a frame with channel names depending on SSID
			if (config_basic->ssid == 0)
				out_size = snprintf(out, out_ln, ":%-6s   :PARM.Rx10min,Tx10min,Digi10min,Vbatt,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB,VBATT_LOW", config_basic->callsign);
			else if (config_basic->ssid > 0 && config_basic->ssid < 10)
				out_size = snprintf(out, out_ln, ":%-9s:PARM.Rx10min,Tx10min,Digi10min,Vbatt,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB,VBATT_LOW", message_prefix_buffer);
			else if (config_basic->ssid >= 10 && config_basic->ssid < 16)
				out_size = snprintf(out, out_ln, ":%-9s:PARM.Rx10min,Tx10min,Digi10min,Vbatt,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB,VBATT_LOW", message_prefix_buffer);
			else
				return out_size;

			break;
		}

		case TELEMETRY_VISCOUS_EQNS :
		case TELEMETRY_NORMAL_EQNS : {

			if (config_basic->ssid == 0)
				out_size = snprintf(out, out_ln, ":%-6s   :EQNS.0,1,0,0,1,0,0,1,0,0,0.02,10,0,0.5,-50", config_basic->callsign);
			else if (config_basic->ssid > 0 && config_basic->ssid < 10)
				out_size = snprintf(out, out_ln, ":%-9s:EQNS.0,1,0,0,1,0,0,1,0,0,0.02,10,0,0.5,-50", message_prefix_buffer);
			else if (config_basic->ssid >= 10 && config_basic->ssid < 16)
				out_size = snprintf(out, out_ln, ":%-9s:EQNS.0,1,0,0,1,0,0,1,0,0,0.02,10,0,0.5,-50", message_prefix_buffer);
			else
				return out_size;

			break;
		}

		case TELEMETRY_VISCOUS_UNIT :
		case TELEMETRY_NORMAL_UNIT : {

			if (config_basic->ssid == 0)
				out_size = snprintf(out, out_ln, ":%-6s   :UNIT.Pkt,Pkt,Pkt,V,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi,Hi", config_basic->callsign);
			else if (config_basic->ssid > 0 && config_basic->ssid < 10)
				out_size = snprintf(out, out_ln, ":%-9s:UNIT.Pkt,Pkt,Pkt,V,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi,Hi", message_prefix_buffer);
			else if (config_basic->ssid >= 10 && config_basic->ssid < 16)
				out_size = snprintf(out, out_ln, ":%-9s:UNIT.Pkt,Pkt,Pkt,V,DegC,Hi,Hi,Hi,Hi,Hi,Hi,Hi,Hi", message_prefix_buffer);
			else
				return out_size;

			break;
		}

		case TELEMETRY_VISCOUS_PARAM : {

			// prepare a frame with channel names depending on SSID
			if (config_basic->ssid == 0)
				out_size = snprintf(out, out_ln, ":%-6s   :PARM.Rx10min,Visc10min,Digi10min,Vbatt,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB,VBATT_LOW", config_basic->callsign);
			else if (config_basic->ssid > 0 && config_basic->ssid < 10)
				out_size = snprintf(out, out_ln, ":%-9s:PARM.Rx10min,Visc10min,Digi10min,Vbatt,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB,VBATT_LOW", message_prefix_buffer);
			else if (config_basic->ssid >= 10 && config_basic->ssid < 16)
				out_size = snprintf(out, out_ln, ":%-9s:PARM.Rx10min,Visc10min,Digi10min,Vbatt,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,QNH_QF_NAVBLE,HUM_QF_NAVBLE,WIND_QF_DEGR,WIND_QF_NAVB,VBATT_LOW", message_prefix_buffer);
			else
				return out_size;

			break;
		}
	}

	out[out_size] = 0;

	return out_size;

}

void telemetry_send_chns_description_pv(const config_data_basic_t * const config_basic) {

	// a buffer to assembly the 'call-ssid' string at the begining of the frame
	char message_prefix_buffer[9];

	memset(message_prefix_buffer, 0x00, 0x09);

	sprintf(message_prefix_buffer, "%s-%d", config_basic->callsign, config_basic->ssid);

	while (main_afsk.sending == 1);


	main_own_aprs_msg_len = telemetry_create_description_string(config_basic, TELEMETRY_PV_PARM, main_own_aprs_msg, OWN_APRS_MSG_LN);
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	while (main_afsk.sending == 1);
	delay_fixed(1200);


	main_own_aprs_msg_len = telemetry_create_description_string(config_basic, TELEMETRY_PV_EQNS, main_own_aprs_msg, OWN_APRS_MSG_LN);
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	while (main_afsk.sending == 1);
	delay_fixed(1200);

	main_own_aprs_msg_len = telemetry_create_description_string(config_basic, TELEMETRY_PV_UNIT, main_own_aprs_msg, OWN_APRS_MSG_LN);
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
								pressure_qf_t press_qf,
								humidity_qf_t humid_qf,
								wind_qf_t anemometer_qf)
{
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
		telemetry_qf = '1', telemetry_degr = '0', telemetry_nav = '0';
		break;
	case DALLAS_QF_DEGRADATED:
		telemetry_qf = '0', telemetry_degr = '1', telemetry_nav = '0';
		break;

	case DALLAS_QF_NOT_AVALIABLE:
		telemetry_qf = '0', telemetry_degr = '0', telemetry_nav = '1';
		break;

	default:
		telemetry_qf = '0', telemetry_degr = '0', telemetry_nav = '0';
		break;
	}

	// set the quality factor for pressure
	switch (press_qf) {
	case PRESSURE_QF_NOT_AVALIABLE:
	case PRESSURE_QF_UNKNOWN:
		telemetry_pressure_qf_navaliable = '1';
		 break;
	default:
		telemetry_pressure_qf_navaliable = '0';
		break;
	}

	switch (humid_qf) {
	case HUMIDITY_QF_UNKNOWN:
	case HUMIDITY_QF_NOT_AVALIABLE:
		telemetry_humidity_qf_navaliable = '1';
		break;
	default:
		telemetry_humidity_qf_navaliable = '0';
	}

	main_own_aprs_msg_len = sprintf(
				main_own_aprs_msg,
				"T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0",
				telemetry_counter++,
				rx_pkts,
				digi_pkts,
				scaled_battery_current,
				scaled_battery_voltage,
				scaled_pvcell_volage,
				telemetry_qf,
				telemetry_degr,
				telemetry_nav,
				telemetry_pressure_qf_navaliable,
				telemetry_humidity_qf_navaliable,
				telemetry_anemometer_degradated,
				telemetry_anemometer_navble);


	if (telemetry_counter > 999)
		telemetry_counter = 0;
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	WAIT_FOR_CHANNEL_FREE();

	afsk_txStart(&main_afsk);
#ifdef PARAMETEO
	rte_main_trigger_gsm_telemetry_values = 1;
#endif
}

void telemetry_send_status_pv(ve_direct_average_struct* avg, ve_direct_error_reason* last_error, ve_direct_system_state state, uint32_t master_time, uint16_t messages_count, uint16_t corrupted_messages_count) {
	char string_buff_err[24], string_buff_state[23];

	ve_direct_state_to_string(state, string_buff_state, 23);
	ve_direct_error_to_string(*last_error, string_buff_err, 24);

	main_own_aprs_msg_len = snprintf(main_own_aprs_msg, sizeof(main_own_aprs_msg), ">MT %lX, MC %lX, CMC %lX, IMIN %d, IMAX %d, %s, %s", master_time, (uint32_t)messages_count, (uint32_t)corrupted_messages_count, avg->min_battery_current, avg->max_battery_current, string_buff_state, string_buff_err);
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
void telemetry_send_chns_description(const config_data_basic_t * const config_basic, const config_data_mode_t * const config_mode) {

	uint8_t is_viscous = 1;

	if (variant_validate_is_within_ram(config_basic) == 0 &&
		variant_validate_is_within_flash(config_basic) == 0) {
		return;
	}

	if (variant_validate_is_within_ram(config_basic) == 0 &&
		variant_validate_is_within_flash(config_basic) == 0) {
		is_viscous = 0;
	}
	else if (config_mode->digi_viscous == 0) {
		is_viscous = 0;
	}
	else {
		;
	}

	// a buffer to assembly the 'call-ssid' string at the begining of the frame
	char message_prefix_buffer[9];

	memset(message_prefix_buffer, 0x00, 0x09);

	sprintf(message_prefix_buffer, "%s-%d", config_basic->callsign, config_basic->ssid);

	// wait for any RF transmission to finish
	main_wait_for_tx_complete();

	// clear the output frame buffer
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

	if (is_viscous == 0) {
		main_own_aprs_msg_len = telemetry_create_description_string(config_basic, TELEMETRY_NORMAL_PARAM, main_own_aprs_msg, OWN_APRS_MSG_LN);
		ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
		after_tx_lock = 1;
	}
	else {
		main_own_aprs_msg_len = telemetry_create_description_string(config_basic, TELEMETRY_VISCOUS_PARAM, main_own_aprs_msg, OWN_APRS_MSG_LN);
		ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
		after_tx_lock = 1;
	}

	// key up the transmitter and
	afsk_txStart(&main_afsk);
	main_wait_for_tx_complete();
	delay_fixed(1500);
	WAIT_FOR_CHANNEL_FREE();

	main_own_aprs_msg_len = telemetry_create_description_string(config_basic, TELEMETRY_NORMAL_EQNS, main_own_aprs_msg, OWN_APRS_MSG_LN);
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;

	afsk_txStart(&main_afsk);
	main_wait_for_tx_complete();
	delay_fixed(1500);
	WAIT_FOR_CHANNEL_FREE();

	main_own_aprs_msg_len = telemetry_create_description_string(config_basic, TELEMETRY_NORMAL_UNIT, main_own_aprs_msg, OWN_APRS_MSG_LN);
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

}

/**
 * This function sends telemetry values in 'typical configuration' when VICTRON VE.direct protocol parser is not enabled.
 */
#ifdef PARAMETEO
void telemetry_send_values(	uint8_t rx_pkts,
							uint8_t tx_pkts,
							uint8_t digi_pkts,
							uint16_t vbatt_voltage,
							uint8_t viscous_drop_pkts,
							float temperature,
							dallas_qf_t dallas_qf,
							pressure_qf_t press_qf,
							humidity_qf_t humid_qf,
							wind_qf_t anemometer_qf,
							int8_t cutoff_and_vbat_low,
							const config_data_mode_t * const config_mode) {

	// this is B+ voltage, which is scaled * 100 what means that 1152 equals to 11.52V
	if (vbatt_voltage < 1511 && vbatt_voltage > 1000) {
		// mininum value will be 10.01V (0x0) and maximum 15.11V (0xFF), with the step of .02V
		telemetry_scaled_vbatt_voltage = (uint8_t)((vbatt_voltage - 1000u) / 2u);
	}
	else if (vbatt_voltage > 1510) {
		telemetry_scaled_vbatt_voltage = 0xFF;
	}
	else {
		;
	}


#else
	void telemetry_send_values(	uint8_t rx_pkts,
								uint8_t tx_pkts,
								uint8_t digi_pkts,
								uint8_t kiss_pkts,
								uint8_t viscous_drop_pkts,
								float temperature,
								dallas_qf_t dallas_qf,
								pressure_qf_t press_qf,
								humidity_qf_t humid_qf,
								wind_qf_t anemometer_qf,
								const config_data_mode_t * const config_mode) {
#endif

	// get the quality factor for wind measurements
	if (anemometer_qf == WIND_QF_DEGRADATED) {
		telemetry_anemometer_degradated = '1';
		telemetry_anemometer_navble = '0';
	}
	else if (anemometer_qf == WIND_QF_NOT_AVALIABLE) {
		telemetry_anemometer_degradated = '0';
		telemetry_anemometer_navble = '1';
	}
	else if (anemometer_qf == WIND_QF_UNKNOWN) {
		telemetry_anemometer_degradated = '1';
		telemetry_anemometer_navble = '1';
	}

	// scale the physical temperature and limit upper and lower boundary if
	// it is required
	if (temperature < -50.0f) {
		telemetry_scaled_temperature = (uint8_t)0;
	}
	else if (temperature > 70.0f) {
		telemetry_scaled_temperature = (uint8_t)255;
	}
	else {
		telemetry_scaled_temperature = (uint8_t)((temperature + 50.0f) * 2.0f);
	}

	// set the quality factor for dallas DS18B20
	switch (dallas_qf) {
	case DALLAS_QF_FULL:
		telemetry_qf = '1', telemetry_degr = '0', telemetry_nav = '0';
		break;
	case DALLAS_QF_DEGRADATED:
		telemetry_qf = '0', telemetry_degr = '1', telemetry_nav = '0';
		break;

	case DALLAS_QF_NOT_AVALIABLE:
		telemetry_qf = '0', telemetry_degr = '0', telemetry_nav = '1';
		break;

	default:
		telemetry_qf = '0', telemetry_degr = '0', telemetry_nav = '0';
		break;
	}

	// set the quality factor for pressure
	switch (press_qf) {
	case PRESSURE_QF_NOT_AVALIABLE:
	case PRESSURE_QF_UNKNOWN:
		telemetry_pressure_qf_navaliable = '1';
		 break;
	default:
		telemetry_pressure_qf_navaliable = '0';
		break;
	}

	switch (humid_qf) {
	case HUMIDITY_QF_UNKNOWN:
	case HUMIDITY_QF_NOT_AVALIABLE:
		telemetry_humidity_qf_navaliable = '1';
		break;
	default:
		telemetry_humidity_qf_navaliable = '0';
	}

#ifdef PARAMETEO
	// telemetry won't be sent during cutoff anyway so this simplification is correct here
	if (cutoff_and_vbat_low > 0) {
		telemetry_vbatt_low = '1';
	}
#endif

	// reset the buffer where the frame will be contructed and stored for transmission
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

#ifdef PARAMETEO
	if (config_mode->digi_viscous == 0) {
			// generate the telemetry frame from values
			main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c%c", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, telemetry_scaled_vbatt_voltage, telemetry_scaled_temperature, telemetry_qf, telemetry_degr, telemetry_nav, telemetry_pressure_qf_navaliable, telemetry_humidity_qf_navaliable, telemetry_anemometer_degradated, telemetry_anemometer_navble, telemetry_vbatt_low);
	}
	else {
			main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c%c", telemetry_counter++, rx_pkts, viscous_drop_pkts, digi_pkts, telemetry_scaled_vbatt_voltage, telemetry_scaled_temperature, telemetry_qf, telemetry_degr, telemetry_nav, telemetry_pressure_qf_navaliable, telemetry_humidity_qf_navaliable, telemetry_anemometer_degradated, telemetry_anemometer_navble, telemetry_vbatt_low);
	}
#else
	if (config_mode->digi_viscous == 0) {
			// generate the telemetry frame from values
		#ifdef _DALLAS_AS_TELEM
			main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, kiss_pkts, scaled_temperature, telemetry_qf, telemetry_degr, telemetry_nav, telemetry_pressure_qf_navaliable, telemetry_humidity_qf_navaliable, telemetry_anemometer_degradated, telemetry_anemometer_navble);
		#else
			main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, kiss_pkts, scaled_temperature, telemetry_qf, telemetry_degr, telemetry_nav, telemetry_pressure_qf_navaliable, telemetry_humidity_qf_navaliable, telemetry_anemometer_degradated, telemetry_anemometer_navble);
		#endif
	}
	else {
		#ifdef _DALLAS_AS_TELEM
			main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, viscous_drop_pkts, scaled_temperature, telemetry_qf, telemetry_degr, telemetry_nav, telemetry_pressure_qf_navaliable, telemetry_humidity_qf_navaliable, telemetry_anemometer_degradated, telemetry_anemometer_navble);
		#else
			main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c0", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, viscous_drop_pkts, scaled_temperature, telemetry_qf, telemetry_degr, telemetry_nav, telemetry_pressure_qf_navaliable, telemetry_humidity_qf_navaliable, telemetry_anemometer_degradated, telemetry_anemometer_navble);
		#endif
	}
#endif

	// reset the frame counter if it overflowed
	if (telemetry_counter > 999) {
		telemetry_counter = 0;
	}

	// store telemetry conter in backup register
	backup_reg_set_telemetry((uint8_t)(telemetry_counter & 0xFFu));

	// put a null terminator at the end of frame (but it should be placed there anyway)
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;

	// wait for completing any previous transmission (afsk_txStart will exit with failure if the modem is transmitting)
	main_wait_for_tx_complete();

	// prepare transmission of the frame
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);

 	// ??
	after_tx_lock = 1;

	// check if RF channel is free from other transmissions and wait for the clearance if it is needed
	WAIT_FOR_CHANNEL_FREE();

	// key up a transmitter and start transmission
	afsk_txStart(&main_afsk);

#ifdef PARAMETEO
	// trigger packet to aprs-is server
	rte_main_trigger_gsm_telemetry_values = 1;
#endif

}

uint16_t telemetry_get_counter(void) {
	return telemetry_counter;
}
