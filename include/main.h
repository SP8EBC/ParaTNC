#ifndef MAIN_H_
#define MAIN_H_

#include "aprs/ax25.h"
#include "drivers/serial.h"

#define SW_VER "DF09"
#define SW_DATE "03112020"

#define SYSTICK_TICKS_PER_SECONDS 100
#define SYSTICK_TICKS_PERIOD 10

#define INTERNAL_WATCHDOG
#define EXTERNAL_WATCHDOG

#define OWN_APRS_MSG_LN 	160

extern uint32_t master_time;

extern int32_t main_wx_sensors_pool_timer;
extern int32_t main_one_minute_pool_timer;
extern int32_t main_one_second_pool_timer;
extern int32_t main_two_second_pool_timer;
extern int32_t main_ten_second_pool_timer;

extern AX25Ctx main_ax25;
extern Afsk main_afsk;

extern AX25Call main_own_path[3];
extern uint8_t main_own_path_ln;
extern uint8_t main_own_aprs_msg_len;
extern char main_own_aprs_msg[OWN_APRS_MSG_LN];

extern srl_context_t* main_kiss_srl_ctx_ptr;
extern srl_context_t* main_wx_srl_ctx_ptr;

extern char after_tx_lock;

extern unsigned short rx10m, tx10m, digi10m, digidrop10m, kiss10m;

uint16_t main_get_adc_sample(void);

void main_service_cpu_load_ticks(void);

inline uint32_t main_get_master_time(void) {
	return master_time;
}

inline void main_wait_for_tx_complete(void) {
	while(main_afsk.sending == 1);
}

inline void main_wx_decremenet_counter(void) {
	if (main_wx_sensors_pool_timer > 0)
		main_wx_sensors_pool_timer -= SYSTICK_TICKS_PERIOD;
}

inline void main_packets_tx_decremenet_counter(void) {
	if (main_one_minute_pool_timer > 0)
		main_one_minute_pool_timer -= SYSTICK_TICKS_PERIOD;
}

inline void main_one_second_pool_decremenet_counter(void) {
	if (main_one_second_pool_timer > 0)
		main_one_second_pool_timer -= SYSTICK_TICKS_PERIOD;
}

inline void main_two_second_pool_decrement_counter(void) {
		main_two_second_pool_timer -= SYSTICK_TICKS_PERIOD;
}

inline void main_ten_second_pool_decremenet_counter(void) {
	if (main_ten_second_pool_timer > 0)
		main_ten_second_pool_timer -= SYSTICK_TICKS_PERIOD;
}


#endif
