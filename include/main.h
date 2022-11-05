#ifndef MAIN_H_
#define MAIN_H_

#include "station_config_target_hw.h"

#include "aprs/ax25.h"
#include "drivers/serial.h"
#include "config_data.h"

#define SW_VER "ZT17"
#define SW_DATE "06112022"
#define SW_KISS_PROTO	"A"

#define SYSTICK_TICKS_PER_SECONDS 100
#define SYSTICK_TICKS_PERIOD 10

//#define INTERNAL_WATCHDOG
#define EXTERNAL_WATCHDOG

#define PWR_SWITCH_BOTH

#define OWN_APRS_MSG_LN 	256

// backup registers (ParaMETEO)
// 0 -> powersave status
// 1 -> last sleep rtc time
// 2 -> last wakeup rtc time
// 3 -> controller configuration status
// 4 -> wakeup events MSB, sleep events LSB

#ifdef STM32L471xx
#define REGISTER 			RTC->BKP0R
#define REGISTER_LAST_SLEEP	RTC->BKP1R
#define REGISTER_LAST_WKUP	RTC->BKP2R
#define REGISTER_COUNTERS	RTC->BKP4R
#define REGISTER_MONITOR	RTC->BKP5R
#define REGISTER_LAST_SLTIM	RTC->BKP6R
#endif

typedef enum main_usart_mode_t {
	USART_MODE_UNDEF,
	USART_MODE_KISS,
	USART_MODE_VICTRON,
	USART_MODE_DUST_SDS,
	USART_MODE_DAVIS,
	USART_MODE_MODBUS,
	USART_MODE_UMB_MASTER,
	USART_MODE_UNINIT
}main_usart_mode_t;

extern uint32_t master_time;

extern const config_data_mode_t * main_config_data_mode;
extern const config_data_basic_t * main_config_data_basic;
extern const config_data_wx_sources_t * main_config_data_wx_sources;
extern const config_data_umb_t * main_config_data_umb;
extern const config_data_rtu_t * main_config_data_rtu;
#ifdef PARAMETEO
extern const config_data_gsm_t * main_config_data_gsm;
#endif

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

extern char main_string_latitude[9];
extern char main_string_longitude[9];

extern char main_symbol_f;
extern char main_symbol_s;

extern srl_context_t* main_kiss_srl_ctx_ptr;
extern srl_context_t* main_wx_srl_ctx_ptr;
extern srl_context_t* main_gsm_srl_ctx_ptr;

extern  uint8_t main_kiss_enabled;

extern uint8_t main_woken_up;

extern int8_t main_cpu_load;

extern char after_tx_lock;

extern unsigned short rx10m, tx10m, digi10m, digidrop10m, kiss10m;

//void main_set_monitor(int8_t bit);

uint16_t main_get_adc_sample(void);

void main_service_cpu_load_ticks(void);

void main_reload_internal_wdg(void);

#if defined(STM32L471xx)
extern uint32_t rte_main_rx_total;
extern uint32_t rte_main_tx_total;
#endif

/**
 * Inline used to trace an execution flow across main for(;;) loop and some
 * powersaving functions. In case of software fault it's value may help to trace
 * at witch point the crash has occured
 */
inline void main_set_monitor(int8_t bit) {
#ifdef STM32L471xx
	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;

	if (bit >= 0) {
		REGISTER_MONITOR |= (1 << bit);

	}
	else {
		REGISTER_MONITOR = 0;
	}

	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_DBP);
#endif
}

inline uint32_t main_get_master_time(void) {
	return master_time;
}

inline void main_wait_for_tx_complete(void) {
	while(main_afsk.sending == 1);
}

inline void main_reset_pooling_timers(void) {
	main_wx_sensors_pool_timer = 35000;

	// global variable used as a timer to trigger packet sending
	main_one_minute_pool_timer = 60000;

	// one second pool interval
	main_one_second_pool_timer = 1000;

	// two second pool interval
	main_two_second_pool_timer = 2000;

	// ten second pool interval
	main_ten_second_pool_timer = 10000;
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
