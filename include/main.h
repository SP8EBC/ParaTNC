#ifndef MAIN_H_
#define MAIN_H_

#include "aprs/ax25.h"

extern uint32_t master_time;

extern uint32_t main_wx_sensors_pool_timer;

extern AX25Ctx main_ax25;
extern Afsk main_afsk;

extern AX25Call path[3];
extern uint8_t path_len;
extern uint8_t aprs_msg_len;
extern char main_own_aprs_msg[160];

extern char after_tx_lock;

extern unsigned short rx10m, tx10m, digi10m, kiss10m;

uint16_t main_get_adc_sample(void);
void main_wx_decremenet_counter(void);
void main_packets_tx_decremenet_counter(void);

inline void main_wait_for_tx_complete(void) {
	while(main_afsk.sending == 1);
}


#endif
