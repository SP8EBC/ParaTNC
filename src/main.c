#include "station_config_target_hw.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x_rcc.h>
#include <stm32f10x_iwdg.h>
#include <stm32f10x.h>
#include <drivers/f1/gpio_conf_stm32f1x.h>
#endif

#ifdef STM32L471xx
#include <stm32l4xx_hal_cortex.h>
#include <stm32l4xx.h>
#include <stm32l4xx_ll_iwdg.h>
#include <stm32l4xx_ll_rcc.h>
#include <stm32l4xx_ll_gpio.h>
#include "cmsis/stm32l4xx/system_stm32l4xx.h"

#include "gsm/sim800c.h"
#endif

#include <delay.h>
#include <LedConfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "packet_tx_handler.h"

#include "station_config.h"
#include "config_data_externs.h"
#include "configuration_handler.h"

#include "diag/Trace.h"
#include "antilib_adc.h"
#include "afsk_pr.h"
#include "TimerConfig.h"
#include "PathConfig.h"
#include "LedConfig.h"
#include "io.h"
#include "float_to_string.h"
#include "pwr_save.h"
#include "pwr_switch.h"

#include "it_handlers.h"

#include "aprs/digi.h"
#include "aprs/telemetry.h"
#include "aprs/dac.h"
#include "aprs/beacon.h"

#include "ve_direct_protocol/parser.h"

#include "rte_wx.h"
#include "rte_pv.h"
#include "rte_main.h"
#include "rte_rtu.h"

#include <wx_handler.h>
#include "drivers/dallas.h"
#include "drivers/i2c.h"
#include "drivers/analog_anemometer.h"
#include "aprs/wx.h"

#include "../system/include/modbus_rtu/rtu_serial_io.h"

#include "../system/include/davis_vantage/davis.h"
#include "../system/include/davis_vantage/davis_parsers.h"

#include "drivers/ms5611.h"
#include <drivers/bme280.h>

#include "umb_master/umb_master.h"
#include "umb_master/umb_channel_pool.h"
#include "umb_master/umb_0x26_status.h"

#include "drivers/dallas.h"

#include "KissCommunication.h"

#define SOH 0x01


//#define SERIAL_TX_TEST_MODE

// Niebieska dioda -> DCD
// Zielona dioda -> anemometr albo TX

// backup registers (ParaTNC)
// 0 ->
// 2 -> boot and hard fault count
// 3 -> controller configuration status
// 4 ->
// 5 ->
// 6 ->

// backup registers (ParaMETEO)
// 0 -> powersave status
// 3 -> controller configuration status


#define CONFIG_FIRST_RESTORED 			(1)
#define CONFIG_FIRST_FAIL_RESTORING	  	(1 << 1)
#define CONFIG_FIRST_CRC_OK				(1 << 2)

#define CONFIG_SECOND_RESTORED 				(1 << 3)
#define CONFIG_SECOND_FAIL_RESTORING	  	(1 << 4)
#define CONFIG_SECOND_CRC_OK				(1 << 5)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wempty-body"

// used configuration structures
const config_data_mode_t * main_config_data_mode = 0;
const config_data_basic_t * main_config_data_basic = 0;
const config_data_wx_sources_t * main_config_data_wx_sources = 0;
const config_data_umb_t * main_config_data_umb = 0;
const config_data_rtu_t * main_config_data_rtu = 0;

// global variable incremented by the SysTick handler to measure time in miliseconds
uint32_t master_time = 0;

// this global variable stores numbers of ticks of idling CPU
uint32_t main_idle_cpu_ticks = 0;

// current cpu idle ticks
uint32_t main_current_cpu_idle_ticks = 0;

// approx cpu load in percents
int8_t main_cpu_load = 0;

// global variable used as a timer to trigger meteo sensors mesurements
int32_t main_wx_sensors_pool_timer = 65500;

// global variable used as a timer to trigger packet sending
int32_t main_one_minute_pool_timer = 45000;

// one second pool interval
int32_t main_one_second_pool_timer = 1000;

// two second pool interval
int32_t main_two_second_pool_timer = 2000;

// ten second pool interval
int32_t main_ten_second_pool_timer = 10000;

// serial context for UART used to KISS
srl_context_t main_kiss_srl_ctx;

// serial context for UART used for comm with wx sensors
srl_context_t main_wx_srl_ctx;

#if defined(PARAMETEO)
// serial context for communication with GSM module
srl_context_t main_gsm_srl_ctx;
#endif

// a pointer to KISS context
srl_context_t* main_kiss_srl_ctx_ptr;

// a pointer to wx comms context
srl_context_t* main_wx_srl_ctx_ptr;

// a pointer to gsm context
srl_context_t* main_gsm_srl_ctx_ptr;

// target USART1 (kiss) baudrate
uint32_t main_target_kiss_baudrate;

// target USART2 (wx) baudrate
uint32_t main_target_wx_baudrate;

// controls if the KISS modem is enabled
uint8_t main_kiss_enabled = 1;

// controls if DAVIS serialprotocol client is enabled by the configuration
uint8_t main_davis_serial_enabled = 0;

uint8_t main_modbus_rtu_master_enabled = 0;

