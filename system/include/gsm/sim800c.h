/*
 * sim800c.h
 *
 *  Created on: Jan 18, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800C_H_
#define INCLUDE_GSM_SIM800C_H_

#include "drivers/serial.h"
#include "gsm/sim800_state_t.h"
#include "gsm/sim800_async_message_t.h"

extern const char * gsm_at_command_sent_last;

extern char gsm_sim800_simcard_status_string[10];
extern char gsm_sim800_registered_network[16];
extern int8_t gsm_sim800_signal_level_dbm;
extern float gsm_sim800_bcch_frequency;
extern char gsm_sim800_cellid[5];
extern char gsm_sim800_lac[5];

uint32_t gsm_sim800_check_for_extra_newlines(uint8_t * ptr, uint16_t size);
sim800_async_message_t gsm_sim800_check_for_async_messages(uint8_t * ptr, uint16_t size, uint16_t * offset);

void gsm_sim800_init(gsm_sim800_state_t * state, uint8_t enable_echo);
void gsm_sim800_initialization_pool(srl_context_t * srl_context, gsm_sim800_state_t * state);
uint8_t gsm_sim800_rx_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter);	// callback used to detect echo
void gsm_sim800_rx_done_event_handler(srl_context_t * srl_context, gsm_sim800_state_t * state);
void gsm_sim800_tx_done_event_handler(srl_context_t * srl_context, gsm_sim800_state_t * state);
void gsm_sim800_reset(gsm_sim800_state_t * state);

uint8_t gsm_sim800_get_waiting_for_command_response(void);
uint16_t gsm_sim800_get_response_start_idx(void);

void gsm_sim800_create_status(char * buffer, int ln);

void gsm_sim800_decrease_counter(void);
void gsm_sim800_inhibit(uint8_t _inhibit);

#endif /* INCLUDE_GSM_SIM800C_H_ */
