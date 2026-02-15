#include "main.h"
#include "main_freertos_externs.h"
#include "main_gsm_pool_handler.h"

#include "cmsis/stm32l4xx/system_stm32l4xx.h"
#include <stm32l4xx.h>
#include <stm32l4xx_hal_cortex.h>
#include <stm32l4xx_hal_rtc.h>
#include <stm32l4xx_ll_gpio.h>
#include <stm32l4xx_ll_iwdg.h>
#include <stm32l4xx_ll_rcc.h>

#include "gsm/sim800c.h"
#include "gsm/sim800c_engineering.h"
#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_poolers.h"
#include "http_client/http_client.h"

#include "./nvm/nvm.h"
#include "./nvm/nvm_event.h"

#include "./event_log.h"
#include "./events_definitions/events_main.h"

#include "api/api.h"
#include "aprsis.h"
#include "drivers/l4/pwm_input_stm32l4x.h"
#include "drivers/l4/spi_speed_stm32l4x.h"
#include "drivers/max31865.h"

#include "gsm_comm_state_handler.h"
#include "ntp.h"

#include "./etc/pwr_save_configuration.h"

#include <LedConfig.h>
#include <delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packet_tx_handler.h"

#include "station_config.h"
#include <stored_configuration_nvm/config_data_externs.h>
#include <stored_configuration_nvm/configuration_handler.h>

#include "LedConfig.h"
#include "PathConfig.h"
#include "TimerConfig.h"
#include "afsk_pr.h"
#include "antilib_adc.h"
#include "backup_registers.h"
#include "button.h"
#include "diag/Trace.h"
#include "event_log_postmortem.h"
#include "float_to_string.h"
#include "io.h"
#include "io_default_vbat_scaling.h"
#include "pwr_save.h"
#include "supervisor.h"
#include <wx_pwr_switch.h>

#include "it_handlers.h"

#include "aprs/beacon.h"
#include "aprs/dac.h"
#include "aprs/digi.h"
#include "aprs/status.h"
#include "aprs/telemetry.h"

#include "ve_direct_protocol/parser.h"

#include "rte_main.h"
#include "rte_pv.h"
#include "rte_rtu.h"
#include "rte_wx.h"

#include "aprs/wx.h"
#include "drivers/analog_anemometer.h"
#include "drivers/dallas.h"
#include "drivers/i2c.h"
#include "drivers/spi.h"
#include "dust_sensor/sds011.h"
#include <wx_handler.h>

#include "../system/include/modbus_rtu/rtu_serial_io.h"

#include "../system/include/davis_vantage/davis.h"
#include "../system/include/davis_vantage/davis_parsers.h"

#include "drivers/ms5611.h"
#include <drivers/bme280.h>

#include "umb_master/umb_0x26_status.h"
#include "umb_master/umb_channel_pool.h"
#include "umb_master/umb_master.h"

#include "drivers/dallas.h"

#include "kiss_communication/diagnostics_services/kiss_security_access.h"
#include <etc/kiss_configuation.h>
#include <kiss_communication/kiss_communication.h>
#include <kiss_communication/kiss_communication_aprsmsg.h>

#include <etc/dallas_temperature_limits.h>

#include <etc/misc_config.h>

#include "memory_map.h"

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <event_groups.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

/* FreeRTOS tasks*/
#include "tasks/task_event_api_ntp.h"
#include "tasks/task_event_apris_msg_triggers.h"
#include "tasks/task_event_gsm_rx_done.h"
#include "tasks/task_event_gsm_tx_done.h"
#include "tasks/task_event_kiss_rx_done.h"
#include "tasks/task_event_kiss_tx_done.h"
#include "tasks/task_event_radio_message.h"
#include "tasks/task_event_serial_sensor.h"
#include "tasks/task_fanet.h"
#include "tasks/task_main.h"
#include "tasks/task_one_minute.h"
#include "tasks/task_one_second.h"
#include "tasks/task_power_save.h"
#include "tasks/task_ten_second.h"
#include "tasks/task_two_second.h"

#include "etc/tasks_list.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SOH 0x01
#define MANUAL_RTC_SET

#ifdef SX1262_IMPLEMENTATION
#include "fanet_app.h"
#endif

// #include "variant.h"

// #define SERIAL_TX_TEST_MODE

// Niebieska dioda -> DCD
// Zielona dioda -> anemometr albo TX

// backup registers (ParaTNC)
// 0 ->
// 2 -> boot and hard fault count
// 3 -> controller configuration status
// 4 ->
// 5 ->
// 6 -> weather and telemetry timers & counters

// backup registers (ParaMETEO)
// 0 -> powersave status
// 1 -> last sleep rtc time
// 2 -> last wakeup rtc time
// 3 -> controller configuration status
// 4 -> wakeup events MSB, sleep events LSB
// 5 -> // not used, previously used by monitor
// 6 -> last sleep time
// 7 -> weather and telemetry timers & counters

#define CONFIG_FIRST_RESTORED		(1)
#define CONFIG_FIRST_FAIL_RESTORING (1 << 1)
#define CONFIG_FIRST_CRC_OK			(1 << 2)

#define CONFIG_SECOND_RESTORED		 (1 << 3)
#define CONFIG_SECOND_FAIL_RESTORING (1 << 4)
#define CONFIG_SECOND_CRC_OK		 (1 << 5)

// clang-format off
/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

/**
 * An entry creating single task
 */