// global variables represending the AX25/APRS stack
AX25Ctx main_ax25;
Afsk main_afsk;


AX25Call main_own_path[3];
uint8_t main_own_path_ln = 0;
uint8_t main_own_aprs_msg_len;
char main_own_aprs_msg[OWN_APRS_MSG_LN];

char main_string_latitude[9];
char main_string_longitude[9];

char main_symbol_f = '/';
char main_symbol_s = '#';

// global variable used to store return value from various functions
volatile uint8_t retval = 100;

uint16_t buffer_len = 0;

// return value from UMB related functions
umb_retval_t main_umb_retval = UMB_UNINITIALIZED;

// result of CRC calculation
uint32_t main_crc_result = 0;

char after_tx_lock;

unsigned short rx10m = 0, tx10m = 0, digi10m = 0, digidrop10m = 0, kiss10m = 0;

#if defined(PARAMETEO)
LL_GPIO_InitTypeDef GPIO_InitTypeDef;

gsm_sim800_state_t main_gsm_state;
#endif

static void message_callback(struct AX25Msg *msg) {

}

int main(int argc, char* argv[]){

  int32_t ln = 0;

  uint8_t button_inhibit = 0;

  memset(main_own_aprs_msg, 0x00, OWN_APRS_MSG_LN);

#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
  RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM7EN | RCC_APB1ENR_TIM4EN);
  RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_TIM1EN);
  RCC->AHBENR |= RCC_AHBENR_CRCEN;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  // choosing the signal source for the SysTick timer.
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  // Configuring the SysTick timer to generate interrupt 100x per second (one interrupt = 10ms)
  SysTick_Config(SystemCoreClock / SYSTICK_TICKS_PER_SECONDS);

  // setting an Systick interrupt priority
  NVIC_SetPriority(SysTick_IRQn, 5);

  // enable access to BKP registers
  RCC->APB1ENR |= (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN);
  PWR->CR |= PWR_CR_DBP;

  // read current number of boot cycles
  rte_main_boot_cycles = (uint8_t)(BKP->DR2 & 0xFF);

  // read current number of hard faults
  rte_main_hard_faults = (uint8_t)((BKP->DR2 & 0xFF00) >> 8);

  // increase boot cycles count
  rte_main_boot_cycles++;

  // erasing old value from backup registers
  BKP->DR2 &= (0xFFFF ^ 0xFF);

  // storing increased value
  BKP->DR2 |= rte_main_boot_cycles;

  BKP->DR3 = 0;
  BKP->DR4 = 0;
  BKP->DR5 = 0;
  BKP->DR6 = 0;
#endif

#if defined(PARAMETEO)
  system_clock_update_l4();

  if (system_clock_configure_l4() != 0) {
	  HAL_NVIC_SystemReset();

  }

  // enable access to PWR control registers
  RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;

  system_clock_update_l4();

  system_clock_configure_rtc_l4();

  RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM3EN | RCC_APB1ENR1_TIM4EN | RCC_APB1ENR1_TIM5EN | RCC_APB1ENR1_TIM7EN | RCC_APB1ENR1_USART2EN | RCC_APB1ENR1_USART3EN | RCC_APB1ENR1_DAC1EN | RCC_APB1ENR1_I2C1EN | RCC_APB1ENR1_USART3EN);
  RCC->APB2ENR |= (RCC_APB2ENR_TIM1EN | RCC_APB2ENR_USART1EN); // RCC_APB1ENR1_USART3EN
  RCC->AHB1ENR |= (RCC_AHB1ENR_CRCEN | RCC_AHB1ENR_DMA1EN);
  RCC->AHB2ENR |= (RCC_AHB2ENR_ADCEN | RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_GPIODEN);
  RCC->BDCR |= RCC_BDCR_RTCEN;

  /* Set Interrupt Group Priority */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  // set systick frequency
  HAL_SYSTICK_Config(SystemCoreClock / (1000U / (uint32_t)10));

  // set systick interrupt priority
  HAL_NVIC_SetPriority(SysTick_IRQn, 5, 0U);
