/*
 * packet_tx_handler.h
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#ifndef PACKET_TX_HANDLER_H_
#define PACKET_TX_HANDLER_H_

#include <stored_configuration_nvm/config_data.h>

extern uint8_t packet_tx_beacon_interval;
extern uint8_t packet_tx_beacon_counter;

extern uint8_t packet_tx_error_status_interval;
extern uint8_t packet_tx_error_status_counter;

extern uint8_t packet_tx_meteo_interval;
extern uint8_t packet_tx_meteo_counter;

extern uint8_t packet_tx_meteo_kiss_interval;
extern uint8_t packet_tx_meteo_kiss_counter;

extern uint8_t packet_tx_telemetry_interval;
extern uint8_t packet_tx_telemetry_counter;

extern uint8_t packet_tx_telemetry_descr_interval;	// 155
extern uint8_t packet_tx_telemetry_descr_counter;

extern uint8_t packet_tx_modbus_raw_values;
extern uint8_t packet_tx_modbus_status;

typedef struct packet_tx_counter_values_t {

	uint8_t beacon_counter;
	uint8_t wx_counter;
	uint8_t gsm_wx_counter;
	uint8_t telemetry_counter;
	uint8_t telemetry_desc_counter;
	uint8_t kiss_counter;

} packet_tx_counter_values_t;

void packet_tx_send_wx_frame(void);
void packet_tx_init(uint8_t meteo_interval, uint8_t beacon_interval, config_data_powersave_mode_t powersave);
void packet_tx_restore_from_backupregs(void);
void packet_tx_tcp_handler(void);
void packet_tx_handler(const config_data_basic_t * const config_basic, const config_data_mode_t * const config_mode);
void packet_tx_get_current_counters(packet_tx_counter_values_t * out);
void packet_tx_set_current_counters(packet_tx_counter_values_t * in);
int16_t packet_tx_get_minutes_to_next_wx(void);
uint8_t packet_tx_is_gsm_meteo_pending(void);
void packet_tx_force_gsm_status(void);

#endif /* PACKET_TX_HANDLER_H_ */
