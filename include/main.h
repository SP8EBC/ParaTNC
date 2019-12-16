#ifndef MAIN_H_
#define MAIN_H_

#include "aprs/ax25.h"

#define SW_VER "DE04"
#define SW_DATE "15122019"

#define SYSTICK_TICKS_PER_SECONDS 100
#define SYSTICK_TICKS_PERIOD 10

extern uint32_t master_time;

extern int32_t main_wx_sensors_pool_timer;
extern int32_t main_packet_tx_pool_timer;
extern int32_t main_two_second_pool_timer;

extern AX25Ctx main_ax25;
extern Afsk main_afsk;

extern AX25Call main_own_path[3];
extern uint8_t main_own_path_ln;
extern uint8_t main_own_aprs_msg_len;
extern char main_own_aprs_msg[160];

extern char after_tx_lock;

extern unsigned short rx10m, tx10m, digi10m, kiss10m;

uint16_t main_get_adc_sample(void);
void main_wx_decremenet_counter(void);
void main_packets_tx_decremenet_counter(void);
void main_two_second_pool_decrement_counter(void);

inline void main_wait_for_tx_complete(void) {
	while(main_afsk.sending == 1);
}


#endif