#endif

  rte_main_reboot_req = 0;

  // initializing variables & arrays in rte_wx
  rte_wx_init();
  rte_rtu_init();

  // calculate CRC over configuration blocks
  main_crc_result = configuration_handler_check_crc();

  // if first section has wrong CRC and it hasn't been restored before
  if ((main_crc_result & 0x01) == 0 && (configuration_get_register() & CONFIG_FIRST_FAIL_RESTORING) == 0) {
	  // restore default configuration
	  if (configuration_handler_restore_default_first() == 0) {

		  // if configuration has been restored successfully
		  configuration_set_bits_register(CONFIG_FIRST_RESTORED);

		  // set also CRC flag because if restoring is successfull the region has good CRC
		  configuration_set_bits_register(CONFIG_FIRST_CRC_OK);

	  }
	  else {
		  // if not store the flag in the backup register to block
		  // reinitializing once again in the consecutive restart
		  configuration_set_bits_register(CONFIG_FIRST_FAIL_RESTORING);

		  configuration_clear_bits_register(CONFIG_FIRST_CRC_OK);
	  }


  }
  else {
	  // if the combined confition is not met check failed restoring flag
	  if ((configuration_get_register() & CONFIG_FIRST_FAIL_RESTORING) == 0) {
		  // a CRC checksum is ok, so first configuration section can be used further
		  configuration_set_bits_register(CONFIG_FIRST_CRC_OK);
	  }
	  else {
		  ;
	  }
  }

  // if second section has wrong CRC and it hasn't been restored before
  if ((main_crc_result & 0x02) == 0 && (configuration_get_register() & CONFIG_SECOND_FAIL_RESTORING) == 0) {
	  // restore default configuration
	  if (configuration_handler_restore_default_second() == 0) {

		  // if configuration has been restored successfully
		  configuration_set_bits_register(CONFIG_SECOND_RESTORED);

		  // set also CRC flag as if restoring is successfull the region has good CRC
		  configuration_set_bits_register(CONFIG_SECOND_CRC_OK);

	  }
	  else {
		  // if not store the flag in the backup register
		  configuration_set_bits_register(CONFIG_SECOND_FAIL_RESTORING);

		  configuration_clear_bits_register(CONFIG_SECOND_CRC_OK);
	  }


  }
  else {
	  // check failed restoring flag
	  if ((configuration_get_register() & CONFIG_SECOND_FAIL_RESTORING) == 0) {
		  // second configuration section has good CRC and can be used further
		  configuration_set_bits_register(CONFIG_SECOND_CRC_OK);
	  }
	  else {
		  ;
	  }
  }

  // at this point both sections have either verified CRC or restored values to default
  if ((configuration_get_register() & CONFIG_FIRST_CRC_OK) != 0 && (configuration_get_register() & CONFIG_SECOND_CRC_OK) != 0) {
	  // if both sections are OK check programming counters
	  if (config_data_pgm_cntr_first > config_data_pgm_cntr_second) {
		  // if first section has bigger programing counter use it
		  configuration_handler_load_configuration(REGION_FIRST);
	  }
	  else {
		  configuration_handler_load_configuration(REGION_SECOND);

	  }
  }
  else if ((configuration_get_register() & CONFIG_FIRST_CRC_OK) != 0 && (configuration_get_register() & CONFIG_SECOND_CRC_OK) == 0) {
	  // if only first region is OK use it
	  configuration_handler_load_configuration(REGION_FIRST);
  }
  else if ((configuration_get_register() & CONFIG_FIRST_CRC_OK) == 0 && (configuration_get_register() & CONFIG_SECOND_CRC_OK) != 0) {
	  // if only first region is OK use it
	  configuration_handler_load_configuration(REGION_FIRST);
  }
  else {
	  configuration_handler_load_configuration(REGION_DEFAULT);
  }

  // set packets intervals
  packet_tx_configure(main_config_data_basic->wx_transmit_period, main_config_data_basic->beacon_transmit_period, main_config_data_mode->powersave);

#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
  // disabling access to BKP registers
  RCC->APB1ENR &= (0xFFFFFFFF ^ (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN));
  PWR->CR &= (0xFFFFFFFF ^ PWR_CR_DBP);
#endif

  // converting latitude into string
  memset(main_string_latitude, 0x00, sizeof(main_string_latitude));
  float_to_string(main_config_data_basic->latitude, main_string_latitude, sizeof(main_string_latitude), 2, 2);

  // converting longitude into string
  memset(main_string_longitude, 0x00, sizeof(main_string_longitude));
  float_to_string(main_config_data_basic->longitude, main_string_longitude, sizeof(main_string_longitude), 2, 5);

  switch(main_config_data_basic->symbol) {
  case 0:		// _SYMBOL_DIGI
	  main_symbol_f = '/';
	  main_symbol_s = '#';
	  break;
  case 1:		// _SYMBOL_WIDE1_DIGI
	  main_symbol_f = '1';
	  main_symbol_s = '#';
	  break;
  case 2:		// _SYMBOL_HOUSE
	  main_symbol_f = '/';
	  main_symbol_s = '-';
	  break;
  case 3:		// _SYMBOL_RXIGATE
	  main_symbol_f = 'I';
	  main_symbol_s = '&';
	  break;
  case 5:		// _SYMBOL_SAILBOAT
	  main_symbol_f = '/';
	  main_symbol_s = 'Y';
	  break;
  default:		// _SYMBOL_IGATE
	  main_symbol_f = 'R';
	  main_symbol_s = '&';
	  break;

  }

#if defined _RANDOM_DELAY
  // configuring a default delay value
  delay_set(_DELAY_BASE, 1);
#elif !defined _RANDOM_DELAY
  delay_set(_DELAY_BASE, 0);

#endif

#if defined(PARAMETEO)
  // initialize all powersaving functions
  pwr_save_init(main_config_data_mode->powersave);
#endif

  // initalizing separated Open Collector output
  io_oc_init();

  // initialize sensor power control and switch off supply voltage
  wx_pwr_switch_init();

  // call periodic handle to wait for 1 second and then switch on voltage
  wx_pwr_switch_periodic_handle();

  // waiting for 1 second to count number of ticks when the CPU is idle
  main_idle_cpu_ticks = delay_fixed_with_count(1000);

