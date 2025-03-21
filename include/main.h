#ifndef MAIN_H_
#define MAIN_H_

#include <stored_configuration_nvm/config_data.h>
#include "main_master_time.h"

#include "station_config_target_hw.h"

#include "aprs/ax25.h"
#include "drivers/serial.h"
#include "gsm/sim800_state_t.h"

#include "software_version.h"

#define SYSTICK_TICKS_PER_SECONDS 100
#define SYSTICK_TICKS_PERIOD 10

#define INTERNAL_WATCHDOG
//#define EXTERNAL_WATCHDOG

#define PWR_SWITCH_BOTH

#define OWN_APRS_MSG_LN 	255u

#define MAIN_GET_RTC_YEAR	1u
#define MAIN_GET_RTC_MONTH	2u
#define MAIN_GET_RTC_DAY	3u
#define MAIN_GET_RTC_HOUR	4u
#define MAIN_GET_RTC_MIN	5u
#define MAIN_GET_RTC_SEC	6u

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


extern uint32_t main_flash_log_start;
extern uint32_t main_flash_log_end;


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
extern int16_t main_one_second_pool_timer;
extern int16_t main_two_second_pool_timer;
extern int16_t main_four_second_pool_timer;
extern int16_t main_ten_second_pool_timer;

extern AX25Ctx main_ax25;
extern Afsk main_afsk;

extern AX25Call main_own_path[3];
extern uint8_t main_own_path_ln;
extern uint8_t main_own_aprs_msg_len;
extern char main_own_aprs_msg[OWN_APRS_MSG_LN];

extern char main_string_latitude[9];
extern char main_string_longitude[9];
extern char main_callsign_with_ssid[10];

extern char main_symbol_f;
extern char main_symbol_s;

extern srl_context_t* main_kiss_srl_ctx_ptr;
extern srl_context_t* main_wx_srl_ctx_ptr;
extern srl_context_t* main_gsm_srl_ctx_ptr;

extern  uint8_t main_kiss_enabled;

extern uint8_t main_woken_up;

extern int8_t main_cpu_load;

extern const float main_test_float;

extern const char  main_test_string[11];

extern char after_tx_lock;

extern unsigned short rx10m, tx10m, digi10m, digidrop10m, kiss10m;

extern gsm_sim800_state_t main_gsm_state;

uint16_t main_get_adc_sample(void);

void main_service_cpu_load_ticks(void);

void main_reload_internal_wdg(void);

uint16_t main_get_rtc_datetime(uint16_t param);

uint32_t main_get_nvm_timestamp(void);

#if defined(STM32L471xx)
extern uint32_t rte_main_rx_total;
extern uint32_t rte_main_tx_total;
#endif

/**
 * Block I/O function which waits for all transmission to end
 */
inline void main_wait_for_tx_complete(void) {
	while(main_afsk.sending == 1);
}

/**
 * Reset pooling timers values after waking up from deep sleep, to be sure
 * than
 */
inline void main_reset_pooling_timers(void) {
	main_wx_sensors_pool_timer = 35000;

	// global variable used as a timer to trigger packet sending
	main_one_minute_pool_timer = 60000;

	// one second pool interval
	main_one_second_pool_timer = 1000;

	// two second pool interval
	main_two_second_pool_timer = 2000;

	main_four_second_pool_timer = 4000;

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

inline void main_four_second_pool_decrement_counter(void) {
		main_four_second_pool_timer -= SYSTICK_TICKS_PERIOD;
}

inline void main_ten_second_pool_decremenet_counter(void) {
	if (main_ten_second_pool_timer > 0)
		main_ten_second_pool_timer -= SYSTICK_TICKS_PERIOD;
}


#endif