#define MAIN_CREATE_TASK(task_entry_point, task_name_string, stack_size, params, priority, output_handle) 	\
														\
	const BaseType_t task_entry_point##_create_result = \
					xTaskCreate( 	task_entry_point, 	\
									task_name_string, 	\
									(stack_size), 		\
									( void * ) (params),\
									priority, 			\
									&(output_handle) );	\
	if (task_entry_point##_create_result != pdPASS)			\
	{														\
		goto task_creation_failed;							\
	}														\


/**
 * Expands full list of tasks and starts FreeRTOS scheduler
 */
#define MAIN_EXPAND_TASKS_LIST											\
								TASKS_LIST(MAIN_CREATE_TASK)			\
																		\
								main_scheduler_prestart_callback();		\
								/* Start the scheduler. */				\
								vTaskStartScheduler();					\
	task_creation_failed:												\


#define MAIN_CREATE_TASKS_ENUM(task_entry_point, task_name_string, stack_size, params, priority, output_handle)	\
		T_##output_handle,

// clang-format on

/**
 * A foreword about '#define' mess. This software is indented to run on at least two
 * different hardware platforms. First which is ParaTNC basing on STM32F100 and second
 * ParaMETEO using STM32L476. In future more platforms may appear. Like ParaTNC2 which
 * will be a ParaMETEO without battery charging and in form factor similar to ParaTNC.
 *
 * To obtain such compatibility a lot of #defines and different makefiles has to be used.
 * Some parts of the code are 'included' per target CPU basis, as are independent from
 * target platform directly. Including system headers (CMSIS, std peripheral driver),
 * configuring low level hardware like interrupt controler, clock etc.
 *
 * Some parts of code and header files are related to certain platform
 *
 */

// ----- main() ---------------------------------------------------------------

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

// used configuration structures
const config_data_mode_t *main_config_data_mode = 0;
const config_data_basic_t *main_config_data_basic = 0;
const config_data_wx_sources_t *main_config_data_wx_sources = 0;
const config_data_umb_t *main_config_data_umb = 0;
const config_data_rtu_t *main_config_data_rtu = 0;
const config_data_gsm_t *main_config_data_gsm = 0;

uint32_t main_flash_log_start = MEMORY_MAP_EVENT_LOG_START;
uint32_t main_flash_log_end = MEMORY_MAP_EVENT_LOG_END;

//! global variable incremented by the SysTick handler to measure time in miliseconds
volatile uint32_t master_time = 0;

//! current timestamp from RTC in NVM format
uint32_t main_nvm_timestamp = 0;

//! this global variable stores numbers of ticks of idling CPU
uint32_t main_idle_cpu_ticks = 0;

//! current cpu idle ticks
uint32_t main_current_cpu_idle_ticks = 0;

//! approx cpu load in percents
int8_t main_cpu_load = 0;

//! global variable used as a timer to trigger meteo sensors mesurements
int32_t main_wx_sensors_pool_timer = 65500;

//! global variable used as a timer to trigger packet sending
int32_t main_one_minute_pool_timer = 45000;

//! one second pool interval
int16_t main_one_second_pool_timer = 1000;

//! two second pool interval
int16_t main_two_second_pool_timer = 2000;

//! four second pool interval
int16_t main_four_second_pool_timer = 4000;

//! ten second pool interval
int16_t main_ten_second_pool_timer = 10000;

//! a pointer to KISS context
srl_context_t *main_kiss_srl_ctx_ptr;

//! a pointer to wx comms context
srl_context_t *main_wx_srl_ctx_ptr;

//! a pointer to gsm context
srl_context_t *main_gsm_srl_ctx_ptr;

//! target USART1 (kiss) baudrate
uint32_t main_target_kiss_baudrate;

//! target USART2 (wx) baudrate
uint32_t main_target_wx_baudrate;

//! controls if the KISS modem is enabled
uint8_t main_kiss_enabled = 1;

uint8_t main_reset_config_to_default = 0;

//! global variables represending the AX25/APRS stack
AX25Ctx main_ax25;
Afsk main_afsk;

volatile NVIC_Type *main_nvic = (volatile NVIC_Type *)NVIC_BASE;

AX25Call main_own_path[3];
uint8_t main_own_path_ln = 0;
uint8_t main_own_aprs_msg_len;
char main_own_aprs_msg[OWN_APRS_MSG_LN];

char main_string_latitude[9];
char main_string_longitude[9];
char main_callsign_with_ssid[10];

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

//! serial context for UART used to KISS
static srl_context_t main_kiss_srl_ctx;

//! serial context for UART used for comm with wx sensors
static srl_context_t main_wx_srl_ctx;

//! serial context for communication with GSM module
static srl_context_t main_gsm_srl_ctx;

//! operation mode of USART1 (RS232 on RJ45 socket)
static main_usart_mode_t main_usart1_kiss_mode = USART_MODE_UNDEF;

//! operation mode of USART2 (RS485)
static main_usart_mode_t main_usart2_wx_mode = USART_MODE_UNDEF;

//! function configuration for left button on ParaMETEO
static configuration_button_function_t main_button_one_left;

//! function configuration for right button on ParaMETEO
static configuration_button_function_t main_button_two_right;

//! controls if DAVIS serialprotocol client is enabled by the configuration
static uint8_t main_davis_serial_enabled = 0;

static uint8_t main_modbus_rtu_master_enabled = 0;

/********************************************************************/
/*************************FREE RTOS related**************************/

typedef enum main_tasks_enum_t {
	TASKS_LIST (MAIN_CREATE_TASKS_ENUM) TASKS_LIST_COUNT
} main_tasks_enum_t;

uint8_t main_rtos_is_runing = 0;

//!
static SemaphoreHandle_t main_mutex_gsm_tcpip;

//! data associated with the event group for powersave task sync
static StaticEventGroup_t main_eventgroup_powersave;

//! data associated with the event group for KISS host pc serial port
static StaticEventGroup_t main_eventgroup_serial_kiss;

//! data associated with the event group for UART communication with GSM module
static StaticEventGroup_t main_eventgroup_serial_gsm;

static StaticEventGroup_t main_eventgroup_serial_sensor;

//! data associated with the event group for KISS host pc serial port
static StaticEventGroup_t main_eventgroup_aprs_trigger;

//! data associated with the event group for new message received from radio network
static StaticEventGroup_t main_eventgroup_new_radio_message_rx;

//! data associated with the event group for new message received from radio network
static StaticEventGroup_t main_eventgroup_ntp_and_api_client;

//! data associated with the event group used by driver code for sx1262 modem
static StaticEventGroup_t main_eventgroup_sx1262;

//! data associated with the event group triggering transmission of various FANET frames
static StaticEventGroup_t main_eventgroup_fanet;

static TaskHandle_t task_powersave_handle = NULL;
static TaskHandle_t task_main_handle = NULL;
static TaskHandle_t task_one_sec_handle = NULL;
static TaskHandle_t task_two_sec_handle = NULL;
static TaskHandle_t task_ten_sec_handle = NULL;
static TaskHandle_t task_one_min_handle = NULL;
static TaskHandle_t task_fanet_handle = NULL;
static TaskHandle_t task_ev_serial_kiss_rx_done_handle = NULL;
static TaskHandle_t task_ev_serial_kiss_tx_done_handle = NULL;
static TaskHandle_t task_ev_serial_gsm_rx_done_handle = NULL;
static TaskHandle_t task_ev_serial_gsm_tx_done_handle = NULL;
static TaskHandle_t task_ev_serial_sensor_handle = NULL;
static TaskHandle_t task_ev_radio_message_handle = NULL;
static TaskHandle_t task_ev_ntp_and_api_client = NULL;
static TaskHandle_t task_ev_aprsis_trigger = NULL;

//! Declare a variable to hold the handle of the created event group.
EventGroupHandle_t main_eventgroup_handle_powersave;

//! Declare a variable to hold the handle of the created event group.
EventGroupHandle_t main_eventgroup_handle_serial_kiss;

//! Declare a variable to hold the handle of the created event group.
EventGroupHandle_t main_eventgroup_handle_serial_gsm;

//! Declare a variable to hold the handle of the created event group.
EventGroupHandle_t main_eventgroup_handle_serial_sensor;

EventGroupHandle_t main_eventgroup_handle_aprs_trigger;

EventGroupHandle_t main_eventgroup_handle_radio_message;

EventGroupHandle_t main_eventgroup_handle_ntp_and_api_client;

//! Declare a variable to hold the handle of the event group for triggering FANET.
EventGroupHandle_t main_eventgroup_handle_fanet;

//! a variable to hold the handle of the event group for sx1262 driver.
EventGroupHandle_t main_eventgroup_handle_sx1262;

/********************************************************************/

char main_symbol_f = '/';
char main_symbol_s = '#';

//! global variable used to store return value from various functions
volatile int retval = 100;

uint16_t buffer_len = 0;

//! return value from UMB related functions
umb_retval_t main_umb_retval = UMB_UNINITIALIZED;

//! result of CRC calculation
uint32_t main_crc_result = 0;

LL_GPIO_InitTypeDef GPIO_InitTypeDef;

gsm_sim800_state_t main_gsm_state;

uint32_t rte_main_rx_total = 0;
uint32_t rte_main_tx_total = 0;

volatile int i = 0;

//!< Value of backup_reg_get_powersave_state() at the powerup. Will be != 0 only when this is a
//!< restart
//! or powerup if RTC coin cell battery is put into a holder.
static uint16_t main_powersave_state_at_bootup = 0;

#if defined(MANUAL_RTC_SET)
uint16_t main_year = 0;
uint8_t main_month = 0;
uint8_t main_day_of_month = 0;
uint8_t main_hour = 0;
uint8_t main_minute = 0;
uint8_t main_second = 0;
#endif

char after_tx_lock;

const float main_test_float = 123.4f;

const char main_test_string[11] = "1234556aaa\0";

unsigned short rx10m = 0, tx10m = 0, digi10m = 0, digidrop10m = 0, kiss10m = 0;

// #define SERIAL_TX_TEST_MODE

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/********************************************************************/
/*************************FREE RTOS related**************************/

static void main_callback_serial_kiss_rx_done (srl_ctx_t *context)
{
	if (context->srl_rx_state == SRL_RX_DONE || context->srl_rx_state == SRL_RX_ERROR) {
		it_handlers_freertos_proxy |= IT_HANDLERS_PROXY_KISS_UART_EV;
		NVIC_SetPendingIRQ (EXTI0_IRQn);
	}
}

static void main_callback_serial_kiss_tx_done (srl_ctx_t *context)
{
	if (context->srl_tx_state == SRL_TX_IDLE) {
		it_handlers_freertos_proxy |= IT_HANDLERS_PROXY_KISS_TX_UART_EV;
		NVIC_SetPendingIRQ (EXTI0_IRQn);
	}
}

static void main_callback_serial_gsm_rx_done (srl_ctx_t *context)
{
	if (context->srl_rx_state == SRL_RX_DONE || context->srl_rx_state == SRL_RX_ERROR) {
		it_handlers_freertos_proxy |= IT_HANDLERS_PROXY_GSM_RX_UART_EV;
		NVIC_SetPendingIRQ (EXTI0_IRQn);
	}
}

static void main_callback_serial_gsm_tx_done (srl_ctx_t *context)
{
	if (context->srl_tx_state == SRL_TX_IDLE) {
		it_handlers_freertos_proxy |= IT_HANDLERS_PROXY_GSM_TX_UART_EV;
		NVIC_SetPendingIRQ (EXTI0_IRQn);
	}
}

static void main_callback_serial_sensor_rx_done (srl_ctx_t *context)
{
	if (context->srl_rx_state == SRL_RX_DONE) {
		it_handlers_freertos_proxy |= IT_HANDLERS_PROXY_WX_RX_UART_EV;
		NVIC_SetPendingIRQ (EXTI0_IRQn);
	}

	if (context->srl_rx_state == SRL_RX_ERROR) {
		it_handlers_freertos_proxy |= IT_HANDLERS_PROXY_WX_RX_ERROR_UART_EV;
		NVIC_SetPendingIRQ (EXTI0_IRQn);
	}
}

static void main_callback_serial_sensor_tx_done (srl_ctx_t *context)
{
	if (context->srl_tx_state == SRL_TX_IDLE) {
		it_handlers_freertos_proxy |= IT_HANDLERS_PROXY_WX_TX_UART_EV;
		NVIC_SetPendingIRQ (EXTI0_IRQn);
	}
}

static void main_scheduler_prestart_callback (void)
{
	main_rtos_is_runing = 1;
	event_log_rtos_running = 1;
	dallas_rtos_running = 1;
	NVIC_EnableIRQ (EXTI0_IRQn);
}
/********************************************************************/

/**
 * This is called from interrupt context!
 * @param msg
 */
static void message_callback (struct AX25Msg *msg)
{
	it_handlers_freertos_proxy |= IT_HANDLERS_PROXY_NEW_RADIO_MESSAGE_EV;
	NVIC_SetPendingIRQ (EXTI0_IRQn);
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS  -- main()
/// ==================================================================================================

/**
 * Just a main function
 * @param argc
 * @param argv
 * @return
 */
int main (int argc, char *argv[])
{

	(void)argc;
	(void)argv;

	int32_t ln = 0;

	it_handlers_inhibit_radiomodem_dcd_led = 1;

	memset (main_own_aprs_msg, 0x00, OWN_APRS_MSG_LN);

	memset (&rte_main_events_extracted_for_api_stat, 0x00, sizeof (nvm_event_result_stats_t));

	main_kiss_srl_ctx_ptr = &main_kiss_srl_ctx;
	main_wx_srl_ctx_ptr = &main_wx_srl_ctx;
	main_gsm_srl_ctx_ptr = &main_gsm_srl_ctx;

	system_clock_update_l4 ();

	if (system_clock_configure_l4 () != 0) {
		HAL_NVIC_SystemReset ();
	}

	// enable access to PWR control registers
	RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;

	system_clock_update_l4 ();

	system_clock_configure_rtc_l4 ();

	RCC->APB1ENR1 |=
		(RCC_APB1ENR1_SPI2EN | RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM3EN | RCC_APB1ENR1_TIM4EN |
		 RCC_APB1ENR1_TIM5EN | RCC_APB1ENR1_TIM7EN | RCC_APB1ENR1_USART2EN | RCC_APB1ENR1_USART3EN |
		 RCC_APB1ENR1_DAC1EN | RCC_APB1ENR1_I2C1EN | RCC_APB1ENR1_USART3EN);
	RCC->APB2ENR |= (RCC_APB2ENR_TIM1EN | RCC_APB2ENR_USART1EN | RCC_APB2ENR_TIM8EN |
					 RCC_APB2ENR_SYSCFGEN); // RCC_APB1ENR1_USART3EN
	RCC->AHB1ENR |= (RCC_AHB1ENR_CRCEN | RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMA2EN);
	RCC->AHB2ENR |= (RCC_AHB2ENR_ADCEN | RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN |
					 RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_GPIODEN);
	RCC->BDCR |= RCC_BDCR_RTCEN;

	/* Set Interrupt Group Priority */
	// HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	NVIC_SetPriorityGrouping (0);

	//  // set systick frequency
	//  HAL_SYSTICK_Config(SystemCoreClock / (1000U / (uint32_t)10));
	//
	//  // set systick interrupt priority
	it_handlers_set_priorities ();

	// configure timer used as a system timebase instead of SysTick
	TimerTimebaseConfig ();

#if defined(HI_SPEED)
	//
	// #define ADC_CCR_CKMODE_1               (0x2UL << ADC_CCR_CKMODE_Pos)           /*!<
	// 0x00020000 */
	//
	//	Bits 17:16 CKMODE[1:0]: ADC clock mode
	// 	These bits are set and cleared by software to define the ADC clock scheme (which is
	// 	common to both master and slave ADCs):
	// 	00: CK_ADCx (x=123) (Asynchronous clock mode), generated at product level (refer to
	// 		Section 6: Reset and clock control (RCC))
	// 	01: HCLK/1 (Synchronous clock mode). This configuration must be enabled only if the AHB
	// 		clock prescaler is set to 1 (HPRE[3:0] = 0xxx in RCC_CFGR register) and if the system
	// clock 		has a 50% duty cycle. 	10: HCLK/2 (Synchronous clock mode) 	11: HCLK/4 (Synchronous clock
	// mode)
	//

	ADC123_COMMON->CCR |= ADC_CCR_CKMODE_1;
	ADC123_COMMON->CCR |= ADC_CCR_CKMODE_0;
#endif

	rte_main_reboot_req = 0;

	// get powersave status after power up
	main_powersave_state_at_bootup = backup_reg_get_powersave_state ();

	// shift it two times towards less significant bit. first two bits are something else
	main_powersave_state_at_bootup = main_powersave_state_at_bootup >> 2;

	// initialize nvm logger
	nvm_event_log_init ();
	event_log_init ();

	if (main_year != 0 && main_month != 0 && main_day_of_month != 0) {
		system_set_rtc_date (main_year, main_month, main_day_of_month);
		system_set_rtc_time (main_hour, main_minute, main_second);
	}

	event_log_sync (EVENT_TIMESYNC,
					EVENT_SRC_MAIN,
					EVENTS_MAIN_TIMESYNC_BOOTUP,
					main_get_rtc_datetime (MAIN_GET_RTC_DAY),
					main_get_rtc_datetime (MAIN_GET_RTC_MONTH),
					main_get_rtc_datetime (MAIN_GET_RTC_YEAR),
					main_get_rtc_datetime (MAIN_GET_RTC_HOUR),
					main_get_rtc_datetime (MAIN_GET_RTC_MIN),
					main_get_rtc_datetime (MAIN_GET_RTC_SEC));

	// initializing variables & arrays in rte_wx
	rte_wx_init ();
	rte_rtu_init ();

	event_log_postmortem_checknstore_hardfault ();
	event_log_postmortem_checknstore_supervisor ();

	// calculate CRC over configuration blocks
	main_crc_result = configuration_handler_check_crc ();

	// restore config to default if requested
	if (main_reset_config_to_default == 1) {
		main_crc_result = 0;

		backup_reg_reset_counters ();

		backup_reg_set_configuration (0);

		//	  nvm_test_prefill();
	}

	// nvm_event_log_find_first_oldest();

	// if first section has wrong CRC and it hasn't been restored before
	if ((main_crc_result & 0x01) == 0 &&
		(backup_reg_get_configuration () & CONFIG_FIRST_FAIL_RESTORING) == 0) {
		// restore default configuration
		if (configuration_handler_restore_default_first () == 0) {

			// if configuration has been restored successfully
			backup_reg_set_bits_configuration (CONFIG_FIRST_RESTORED);

			// set also CRC flag because if restoring is successfull the region has good CRC
			backup_reg_set_bits_configuration (CONFIG_FIRST_CRC_OK);

			// additionally resets packet counters stored in backup registers
			backup_reg_reset_counters ();
		}
		else {
			// if not store the flag in the backup register to block
			// reinitializing once again in the consecutive restart
			backup_reg_set_bits_configuration (CONFIG_FIRST_FAIL_RESTORING);

			backup_reg_clear_bits_configuration (CONFIG_FIRST_CRC_OK);
		}
	}
	else {
		// if the combined confition is not met check failed restoring flag
		if ((backup_reg_get_configuration () & CONFIG_FIRST_FAIL_RESTORING) == 0) {
			// a CRC checksum is ok, so first configuration section can be used further
			backup_reg_set_bits_configuration (CONFIG_FIRST_CRC_OK);
		}
		else {
			;
		}
	}

	// if second section has wrong CRC and it hasn't been restored before
	if ((main_crc_result & 0x02) == 0 &&
		(backup_reg_get_configuration () & CONFIG_SECOND_FAIL_RESTORING) == 0) {
		// restore default configuration
		if (configuration_handler_restore_default_second () == 0) {

			// if configuration has been restored successfully
			backup_reg_set_bits_configuration (CONFIG_SECOND_RESTORED);

			// set also CRC flag as if restoring is successfull the region has good CRC
			backup_reg_set_bits_configuration (CONFIG_SECOND_CRC_OK);

			// additionally resets packet counters stored in backup registers
			backup_reg_reset_counters ();
		}
		else {
			// if not store the flag in the backup register
			backup_reg_set_bits_configuration (CONFIG_SECOND_FAIL_RESTORING);

			backup_reg_clear_bits_configuration (CONFIG_SECOND_CRC_OK);
		}
	}
	else {
		// check failed restoring flag
		if ((backup_reg_get_configuration () & CONFIG_SECOND_FAIL_RESTORING) == 0) {
			// second configuration section has good CRC and can be used further
			backup_reg_set_bits_configuration (CONFIG_SECOND_CRC_OK);
		}
		else {
			;
		}
	}

	// at this point both sections have either verified CRC or restored values to default
	if ((backup_reg_get_configuration () & CONFIG_FIRST_CRC_OK) != 0 &&
		(backup_reg_get_configuration () & CONFIG_SECOND_CRC_OK) != 0) {
		// if both sections are OK check programming counters
		if (config_data_pgm_cntr_first > config_data_pgm_cntr_second) {
			// if first section has bigger programing counter use it
			configuration_handler_load_configuration (REGION_FIRST);
		}
		else {
			configuration_handler_load_configuration (REGION_SECOND);
		}
	}
	else if ((backup_reg_get_configuration () & CONFIG_FIRST_CRC_OK) != 0 &&
			 (backup_reg_get_configuration () & CONFIG_SECOND_CRC_OK) == 0) {
		// if only first region is OK use it
		configuration_handler_load_configuration (REGION_FIRST);
	}
	else if ((backup_reg_get_configuration () & CONFIG_FIRST_CRC_OK) == 0 &&
			 (backup_reg_get_configuration () & CONFIG_SECOND_CRC_OK) != 0) {
		// if only first region is OK use it
		configuration_handler_load_configuration (REGION_FIRST);
	}
	else {
		configuration_handler_load_configuration (REGION_DEFAULT);
	}

	// set function for left button
	main_button_one_left = configuration_get_left_button ();

	// set function for right button
	main_button_two_right = configuration_get_right_button ();

	// set packets intervals
	packet_tx_init (main_config_data_basic->wx_transmit_period,
					main_config_data_basic->wx_transmit_period_forced_aggresive_pwrsave,
					main_config_data_basic->beacon_transmit_period,
					main_config_data_mode->powersave);

	// initialie telemetry frames counter
	telemetry_init ();

	// converting latitude into string
	memset (main_string_latitude, 0x00, sizeof (main_string_latitude));
	float_to_string (main_config_data_basic->latitude,
					 main_string_latitude,
					 sizeof (main_string_latitude),
					 2,
					 2);

	// converting longitude into string
	memset (main_string_longitude, 0x00, sizeof (main_string_longitude));
	float_to_string (main_config_data_basic->longitude,
					 main_string_longitude,
					 sizeof (main_string_longitude),
					 2,
					 5);

	// make a string with callsign and ssid
	if (main_config_data_basic->ssid != 0) {
		sprintf (main_callsign_with_ssid,
				 "%s-%d",
				 main_config_data_basic->callsign,
				 main_config_data_basic->ssid);
	}
	else {
		sprintf (main_callsign_with_ssid, "%s", main_config_data_basic->callsign);
	}

	switch (main_config_data_basic->symbol) {
	case 0: // _SYMBOL_DIGI
		main_symbol_f = '/';
		main_symbol_s = '#';
		break;
	case 1: // _SYMBOL_WIDE1_DIGI
		main_symbol_f = '1';
		main_symbol_s = '#';
		break;
	case 2: // _SYMBOL_HOUSE
		main_symbol_f = '/';
		main_symbol_s = '-';
		break;
	case 3: // _SYMBOL_RXIGATE
		main_symbol_f = 'I';
		main_symbol_s = '&';
		break;
	case 5: // _SYMBOL_SAILBOAT
		main_symbol_f = '/';
		main_symbol_s = 'Y';
		break;
	default: // _SYMBOL_IGATE
		main_symbol_f = 'R';
		main_symbol_s = '&';
		break;
	}

#if defined _RANDOM_DELAY
	// configuring a default delay value
	delay_set (_DELAY_BASE, 1);
#elif !defined _RANDOM_DELAY
	delay_set (_DELAY_BASE, 0);

#endif

	if (main_button_one_left != BUTTON_DISABLED || main_button_two_right != BUTTON_DISABLED) {
		// initializing GPIO used for buttons
		io_buttons_init ();
	}

	// get initial powersave mode
	rte_main_curret_powersave_mode = main_config_data_mode->powersave;

	// initialize all powersaving functions
	pwr_save_init (main_config_data_mode->powersave);

	// initialize B+ measurement
	io_vbat_meas_init (configuration_get_vbat_a_coeff (), configuration_get_vbat_b_coeff ());

	// initalizing separated Open Collector output
	io_oc_init ();

	// initializing GPIO used for swithing on and off voltages on pcb
	io_pwr_init ();

	// initialize sensor power control and switch off supply voltage
	wx_pwr_switch_init ();

	// call periodic handle to wait for 1 second and then switch on voltage
	wx_pwr_switch_periodic_handle ();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states ();

	// swtich power to M4. turn on sensors but keep GSM modem turned off
	pwr_save_switch_mode_to_c1 ();

	rte_main_reset_gsm_modem = 0;

	delay_fixed (300);

	// waiting for 1 second to count number of ticks when the CPU is idle
	main_idle_cpu_ticks = delay_fixed_with_count (1000);

	// initializing UART gpio pins
	io_uart_init ();

	main_target_kiss_baudrate = 9600u;

	// if Victron VE-direct protocol is enabled set the baudrate to the 19200u
	if (main_config_data_mode->victron == 1) {
		main_target_kiss_baudrate = 19200u;

		// and disable the kiss TNC option as it shares the same port
		main_kiss_enabled = 0;
	}

	// get target working mode of USART1
	if (main_config_data_mode->wx_davis == 1) {
		main_usart1_kiss_mode = USART_MODE_DAVIS;
	}
	else if ((main_config_data_mode->wx_dust_sensor & WX_DUST_SDS011_SERIAL) > 0) {
		main_usart1_kiss_mode = USART_MODE_DUST_SDS;
	}
	else if (main_config_data_mode->victron == 1) {
		main_usart1_kiss_mode = USART_MODE_VICTRON;
	}
	else {
		main_usart1_kiss_mode = USART_MODE_KISS;
	}

	// get target working mode for USART2
	if (main_config_data_mode->wx_modbus == 1) {
		main_usart2_wx_mode = USART_MODE_MODBUS;
	}
	else if (main_config_data_mode->wx_umb == 1) {
		main_usart2_wx_mode = USART_MODE_UMB_MASTER;
	}
	else {
		main_usart2_wx_mode = USART_MODE_UNINIT;
	}

	switch (main_usart1_kiss_mode) {
	case USART_MODE_DAVIS: {
		// reinitialize the KISS serial port temporary to davis baudrate
		main_target_kiss_baudrate = DAVIS_DEFAULT_BAUDRATE;

		// reset RX state to allow reinitialization with changed baudrate
		main_kiss_srl_ctx_ptr->srl_rx_state = SRL_RX_NOT_CONFIG;

		// reinitializing serial hardware to wake up Davis wx station
		srl_init (main_kiss_srl_ctx_ptr,
				  USART1,
				  srl_usart1_rx_buffer,
				  RX_BUFFER_1_LN,
				  srl_usart1_tx_buffer,
				  TX_BUFFER_1_LN,
				  main_target_kiss_baudrate,
				  1);

		srl_switch_timeout (main_kiss_srl_ctx_ptr, SRL_TIMEOUT_ENABLE, 3000);

		davis_init (main_kiss_srl_ctx_ptr);

		// try to wake up the davis base
		rte_wx_davis_station_avaliable = (davis_wake_up (DAVIS_BLOCKING_IO) == 0 ? 1 : 0);

		// if davis weather stations is connected to SERIAL port
		if (rte_wx_davis_station_avaliable == 1) {
			// turn LCD backlight on..
			davis_control_backlight (1);

			// wait for a while
			delay_fixed (1000);

			// and then off to let the user know that communication is working
			davis_control_backlight (0);

			// disable the KISS modem as the UART will be used for DAVIS wx station
			main_kiss_enabled = 0;

			// enable the davis serial protocol client to allow pooling callbacks to be called in
			// main loop. This only controls the callback it doesn't mean that the station itself is
			// responding to communication. It stays set to one event if Davis station
			main_davis_serial_enabled = 1;

			// trigger the rxcheck to get all counter values
			davis_trigger_rxcheck_packet ();
		}
		else {
			// if not revert back to KISS configuration
			main_target_kiss_baudrate = 9600u;
			main_kiss_srl_ctx_ptr->srl_rx_state = SRL_RX_NOT_CONFIG;

			// initializing UART drvier
			srl_init (main_kiss_srl_ctx_ptr,
					  USART1,
					  srl_usart1_rx_buffer,
					  RX_BUFFER_1_LN,
					  srl_usart1_tx_buffer,
					  TX_BUFFER_1_LN,
					  main_target_kiss_baudrate,
					  1);
			srl_set_done_error_callback (main_kiss_srl_ctx_ptr,
										 main_callback_serial_kiss_rx_done,
										 0);

			main_usart1_kiss_mode = USART_MODE_KISS;
		}
		break;
	}
	case USART_MODE_DUST_SDS: {
		srl_init (main_kiss_srl_ctx_ptr,
				  USART1,
				  srl_usart1_rx_buffer,
				  RX_BUFFER_1_LN,
				  srl_usart1_tx_buffer,
				  TX_BUFFER_1_LN,
				  9600u,
				  1);

		main_kiss_enabled = 0;

		break;
	}
	case USART_MODE_VICTRON: {
		break;
	}
	case USART_MODE_KISS: {
		srl_init (main_kiss_srl_ctx_ptr,
				  USART1,
				  srl_usart1_rx_buffer,
				  RX_BUFFER_1_LN,
				  srl_usart1_tx_buffer,
				  TX_BUFFER_1_LN,
				  main_target_kiss_baudrate,
				  1);
		srl_set_done_error_callback (main_kiss_srl_ctx_ptr,
									 main_callback_serial_kiss_rx_done,
									 main_callback_serial_kiss_tx_done);
		srl_switch_timeout (main_kiss_srl_ctx_ptr, SRL_TIMEOUT_ENABLE, 100);

		main_kiss_enabled = 1;

		break;
	}
	case USART_MODE_MODBUS:
	case USART_MODE_UMB_MASTER:
	case USART_MODE_UNINIT:
	case USART_MODE_UNDEF: main_kiss_enabled = 0; break;
	}

	switch (main_usart2_wx_mode) {
	case USART_MODE_MODBUS: {
		rtu_serial_init (&rte_rtu_pool_queue, 1, main_wx_srl_ctx_ptr, main_config_data_rtu);

		main_target_wx_baudrate = main_config_data_rtu->slave_speed;

		srl_init (main_wx_srl_ctx_ptr,
				  USART2,
				  srl_usart2_rx_buffer,
				  RX_BUFFER_2_LN,
				  srl_usart2_tx_buffer,
				  TX_BUFFER_2_LN,
				  main_target_wx_baudrate,
				  main_config_data_rtu->slave_stop_bits);
		srl_switch_tx_delay (main_wx_srl_ctx_ptr, 1);

		// enabling rtu master code
		main_modbus_rtu_master_enabled = 1;

		rtu_serial_start ();

		break;
	}
	case USART_MODE_UMB_MASTER: {
		main_target_wx_baudrate = main_config_data_umb->serial_speed;

		srl_init (main_wx_srl_ctx_ptr,
				  USART2,
				  srl_usart2_rx_buffer,
				  RX_BUFFER_2_LN,
				  srl_usart2_tx_buffer,
				  TX_BUFFER_2_LN,
				  main_target_wx_baudrate,
				  1);
		umb_master_init (&rte_wx_umb_context, main_wx_srl_ctx_ptr, main_config_data_umb);

		break;
	}
	case USART_MODE_DAVIS:
	case USART_MODE_DUST_SDS:
	case USART_MODE_VICTRON:
	case USART_MODE_KISS:
	case USART_MODE_UNINIT:
	case USART_MODE_UNDEF: break;
	}

	srl_set_done_error_callback (main_wx_srl_ctx_ptr,
								 main_callback_serial_sensor_rx_done,
								 main_callback_serial_sensor_tx_done);

	main_wx_srl_ctx_ptr->te_pin = LL_GPIO_PIN_8;
	main_wx_srl_ctx_ptr->te_port = GPIOA;
	main_wx_srl_ctx_ptr->early_tx_assert = configuration_get_early_tx_assert (); // TODO: was 1

	// initialize UART used to communicate with GPRS modem
	srl_init (main_gsm_srl_ctx_ptr,
			  USART3,
			  srl_usart3_rx_buffer,
			  RX_BUFFER_3_LN,
			  srl_usart3_tx_buffer,
			  TX_BUFFER_3_LN,
			  115200,
			  1);
	srl_set_done_error_callback (main_gsm_srl_ctx_ptr,
								 main_callback_serial_gsm_rx_done,
								 main_callback_serial_gsm_tx_done);

	// initialize APRS path with zeros
	memset (main_own_path, 0x00, sizeof (main_own_path));

	// configuring an APRS path used to transmit own packets (telemetry, wx, beacons)
	main_own_path_ln = ConfigPath (main_own_path, main_config_data_basic);

#ifdef INTERNAL_WATCHDOG
	// enable watchdog
	LL_IWDG_Enable (IWDG);

	// unlock write access to configuratio registers
	LL_IWDG_EnableWriteAccess (IWDG);

	// set prescaler - watchdog timeout on about 32 seconds
	LL_IWDG_SetPrescaler (IWDG, LL_IWDG_PRESCALER_256);

	// wait for watchdog registers to update
	while (LL_IWDG_IsActiveFlag_PVU (IWDG) != 0) {
		i++;

		if (i > 0x9FF) {
			break;
		}
	}

	// reload watchdog which also close access to configiguration registers
	LL_IWDG_ReloadCounter (IWDG);

	i = 0;

	// do not disable watchdog when MCU halts on breakpoints
	DBGMCU->APB1FZR1 &= (0xFFFFFFFF ^ DBGMCU_APB1FZR1_DBG_IWDG_STOP);

#endif

	// initialize i2c controller
	i2cConfigure ();

	// initialize SPI
	spi_init_full_duplex_pio (SPI_MASTER_MOTOROLA,
							  CLOCK_NORMAL_RISING,
							  SPI_SPEED_DIV256,
							  SPI_ENDIAN_MSB);

	// initialize measurements averager
	max31865_init_average ();

	// initialize MAX RDT amplifier
	max31865_init (main_config_data_mode->wx_pt_sensor & 0x3,
				   (main_config_data_mode->wx_pt_sensor & 0xFC) >> 2);

	// initialize GPIO pins leds are connecting to
	led_init ();

	// initialize AX25 & APRS stuff
	AFSK_Init (&main_afsk);
	ax25_init (&main_ax25, &main_afsk, 0, 0x00, 0x00);
	DA_Init ();

	// configure external watchdog
	io_ext_watchdog_config ();

	// initializing the digipeater configuration
	digi_init (main_config_data_mode);

	if ((main_config_data_mode->wx & WX_ENABLED) == 1) {
		if (configuration_get_disable_dallas () == 0) {
			// initialize dallas one-wire driver for termometer
			dallas_init (GPIOC, LL_GPIO_PIN_11, 0x0, &rte_wx_dallas_average);
		}

		if ((main_config_data_mode->wx & WX_INTERNAL_SPARKFUN_WIND) == 0) {
			analog_anemometer_init (main_config_data_mode->wx_anemometer_pulses_constant,
									38,
									100,
									1);
		}
		else {
			analog_anemometer_init (main_config_data_mode->wx_anemometer_pulses_constant,
									38,
									100,
									1);
		}
	}

	// configuring interrupt priorities
	it_handlers_set_priorities ();

	// read calibration data from I2C pressure / humidity sensor
	if (main_config_data_mode->wx_ms5611_or_bme == 0) {
		ms5611_reset (&rte_wx_ms5611_qf);
		ms5611_read_calibration (SensorCalData, &rte_wx_ms5611_qf);
		ms5611_trigger_measure (0, 0);
	}
	else if (main_config_data_mode->wx_ms5611_or_bme == 1) {
		bme280_reset (&rte_wx_bme280_qf);
		bme280_setup ();
		bme280_read_calibration (bme280_calibration_data);
	}
	else {
		; // no internal sensor enabled
	}

	kiss_security_access_init (main_config_data_basic);

	if (main_kiss_enabled == 1) {
		// preparing initial beacon which will be sent to host PC using KISS protocol via UART
		main_own_aprs_msg_len = snprintf (main_own_aprs_msg,
										  OWN_APRS_MSG_LN,
										  "=%s%c%c%s%c%c %s",
										  main_string_latitude,
										  main_config_data_basic->n_or_s,
										  main_symbol_f,
										  main_string_longitude,
										  main_config_data_basic->e_or_w,
										  main_symbol_s,
										  main_config_data_basic->comment);

		// terminating the aprs message
		main_own_aprs_msg[main_own_aprs_msg_len] = 0;

		// 'sending' the message which will only encapsulate it inside AX25 protocol (ax25_starttx
		// is not called here)
		// ax25_sendVia(&main_ax25, main_own_path, (sizeof(main_own_path) /
		// sizeof(*(main_own_path))), main_own_aprs_msg, main_own_aprs_msg_len);
		ln = ax25_sendVia_toBuffer (main_own_path,
									(sizeof (main_own_path) / sizeof (*(main_own_path))),
									main_own_aprs_msg,
									main_own_aprs_msg_len,
									main_kiss_srl_ctx.srl_tx_buf_pointer,
									TX_BUFFER_1_LN);

		// SendKISSToHost function cleares the output buffer hence routine need to wait till the
		// UART will be ready for next transmission. Here this could be omitted because UART isn't
		// used before but general idea
		while (main_kiss_srl_ctx.srl_tx_state != SRL_TX_IDLE &&
			   main_kiss_srl_ctx.srl_tx_state != SRL_TX_ERROR)
			;

		// converting AX25 with beacon to KISS format
		// ln = SendKISSToHost(main_afsk.tx_buf + 1, main_afsk.tx_fifo.tail - main_afsk.tx_fifo.head
		// - 4, srl_tx_buffer, TX_BUFFER_LN);

		// checking if KISS-framing was done correctly
		if (ln != KISS_TOO_LONG_FRM) {
#ifdef SERIAL_TX_TEST_MODE
			// infinite loop for testing UART transmission
			for (;;) {

				retval = srl_receive_data (main_kiss_srl_ctx_ptr, 100, '\r', '\r', 0, 0, 0);
#endif
				retval = srl_start_tx (main_kiss_srl_ctx_ptr, ln);

#ifdef SERIAL_TX_TEST_MODE
				while (main_kiss_srl_ctx_ptr->srl_tx_state != SRL_TX_IDLE)
					;

#if defined(PARAMETEO)
				LL_GPIO_TogglePin (GPIOC, LL_GPIO_PIN_9);
#else
				GPIOC->ODR = (GPIOC->ODR ^ GPIO_Pin_9);
#endif

				if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE) {
#if defined(PARAMETEO)
					LL_GPIO_TogglePin (GPIOC, LL_GPIO_PIN_9);
#else
					GPIOC->ODR = (GPIOC->ODR ^ GPIO_Pin_9);
#endif
					retval = 200;
				}
			}
#endif
		}
	}

	// reinitializing AFSK and AX25 driver
	AFSK_Init (&main_afsk);

	ADCStartConfig ();
	DACStartConfig ();
	AFSK_Init (&main_afsk);
	ax25_init (&main_ax25, &main_afsk, 0, message_callback, 0);

	if ((main_config_data_mode->wx & WX_ENABLED) == 1) {
		max31865_pool_synchro ();

		// getting all meteo measuremenets to be sure that WX frames want be sent with zeros
		wx_get_all_measurements (main_config_data_wx_sources,
								 main_config_data_mode,
								 main_config_data_umb,
								 main_config_data_rtu);
	}

	// start serial port i/o transaction depending on station configuration
	if (main_config_data_mode->victron == 1) {
		// initializing protocol parser
		ve_direct_parser_init (&rte_pv_struct, &rte_pv_average);

		// enabling timeout handling for serial port. This is required because VE protocol frame may
		// vary in lenght and serial port driver could finish reception only either on stop
		// character or when declared number of bytes has been received.
		srl_switch_timeout (main_kiss_srl_ctx_ptr, 1, 50);

		// switching UART to receive mode to be ready for data from charging controller
		srl_receive_data (main_kiss_srl_ctx_ptr, VE_DIRECT_MAX_FRAME_LN, 0, 0, 0, 0, 0);
	}
	else {
		// switching UART to receive mode to be ready for KISS frames from host
		srl_receive_data (main_kiss_srl_ctx_ptr, 100, FEND, FEND, 0, 0, 0);
	}

	io_oc_output_low ();

	led_control_led1_upper (false);
	led_control_led2_bottom (false);

	io_vbat_meas_fill (&rte_main_battery_voltage, &rte_main_average_battery_voltage);

	if (main_config_data_mode->gsm == 1) {
		pwr_save_switch_mode_to_c0 ();
	}

	if (NVIC_GetEnableIRQ (TIM1_UP_TIM16_IRQn) == 0U) {
		backup_assert (BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_OTHER);
	}

	supervisor_iam_alive (SUPERVISOR_THREAD_MAIN_LOOP);

	sx1262_init ();

	fanet_test_init ();

	// rte_main_battery_voltage = io_vbat_meas_get_synchro();

	supervisor_iam_alive (SUPERVISOR_THREAD_MAIN_LOOP);

	// sleep a little bit and wait for everything to power up completely
	delay_fixed (1000);

	led_control_led1_upper (true);
	led_control_led2_bottom (false);

	delay_fixed (1000);

	led_control_led1_upper (false);
	led_control_led2_bottom (true);

	delay_fixed (1000);

	led_control_led1_upper (true);
	led_control_led2_bottom (true);

	delay_fixed (1000);

	led_control_led1_upper (false);
	led_control_led2_bottom (false);

	supervisor_iam_alive (SUPERVISOR_THREAD_MAIN_LOOP);

	// configuring system timers
	TimerConfig ();

	// initialize UMB transaction
	if (main_config_data_mode->wx_umb == 1) {
		umb_0x26_status_request (&rte_wx_umb, &rte_wx_umb_context, main_config_data_umb);
	}

	// reload watchdog counter
	main_reload_internal_wdg ();

	// reload external watchdog in case it is installed
	io_ext_watchdog_service ();

	// initialize NVM logger if it is enabled
	if (main_config_data_mode->nvm_logger != 0) {
		nvm_measurement_init ();
	}

	// initialize everything related to GSM module
	if (main_config_data_mode->gsm == 1) {
		it_handlers_inhibit_radiomodem_dcd_led = 1;

		led_control_led1_upper (false);

		gsm_sim800_init (&main_gsm_state, 1);

		http_client_init (&main_gsm_state, main_gsm_srl_ctx_ptr, 0);

		ntp_init (main_gsm_srl_ctx_ptr, &main_gsm_state);

		// as for now it is not possible to have APRS-IS communciation and REST api at once,
		// due to some data races and another timing problems while disconnecting APRS-IS to make
		// room for HTTP request - hence that if below
		// if (main_config_data_gsm->api_enable == 1 && main_config_data_gsm->aprsis_enable == 0) {
		api_init ((const char *)main_config_data_gsm->api_base_url,
				  (const char *)(main_config_data_gsm->api_station_name));
		//}

		if (main_config_data_gsm->api_enable == 0 && main_config_data_gsm->aprsis_enable == 1) {
			aprsis_init (&main_gsm_srl_ctx,
						 &main_gsm_state,
						 (const char *)main_config_data_basic->callsign,
						 main_config_data_basic->ssid,
						 main_config_data_gsm->aprsis_passcode,
						 (const char *)main_config_data_gsm->aprsis_server_address,
						 main_config_data_gsm->aprsis_server_port,
						 configuration_get_power_cycle_gsmradio_on_no_communications (),
						 main_callsign_with_ssid);
		}

		if (backup_reg_get_inhibit_log_report_send_api () == 0) {
			// extract events from NVM to then sent them into the API
			rte_main_events_extracted_for_api_stat =
				nvm_event_get_last_events_in_exposed (rte_main_exposed_events,
													  MAIN_HOW_MANY_EVENTS_SEND_REPORT * 3,
													  EVENT_INFO_CYCLIC);
		}
		else {
			backup_reg_reset_inhibit_log_report_send_api ();
		}
	}

	if (NVIC_GetEnableIRQ (TIM1_UP_TIM16_IRQn) == 0U) {
		backup_assert (BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_OTHER);
	}

	if (main_config_data_basic->beacon_at_bootup == 1) {
		beacon_send_own (rte_main_battery_voltage, system_is_rtc_ok ());
		main_wait_for_tx_complete ();

		// this delay is put in case if beacon is configured to use
		// any path like WIDE1-1 or WIDE2-1 or another. The delay
		// will wait for some time to have this beacon digipeated
		// by the APRS radio network
		delay_fixed (1500);

		status_send_powersave_registers ();
	}

	(void)event_log_sync (EVENT_BOOTUP,
						  EVENT_SRC_MAIN,
						  EVENTS_MAIN_BOOTUP_COMPLETE,
						  (uint8_t)system_is_rtc_ok (),
						  (uint8_t)main_powersave_state_at_bootup,
						  rte_main_battery_voltage,
						  rte_main_average_battery_voltage,
						  0u,
						  backup_reg_get_sleep_counter ());

	main_nvm_timestamp = main_get_nvm_timestamp ();

	it_handlers_inhibit_radiomodem_dcd_led = 0;

	packet_tx_meteo_counter = main_config_data_basic->wx_transmit_period - 1;

	////////////////////////// FREERTOS       /////////////////////////////////
	///

	main_eventgroup_handle_powersave = xEventGroupCreateStatic (&main_eventgroup_powersave);
	main_eventgroup_handle_serial_kiss = xEventGroupCreateStatic (&main_eventgroup_serial_kiss);
	main_eventgroup_handle_serial_gsm = xEventGroupCreateStatic (&main_eventgroup_serial_gsm);
	main_eventgroup_handle_serial_sensor = xEventGroupCreateStatic (&main_eventgroup_serial_sensor);
	main_eventgroup_handle_aprs_trigger = xEventGroupCreateStatic (&main_eventgroup_aprs_trigger);
	main_eventgroup_handle_radio_message =
		xEventGroupCreateStatic (&main_eventgroup_new_radio_message_rx);
	main_eventgroup_handle_ntp_and_api_client =
		xEventGroupCreateStatic (&main_eventgroup_ntp_and_api_client);
	main_eventgroup_handle_sx1262 = xEventGroupCreateStatic (&main_eventgroup_sx1262);
	main_eventgroup_handle_fanet = xEventGroupCreateStatic (&main_eventgroup_fanet);

	main_mutex_gsm_tcpip = xSemaphoreCreateMutex ();

	MAIN_EXPAND_TASKS_LIST

	///
	///////////////////////////////////////////////////////////////////////////

	// Infinite loop
	while (1) {

		supervisor_iam_alive (SUPERVISOR_THREAD_MAIN_LOOP);

		// incrementing current cpu ticks
		main_current_cpu_idle_ticks++;

		// system reset may be requested from various places in the application
		if (rte_main_reboot_req == 1) {
			NVIC_SystemReset ();
		}
		else {
			i = __NVIC_PRIO_BITS;
		}

	} // Infinite loop, never return.
}

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

void main_set_ax25_my_callsign (AX25Call *call)
{
	if (variant_validate_is_within_ram (call)) {
		strncpy (call->call, main_config_data_basic->callsign, 6);
		call->ssid = main_config_data_basic->ssid;
	}
}

void main_copy_ax25_call (AX25Call *to, AX25Call *from)
{
	if (variant_validate_is_within_ram (to)) {
		strncpy (to->call, from->call, 6);
		to->ssid = from->ssid;
	}
}

/**
 *
 * @return
 */
uint16_t main_get_adc_sample (void)
{
	return (uint16_t)ADC1->DR;
}

/**
 *
 */
void main_service_cpu_load_ticks (void)
{

	uint32_t cpu_ticks_load = 0;

	// the biggest this result will be the biggest load the CPU is handling
	cpu_ticks_load = main_idle_cpu_ticks - main_current_cpu_idle_ticks;

	// calculate the cpu load
	main_cpu_load = (int8_t)((cpu_ticks_load * 100) / main_idle_cpu_ticks);

	// reset the tick counter back to zero;
	main_current_cpu_idle_ticks = 0;
}

void main_reload_internal_wdg (void)
{
	LL_IWDG_ReloadCounter (IWDG);
}

uint16_t main_get_rtc_datetime (uint16_t param)
{

	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;

	uint16_t out = 0;

	RTC_HandleTypeDef rtc;
	rtc.Instance = RTC;

	HAL_RTC_GetTime (&rtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate (&rtc, &date, RTC_FORMAT_BIN);

	switch (param) {
	case MAIN_GET_RTC_YEAR: out = date.Year; break;
	case MAIN_GET_RTC_MONTH: out = date.Month; break;
	case MAIN_GET_RTC_DAY: out = date.Date; break;
	case MAIN_GET_RTC_HOUR: out = time.Hours; break;
	case MAIN_GET_RTC_MIN: out = time.Minutes; break;
	case MAIN_GET_RTC_SEC: out = time.Seconds; break;
	}

	return out;
}

uint32_t main_get_nvm_timestamp (void)
{
	uint32_t out = 0;

	/**
	 * Date-time timestamp in timezone local for a place where station is installed.
	 * Mixture of BCD and integer format, this is just sligtly processed RTC registers
	 * content.
	 *	bit 0  - bit 10 === number of minutes starting from midnight (max 1440)
	 *	bit 16 - bit 24 === days from new year (max 356)
	 *	bit 25 - bit 31 === years (from 00 to 99, from 2000 up to 2099)
	 */

	uint16_t temp = 0;

	const uint16_t real_year = 2000u + 10 * ((RTC->DR & RTC_DR_YT) >> RTC_DR_YT_Pos) +
							   1 * ((RTC->DR & RTC_DR_YU) >> RTC_DR_YU_Pos);

	const bool is_leap = ((real_year % 4) == 0) ? true : false;

	// minutes
	temp = 600 * ((RTC->TR & RTC_TR_HT) >> RTC_TR_HT_Pos) +
		   60 * ((RTC->TR & RTC_TR_HU) >> RTC_TR_HU_Pos) +
		   10 * ((RTC->TR & RTC_TR_MNT) >> RTC_TR_MNT_Pos) +
		   1 * ((RTC->TR & RTC_TR_MNU) >> RTC_TR_MNU_Pos);

	out = out | (temp & 0x7FF);

	// current month
	temp = 1 * ((RTC->DR & RTC_DR_MU) >> RTC_DR_MU_Pos) +
		   10 * ((RTC->DR & RTC_DR_MT) >> RTC_DR_MT_Pos);

	if (is_leap == true) {
		switch (temp) {
		case 1: temp = 0; break;
		case 2: temp = 31; break;
		case 3: temp = 31 + 29; break;
		case 4: temp = 31 + 29 + 31; break;
		case 5: temp = 31 + 29 + 31 + 30; break;
		case 6: temp = 31 + 29 + 31 + 30 + 31; break;
		case 7: temp = 31 + 29 + 31 + 30 + 31 + 30; break;
		case 8: temp = 31 + 29 + 31 + 30 + 31 + 30 + 31; break;
		case 9: temp = 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31; break;
		case 10: temp = 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30; break;
		case 11: temp = 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31; break;
		case 12: temp = 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30; break;
		}
	}
	else {
		switch (temp) {
		case 1: temp = 0; break;
		case 2: temp = 31; break;
		case 3: temp = 31 + 28; break;
		case 4: temp = 31 + 28 + 31; break;
		case 5: temp = 31 + 28 + 31 + 30; break;
		case 6: temp = 31 + 28 + 31 + 30 + 31; break;
		case 7: temp = 31 + 28 + 31 + 30 + 31 + 30; break;
		case 8: temp = 31 + 28 + 31 + 30 + 31 + 30 + 31; break;
		case 9: temp = 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31; break;
		case 10: temp = 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30; break;
		case 11: temp = 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31; break;
		case 12: temp = 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30; break;
		}
	}

	// then add number of days from current month
	temp = temp + 1 * ((RTC->DR & RTC_DR_DU) >> RTC_DR_DU_Pos) +
		   10 * ((RTC->DR & RTC_DR_DT) >> RTC_DR_DT_Pos);

	out = out | ((temp & 0x1FF) << 16);

	// years
	temp = 10 * ((RTC->DR & RTC_DR_YT) >> RTC_DR_YT_Pos) +
		   1 * ((RTC->DR & RTC_DR_YU) >> RTC_DR_YU_Pos);

	out = out | ((temp & 0x7F) << 25);

	return out;
}

uint32_t main_get_master_time_highres (void)
{
	// this is incremented by 10ms, variable resolution is 1ms
	uint32_t out = master_time;

	// counter runs with input frequency of 100kHz (period: 10us)
	uint32_t counter = TIM1->CNT;

	// scale to 10us resolution
	uint32_t tens_us = out * 100;

	out = tens_us + counter;

	return out;
}

srl_context_t *main_get_kiss_srl_ctx_ptr (void)
{
	return main_kiss_srl_ctx_ptr;
}

uint8_t main_get_main_davis_serial_enabled (void)
{
	return main_davis_serial_enabled;
}

uint8_t main_get_modbus_rtu_master_enabled (void)
{
	return main_modbus_rtu_master_enabled;
}

//! function configuration for left button on ParaMETEO
configuration_button_function_t main_get_button_one_left ()
{
	return main_button_one_left;
}

//! function configuration for right button on ParaMETEO
configuration_button_function_t main_get_button_two_right ()
{
	return main_button_two_right;
}

/********************************************************************/
/*************************FREE RTOS related**************************/

void main_suspend_task_for_psaving (void)
{

	vTaskSuspend (task_one_sec_handle);
	vTaskSuspend (task_two_sec_handle);
	vTaskSuspend (task_ten_sec_handle);
	vTaskSuspend (task_one_min_handle);
	// not suspended on purpose! keep this commented
	// vTaskSuspend(task_main_handle);
	vTaskSuspend (task_fanet_handle);

	/*
	 * TODO: There is a bug related to serial port got stuck
	 * because of these two (mainly rx) tasks being suspended.
	 * What happens (probably): When the task is suspended in the middle
	 * of data reception, an RX_DONE event is not serviced by the task.
	 * After the task is resumed, it for some reason forgot that an event
	 * is set in the event group, so RX_DONE is never serviced and KISS
	 * diagnostics and KISS modem interface doesn't work.
	 */
	//	vTaskSuspend (task_ev_serial_kiss_rx_done_handle);
	//	vTaskSuspend (task_ev_serial_kiss_tx_done_handle);
	vTaskSuspend (task_ev_serial_gsm_rx_done_handle);
	vTaskSuspend (task_ev_serial_gsm_tx_done_handle);
	vTaskSuspend (task_ev_serial_sensor_handle);
	vTaskSuspend (task_ev_radio_message_handle);
	vTaskSuspend (task_ev_aprsis_trigger);
}

void main_resume_task_for_psaving (void)
{
	vTaskResume (task_one_sec_handle);
	vTaskResume (task_two_sec_handle);
	vTaskResume (task_ten_sec_handle);
	vTaskResume (task_one_min_handle);
	// vTaskResume(task_main_handle);
	vTaskResume (task_fanet_handle);
	//	vTaskResume(task_ev_serial_kiss_rx_done_handle);
	//	vTaskResume(task_ev_serial_kiss_tx_done_handle);
	vTaskResume (task_ev_serial_gsm_rx_done_handle);
	vTaskResume (task_ev_serial_gsm_tx_done_handle);
	vTaskResume (task_ev_serial_sensor_handle);
	vTaskResume (task_ev_radio_message_handle);
	vTaskResume (task_ev_aprsis_trigger);
}

void main_get_tasks_stats ()
{
	rte_main_load.task_powersave = ulTaskGetRunTimeCounter (task_powersave_handle);
	rte_main_load.task_main = ulTaskGetRunTimeCounter (task_main_handle);
	rte_main_load.task_one_sec = ulTaskGetRunTimeCounter (task_one_sec_handle);
	rte_main_load.task_two_sec = ulTaskGetRunTimeCounter (task_two_sec_handle);
	rte_main_load.task_ten_sec = ulTaskGetRunTimeCounter (task_ten_sec_handle);
	rte_main_load.task_one_min = ulTaskGetRunTimeCounter (task_one_min_handle);

	rte_main_load.tev_serial_kiss = ulTaskGetRunTimeCounter (task_ev_serial_kiss_rx_done_handle);
	rte_main_load.tev_serial_kiss_tx = ulTaskGetRunTimeCounter (task_ev_serial_kiss_tx_done_handle);
	rte_main_load.tev_serial_gsm_rx = ulTaskGetRunTimeCounter (task_ev_serial_gsm_rx_done_handle);
	rte_main_load.tev_serial_gsm_tx = ulTaskGetRunTimeCounter (task_ev_serial_gsm_tx_done_handle);
	rte_main_load.tev_radio_message = ulTaskGetRunTimeCounter (task_ev_radio_message_handle);
	rte_main_load.tev_ntp_api = ulTaskGetRunTimeCounter (task_ev_ntp_and_api_client);

	rte_main_state.task_powersave = eTaskGetState (task_powersave_handle);
	rte_main_state.task_main = eTaskGetState (task_main_handle);
	rte_main_state.task_one_sec = eTaskGetState (task_one_sec_handle);
	rte_main_state.task_two_sec = eTaskGetState (task_two_sec_handle);
	rte_main_state.task_ten_sec = eTaskGetState (task_ten_sec_handle);
	rte_main_state.task_one_min = eTaskGetState (task_one_min_handle);

	rte_main_state.tev_serial_kiss = eTaskGetState (task_ev_serial_kiss_rx_done_handle);
	rte_main_state.tev_serial_kiss_tx = eTaskGetState (task_ev_serial_kiss_tx_done_handle);
	rte_main_state.tev_serial_gsm_rx = eTaskGetState (task_ev_serial_gsm_rx_done_handle);
	rte_main_state.tev_serial_gsm_tx = eTaskGetState (task_ev_serial_gsm_tx_done_handle);
	rte_main_state.tev_radio_message = eTaskGetState (task_ev_radio_message_handle);
	rte_main_state.tev_ntp_api = eTaskGetState (task_ev_ntp_and_api_client);
	rte_main_state.tev_apris_trig = eTaskGetState (task_ev_aprsis_trigger);

	rte_main_state.master_time = master_time;
}

/**
 *
 * @param what_to_do
 */
void main_handle_mutex_gsm_tcpip (uint8_t what_to_do)
{
	if (what_to_do == 1) {
		xSemaphoreTake (main_mutex_gsm_tcpip, portMAX_DELAY);
	}
	else {
		xSemaphoreGive (main_mutex_gsm_tcpip);
	}
}

void vApplicationStackOverflowHook (TaskHandle_t xTask, char *pcTaskName)
{
	(void)xTask;
	(void)pcTaskName;
	rte_main_reboot_req = 1;
}

void vApplicationMallocFailedHook (void)
{
	rte_main_reboot_req = 1;
}

void vApplicationGetRandomHeapCanary (portPOINTER_SIZE_TYPE *pxHeapCanary)
{
	if (pxHeapCanary != NULL) {
		*pxHeapCanary = main_get_nvm_timestamp ();
	}
}

BaseType_t xApplicationMemoryPermissions (uint32_t aAddress)
{

	/*
	 * Return 1 for readable, 2 for writeable, 3 for both.
	 * Function must be provided by the application.
	 */
	BaseType_t out = 0;

	if (variant_validate_is_within_ram (aAddress) == 1) {
		out = 3;
	}
	else if (variant_validate_is_within_flash (aAddress) == 1) {
		out = 1;
	}
	else {
		;
	}

	return out;
}

/********************************************************************/

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