#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)

  // Configure I/O pins for USART1 (Kiss modem)
  Configure_GPIO(GPIOA,10,PUD_INPUT);		// RX
  Configure_GPIO(GPIOA,9,AFPP_OUTPUT_2MHZ);	// TX

  // Configure I/O pins for USART2 (wx meteo comm)
  Configure_GPIO(GPIOA,3,PUD_INPUT);		// RX
  Configure_GPIO(GPIOA,2,AFPP_OUTPUT_2MHZ);	// TX

#endif

#if defined(PARAMETEO)
  	// USART1 - KISS
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_10;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// RX

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_9;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// TX

	// USART2 - METEO
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_3;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// RX

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_2;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// TX

	// USART3 - GSM
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_10;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// TX

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_11;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// RX

#endif

#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B)
  Configure_GPIO(GPIOA,7,GPPP_OUTPUT_2MHZ);	// re/te
  GPIO_ResetBits(GPIOA, GPIO_Pin_7);
#endif
#if defined(PARATNC_HWREV_C)
  Configure_GPIO(GPIOA,8,GPPP_OUTPUT_2MHZ);	// re/te
  GPIO_ResetBits(GPIOA, GPIO_Pin_8);
#endif

#if defined(PARAMETEO)
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_2;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// RE-TE
#endif

#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
  // enabling the clock for both USARTs
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
#endif

  main_kiss_srl_ctx_ptr = &main_kiss_srl_ctx;
  main_wx_srl_ctx_ptr = &main_wx_srl_ctx;
#if defined(PARAMETEO)
  main_gsm_srl_ctx_ptr = &main_gsm_srl_ctx;
#endif

  main_target_kiss_baudrate = 9600u;
  main_target_wx_baudrate = _SERIAL_BAUDRATE;

#if defined(PARAMETEO)

#endif

#if defined(PARAMETEO)
  // swtich power to M4. turn on sensors but keep GSM modem turned off
  pwr_save_switch_mode_to_m4();
#endif

  // if Victron VE-direct protocol is enabled set the baudrate to the 19200u
  if (main_config_data_mode->victron == 1) {
    main_target_kiss_baudrate = 19200u;

    // and disable the kiss TNC option as it shares the same port
    main_kiss_enabled = 0;
  }


  if (main_config_data_mode->wx_davis == 1) {
	  // reinitialize the KISS serial port temporary to davis baudrate
	  main_target_kiss_baudrate = DAVIS_DEFAULT_BAUDRATE;

	  // reset RX state to allow reinitialization with changed baudrate
	  main_kiss_srl_ctx_ptr->srl_rx_state = SRL_RX_NOT_CONFIG;

	  // reinitializing serial hardware to wake up Davis wx station
	  srl_init(main_kiss_srl_ctx_ptr, USART1, srl_usart1_rx_buffer, RX_BUFFER_1_LN, srl_usart1_tx_buffer, TX_BUFFER_1_LN, main_target_kiss_baudrate, 1);

	  srl_switch_timeout(main_kiss_srl_ctx_ptr, SRL_TIMEOUT_ENABLE, 3000);

	  davis_init(main_kiss_srl_ctx_ptr);

	  // try to wake up the davis base
	  rte_wx_davis_station_avaliable = (davis_wake_up(DAVIS_BLOCKING_IO) == 0 ? 1 : 0);

	  // if davis weather stations is connected to SERIAL port
	  if (rte_wx_davis_station_avaliable == 1) {
		  // turn LCD backlight on..
		  davis_control_backlight(1);

		  // wait for a while
		  delay_fixed(1000);

		  // and then off to let the user know that communication is working
		  davis_control_backlight(0);

		  // disable the KISS modem as the UART will be used for DAVIS wx station
		  main_kiss_enabled = 0;

		  // enable the davis serial protocol client to allow pooling callbacks to be called in main loop.
		  // This only controls the callback it doesn't mean that the station itself is responding to
		  // communication. It stays set to one event if Davis station
		  main_davis_serial_enabled = 1;

		  // trigger the rxcheck to get all counter values
		  davis_trigger_rxcheck_packet();

	  }
	  else {
		  // if not revert back to KISS configuration
		  main_target_kiss_baudrate = 9600u;
		  main_kiss_srl_ctx_ptr->srl_rx_state = SRL_RX_NOT_CONFIG;

		  // initializing UART drvier
		  srl_init(main_kiss_srl_ctx_ptr, USART1, srl_usart1_rx_buffer, RX_BUFFER_1_LN, srl_usart1_tx_buffer, TX_BUFFER_1_LN, main_target_kiss_baudrate, 1);
		  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer, RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN, main_target_wx_baudrate, 1);

	  }
  }
  else if (main_config_data_mode->wx_modbus == 1) {

	  rtu_serial_init(&rte_rtu_pool_queue, 1, main_wx_srl_ctx_ptr, main_config_data_rtu);

	  main_target_wx_baudrate = main_config_data_rtu->slave_speed;

	  // initialize serial ports according to RS485 network configuration for Modbus-RTU
	  srl_init(main_kiss_srl_ctx_ptr, USART1, srl_usart1_rx_buffer, RX_BUFFER_1_LN, srl_usart1_tx_buffer, TX_BUFFER_1_LN, main_target_kiss_baudrate, 1);
	  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer, RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN, main_target_wx_baudrate, main_config_data_rtu->slave_stop_bits);
	  srl_switch_tx_delay(main_wx_srl_ctx_ptr, 1);

	  // enabling rtu master code
	  main_modbus_rtu_master_enabled = 1;

	  rtu_serial_start();
  }
  else {
	  // initializing UART drvier
	  srl_init(main_kiss_srl_ctx_ptr, USART1, srl_usart1_rx_buffer, RX_BUFFER_1_LN, srl_usart1_tx_buffer, TX_BUFFER_1_LN, main_target_kiss_baudrate, 1);
	  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer, RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN, main_target_wx_baudrate, 1);
  }

#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B)
  main_wx_srl_ctx_ptr->te_pin = GPIO_Pin_7;
  main_wx_srl_ctx_ptr->te_port = GPIOA;
#endif
#if defined(PARATNC_HWREV_C)
  main_wx_srl_ctx_ptr->te_pin = GPIO_Pin_8;
  main_wx_srl_ctx_ptr->te_port = GPIOA;
#endif
#if defined(PARAMETEO)
  main_wx_srl_ctx_ptr->te_pin = LL_GPIO_PIN_8;
  main_wx_srl_ctx_ptr->te_port = GPIOA;

  srl_init(main_gsm_srl_ctx_ptr, USART3, srl_usart3_rx_buffer, RX_BUFFER_1_LN, srl_usart3_tx_buffer, TX_BUFFER_1_LN, 115200, 1);
#endif

  // initialize APRS path with zeros
  memset (main_own_path, 0x00, sizeof(main_own_path));

  // configuring an APRS path used to transmit own packets (telemetry, wx, beacons)
  main_own_path_ln = ConfigPath(main_own_path, main_config_data_basic);

#ifdef INTERNAL_WATCHDOG
#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
  // enable write access to watchdog registers
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  // Set watchdog prescaler
  IWDG_SetPrescaler(IWDG_Prescaler_128);

  // Set the counter value to program watchdog for about 13 seconds
  IWDG_SetReload(0xFFF);

  // enable the watchdog
  IWDG_Enable();

  // do not disable the watchdog when the core is halted on a breakpoint
  DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);

  // reload watchdog counter
  IWDG_ReloadCounter();
#endif
#endif

#ifdef _METEO
  // initialize i2c controller
  i2cConfigure();
#endif

  // initialize GPIO pins leds are connecting to
  led_init();

  // initialize AX25 & APRS stuff
  AFSK_Init(&main_afsk);
  ax25_init(&main_ax25, &main_afsk, 0, 0x00);
  DA_Init();

  // configure external watchdog
  io_ext_watchdog_config();

  // initializing the digipeater configuration
  digi_init(main_config_data_mode);

  if ((main_config_data_mode->wx & WX_ENABLED) == 1) {
#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
	  dallas_init(GPIOC, GPIO_Pin_11, GPIO_PinSource11, &rte_wx_dallas_average);
#endif

#if defined(PARAMETEO)

	  // switch on voltages exclusively for ParaMETEO

	  // initialize dallas one-wire driver for termometer
	  dallas_init(GPIOC, LL_GPIO_PIN_11, 0x0, &rte_wx_dallas_average);
#endif

	  if (main_config_data_mode->wx_umb == 1) {
		  // client initialization
		  umb_master_init(&rte_wx_umb_context, main_wx_srl_ctx_ptr, main_config_data_umb);
	  }

	  if ((main_config_data_mode->wx & WX_INTERNAL_SPARKFUN_WIND) == 0) {
		  analog_anemometer_init(main_config_data_mode->wx_anemometer_pulses_constant, 38, 100, 1);
	  }
	  else {
		  analog_anemometer_init(main_config_data_mode->wx_anemometer_pulses_constant, 38, 100, 1);
	  }
  }

  // configuring interrupt priorities
  it_handlers_set_priorities();

	if (main_config_data_mode->wx_ms5611_or_bme == 0) {
	 ms5611_reset(&rte_wx_ms5611_qf);
	 ms5611_read_calibration(SensorCalData, &rte_wx_ms5611_qf);
	 ms5611_trigger_measure(0, 0);
	}
	else if (main_config_data_mode->wx_ms5611_or_bme == 1) {
	 bme280_reset(&rte_wx_bme280_qf);
	 bme280_setup();
	 bme280_read_calibration(bme280_calibration_data);
	}

 if (main_kiss_enabled == 1) {
	  // preparing initial beacon which will be sent to host PC using KISS protocol via UART
	  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%s%c%c%s%c%c %s", main_string_latitude, main_config_data_basic->n_or_s, main_symbol_f, main_string_longitude, main_config_data_basic->e_or_w, main_symbol_s, main_config_data_basic->comment);

	  // terminating the aprs message
	  main_own_aprs_msg[main_own_aprs_msg_len] = 0;

	  // 'sending' the message which will only encapsulate it inside AX25 protocol (ax25_starttx is not called here)
	  //ax25_sendVia(&main_ax25, main_own_path, (sizeof(main_own_path) / sizeof(*(main_own_path))), main_own_aprs_msg, main_own_aprs_msg_len);
	  ln = ax25_sendVia_toBuffer(main_own_path, (sizeof(main_own_path) / sizeof(*(main_own_path))), main_own_aprs_msg, main_own_aprs_msg_len, main_kiss_srl_ctx.srl_tx_buf_pointer, TX_BUFFER_1_LN);

	  // SendKISSToHost function cleares the output buffer hence routine need to wait till the UART will be ready for next transmission.
	  // Here this could be omitted because UART isn't used before but general idea
	  while(main_kiss_srl_ctx.srl_tx_state != SRL_TX_IDLE && main_kiss_srl_ctx.srl_tx_state != SRL_TX_ERROR);

	  // converting AX25 with beacon to KISS format
	  //ln = SendKISSToHost(main_afsk.tx_buf + 1, main_afsk.tx_fifo.tail - main_afsk.tx_fifo.head - 4, srl_tx_buffer, TX_BUFFER_LN);


	  // checking if KISS-framing was done correctly
	  if (ln != KISS_TOO_LONG_FRM) {
	#ifdef SERIAL_TX_TEST_MODE
		  // infinite loop for testing UART transmission
		  for (;;) {

			  retval = srl_receive_data(main_kiss_srl_ctx_ptr, 100, FEND, FEND, 0, 0, 0);
	#endif
			  retval = srl_start_tx(main_kiss_srl_ctx_ptr, ln);

	#ifdef SERIAL_TX_TEST_MODE
			  	  	while(main_kiss_srl_ctx_ptr->srl_tx_state != SRL_TX_IDLE);

				#if defined(PARAMETEO)
			  	 	LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_9);
				#else
		  	  		GPIOC->ODR = (GPIOC->ODR ^ GPIO_Pin_9);
				#endif

			  if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE) {
				#if defined(PARAMETEO)
			  	 		LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_9);
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
  AFSK_Init(&main_afsk);

  ADCStartConfig();
  DACStartConfig();
  AFSK_Init(&main_afsk);
  ax25_init(&main_ax25, &main_afsk, 0, message_callback);

	if ((main_config_data_mode->wx & WX_ENABLED) == 1) {
	  // getting all meteo measuremenets to be sure that WX frames want be sent with zeros
	  wx_get_all_measurements(main_config_data_wx_sources, main_config_data_mode, main_config_data_umb, main_config_data_rtu);
	}

  // start serial port i/o transaction depending on station configuration
  if (main_config_data_mode->victron == 1) {
	  // initializing protocol parser
	  ve_direct_parser_init(&rte_pv_struct, &rte_pv_average);

	  // enabling timeout handling for serial port. This is required because VE protocol frame may vary in lenght
	  // and serial port driver could finish reception only either on stop character or when declared number of bytes
	  // has been received.
	  srl_switch_timeout(main_kiss_srl_ctx_ptr, 1, 50);

	  // switching UART to receive mode to be ready for data from charging controller
	  srl_receive_data(main_kiss_srl_ctx_ptr, VE_DIRECT_MAX_FRAME_LN, 0, 0, 0, 0, 0);
  }
  else {
	  // switching UART to receive mode to be ready for KISS frames from host
	  srl_receive_data(main_kiss_srl_ctx_ptr, 100, FEND, FEND, 0, 0, 0);
  }

  io_oc_output_low();

  led_control_led1_upper(false);
  led_control_led2_bottom(false);

#if defined(PARAMETEO)
   pwr_save_switch_mode_to_c0();

   // sleep a little bit and wait for everything to power up completely
   delay_fixed(1000);

   led_control_led1_upper(true);
   led_control_led2_bottom(false);

   delay_fixed(1000);

   led_control_led1_upper(false);
   led_control_led2_bottom(true);

   delay_fixed(1000);

   led_control_led1_upper(true);
   led_control_led2_bottom(true);

   delay_fixed(1000);

   led_control_led1_upper(false);
   led_control_led2_bottom(false);

#endif

  // configuting system timers
  TimerConfig();

  // initialize UMB transaction
  if (main_config_data_mode->wx_umb == 1) {
	umb_0x26_status_request(&rte_wx_umb, &rte_wx_umb_context, main_config_data_umb);
  }

#ifdef INTERNAL_WATCHDOG
   // reload watchdog counter
   IWDG_ReloadCounter();
#endif

   io_ext_watchdog_service();

   if (main_config_data_mode->gsm == 1) {
	   gsm_sim800_init(&main_gsm_state, 1);
   }

   if (main_config_data_basic-> beacon_at_bootup == 1) {
	   beacon_send_own();
   }

  // Infinite loop
  while (1)
    {
	  // incrementing current cpu ticks
	  main_current_cpu_idle_ticks++;

	    if (rte_main_reboot_req == 1) {
	    	NVIC_SystemReset();
	    }
	    else {
	    	;
	    }

#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
	    // read the state of a button input
	  	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {

	  		// if modem is not busy on transmitting something and the button is not
	  		// inhibited
	  		if (main_afsk.sending == false && button_inhibit == 0) {

	  			// wait for radio channel to be released
	  			while(main_ax25.dcd == true);

	  			if ((main_config_data_mode->wx & WX_ENABLED) == 0) {

	  				beacon_send_own();
	  			}
	  			else {

					srl_wait_for_tx_completion(main_kiss_srl_ctx_ptr);

					SendWXFrameToBuffer(rte_wx_average_windspeed, rte_wx_max_windspeed, rte_wx_average_winddirection, rte_wx_temperature_average_external_valid, rte_wx_pressure_valid, rte_wx_humidity, main_kiss_srl_ctx.srl_tx_buf_pointer, TX_BUFFER_1_LN, &ln);

					if (main_kiss_enabled == 1) {
						srl_start_tx(main_kiss_srl_ctx_ptr, ln);
					}
	  			}
	  		}

	  		button_inhibit = 1;
	  	}
	  	else {
	  		button_inhibit = 0;
	  	}
#endif


	  	// if new packet has been received from radio channel
		if(ax25_new_msg_rx_flag == 1) {
			memset(main_kiss_srl_ctx.srl_tx_buf_pointer, 0x00, main_kiss_srl_ctx.srl_tx_buf_ln);

			if (main_kiss_enabled == 1) {
				// convert message to kiss format and send it to host
				srl_start_tx(main_kiss_srl_ctx_ptr, SendKISSToHost(ax25_rxed_frame.raw_data, (ax25_rxed_frame.raw_msg_len - 2), main_kiss_srl_ctx.srl_tx_buf_pointer, main_kiss_srl_ctx.srl_tx_buf_ln));
			}

			main_ax25.dcd = false;

			digi_check_with_viscous(&ax25_rxed_frame);

			// check if this packet needs to be repeated (digipeated) and do it if it is necessary
			digi_process(&ax25_rxed_frame, main_config_data_basic, main_config_data_mode);

			ax25_new_msg_rx_flag = 0;
			rx10m++;
		}

		// if GSM communication is enabled
		if (main_config_data_mode->gsm == 1) {

			// if data has been received
			if (main_gsm_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE || main_gsm_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR) {

				// receive callback for communicatio with the modem
				//gsm_sim800_rx_done_callback(main_gsm_srl_ctx_ptr, &main_gsm_state);
			}
		}

		// if Victron VE.direct client is enabled
		if (main_config_data_mode->victron == 1) {

			// if new KISS message has been received from the host
			if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE || main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR) {

				// cutting received string to Checksum, everything after will be skipped
				ve_direct_cut_to_checksum(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), TX_BUFFER_1_LN, &buffer_len);

				// checking if this frame is ok
				ve_direct_validate_checksum(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), buffer_len, &retval);

				if (retval == 1) {
					// parsing data from input serial buffer to
					retval = ve_direct_parse_to_raw_struct(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), buffer_len, &rte_pv_struct);

					if (retval == 0) {
						ve_direct_add_to_average(&rte_pv_struct, &rte_pv_average);

						ve_direct_get_averages(&rte_pv_average, &rte_pv_battery_current, &rte_pv_battery_voltage, &rte_pv_cell_voltage, &rte_pv_load_current);

						ve_direct_set_sys_voltage(&rte_pv_struct, &rte_pv_sys_voltage);

						ve_direct_store_errors(&rte_pv_struct, &rte_pv_last_error);

						rte_pv_messages_count++;
					}
				}
				else {
					rte_pv_corrupted_messages_count++;
				}

				//memset(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), 0x00, TX_BUFFER_1_LN);

				srl_receive_data(main_kiss_srl_ctx_ptr, VE_DIRECT_MAX_FRAME_LN, 0, 0, 0, 0, 0);
			}
		}
		else if (main_config_data_mode->wx_umb == 1) {
			// if some UMB data have been received
			if (main_wx_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE) {
				umb_pooling_handler(&rte_wx_umb_context, REASON_RECEIVE_IDLE, master_time, main_config_data_umb);
			}

			// if there were an error during receiving frame from host, restart rxing once again
			if (main_wx_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR) {
				umb_pooling_handler(&rte_wx_umb_context, REASON_RECEIVE_ERROR, master_time, main_config_data_umb);
			}

			if (main_wx_srl_ctx_ptr->srl_tx_state == SRL_TX_IDLE) {
				umb_pooling_handler(&rte_wx_umb_context, REASON_TRANSMIT_IDLE, master_time, main_config_data_umb);
			}
		}
		else {
			// if new KISS message has been received from the host
			if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE && main_kiss_enabled == 1) {
				// parse i ncoming data and then transmit on radio freq
				ln = kiss_parse_received(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), srl_get_num_bytes_rxed(main_kiss_srl_ctx_ptr), &main_ax25, &main_afsk);
				if (ln == 0)
					kiss10m++;	// increase kiss messages counter

				// restart KISS receiving to be ready for next frame
				srl_receive_data(main_kiss_srl_ctx_ptr, 120, FEND, FEND, 0, 0, 0);
			}

			// if there were an error during receiving frame from host, restart rxing once again
			if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR && main_kiss_enabled == 1) {
				srl_receive_data(main_kiss_srl_ctx_ptr, 120, FEND, FEND, 0, 0, 0);
			}
		}

		// if Davis wx station is enabled and it is alive
		if (main_davis_serial_enabled == 1) {

			// pool the Davis wx station driver for LOOP packet
			davis_loop_packet_pooler(&rte_wx_davis_loop_packet_avaliable);

			davis_rxcheck_packet_pooler();
		}

		// if modbus rtu master is enabled
		if (main_modbus_rtu_master_enabled == 1) {
			rtu_serial_pool();
		}

		// get all meteo measuremenets each 65 seconds. some values may not be
		// downloaded from sensors if _METEO and/or _DALLAS_AS_TELEM aren't defined
		if (main_wx_sensors_pool_timer < 10) {

			if (main_modbus_rtu_master_enabled == 1) {
				rtu_serial_start();
			}

			if ((main_config_data_mode->wx & WX_ENABLED) == 1) {
				wx_get_all_measurements(main_config_data_wx_sources, main_config_data_mode, main_config_data_umb, main_config_data_rtu);
			}


			if (main_config_data_mode->wx_umb == 1) {
				//
				umb_0x26_status_request(&rte_wx_umb, &rte_wx_umb_context, main_config_data_umb);
			}

			if (main_davis_serial_enabled == 1) {
				davis_trigger_rxcheck_packet();
			}

			if (rte_main_trigger_modbus_status == 1 && main_modbus_rtu_master_enabled == 1) {
				rtu_serial_get_status_string(&rte_rtu_pool_queue, main_wx_srl_ctx_ptr, main_own_aprs_msg, OWN_APRS_MSG_LN, &main_own_aprs_msg_len);

			 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);

			 	afsk_txStart(&main_afsk);

			 	rte_main_trigger_modbus_status = 0;


			}

			main_wx_sensors_pool_timer = 65500;
		}

		if (main_one_minute_pool_timer < 10) {

			#ifndef _MUTE_OWN
			packet_tx_handler(main_config_data_basic, main_config_data_mode);
			#endif

			main_one_minute_pool_timer = 60000;
		}

		if (main_one_second_pool_timer < 10) {

			//digi_pool_viscous();

			digi_pool_viscous();

			#ifdef PARAMETEO
			gsm_sim800_pool(main_gsm_srl_ctx_ptr, &main_gsm_state);
			#endif

			if ((main_config_data_mode->wx & WX_ENABLED) == 1) {
				analog_anemometer_direction_handler();
			}

			main_one_second_pool_timer = 1000;
		}
		else if (main_one_second_pool_timer < -10) {

			if ((main_config_data_mode->wx & WX_ENABLED) == 1) {
				analog_anemometer_direction_reset();
			}

			main_one_second_pool_timer = 1000;
		}

		if (main_two_second_pool_timer < 10) {

			wx_check_force_i2c_reset();

			wx_pwr_switch_periodic_handle();

			#ifdef INTERNAL_WATCHDOG
			IWDG_ReloadCounter();
			#endif

			main_two_second_pool_timer = 2000;
		}

		if (main_ten_second_pool_timer < 10) {

			if (rte_main_trigger_wx_packet == 1) {

				packet_tx_send_wx_frame();

				rte_main_trigger_wx_packet = 0;
			}

			#ifdef STM32L471xx
			// inhibit any power save switching when modem transmits data
			if (!main_afsk.sending) {
				pwr_save_pooling_handler(main_config_data_mode, main_config_data_basic, packet_tx_get_minutes_to_next_wx());
			}
			#endif

			if (main_config_data_mode->wx_umb == 1) {
				umb_channel_pool(&rte_wx_umb, &rte_wx_umb_context, main_config_data_umb);
			}

			if (main_config_data_mode->wx_umb == 1) {
				rte_wx_umb_qf = umb_get_current_qf(&rte_wx_umb_context, master_time);
			}

			wx_pool_anemometer(main_config_data_wx_sources, main_config_data_mode, main_config_data_umb, main_config_data_rtu);

			if (main_davis_serial_enabled == 1) {

				// if previous LOOP packet is ready for processing
				if (rte_wx_davis_loop_packet_avaliable == 1) {
					davis_parsers_loop(main_kiss_srl_ctx_ptr->srl_rx_buf_pointer, main_kiss_srl_ctx_ptr->srl_rx_buf_ln, &rte_wx_davis_loop_content);
				}

				// trigger consecutive LOOP packet
				davis_trigger_loop_packet();
			}

			main_ten_second_pool_timer = 10000;
		}


    }
  // Infinite loop, never return.
}

uint16_t main_get_adc_sample(void) {
	return (uint16_t) ADC1->DR;
}

void main_service_cpu_load_ticks(void) {

	uint32_t cpu_ticks_load = 0;

	// the biggest this result will be the biggest load the CPU is handling
	cpu_ticks_load = main_idle_cpu_ticks - main_current_cpu_idle_ticks;

	// calculate the cpu load
	main_cpu_load = (int8_t) ((cpu_ticks_load * 100) / main_idle_cpu_ticks);

	// reset the tick counter back to zero;
	main_current_cpu_idle_ticks = 0;
}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
