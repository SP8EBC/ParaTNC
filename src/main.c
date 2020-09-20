#include <delay.h>
#include <LedConfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_iwdg.h>
#include <stm32f10x.h>

#include "main.h"
#include "packet_tx_handler.h"
#include "station_config.h"

#include "diag/Trace.h"
#include "antilib_adc.h"
#include "afsk_pr.h"
#include "TimerConfig.h"
#include "PathConfig.h"
#include "LedConfig.h"
#include "io.h"

#include "it_handlers.h"

#include "aprs/digi.h"
#include "aprs/telemetry.h"
#include "aprs/dac.h"
#include "aprs/beacon.h"

#ifdef _VICTRON
#include "ve_direct_protocol/parser.h"
#endif

#include "rte_wx.h"
#include "rte_pv.h"
#include "rte_main.h"

#ifdef _METEO
#include <wx_handler.h>
#include "drivers/dallas.h"
#include "drivers/i2c.h"
#include "drivers/tx20.h"
#include "drivers/analog_anemometer.h"
#include "aprs/wx.h"
#include "drivers/gpio_conf.h"

#include "../system/include/modbus_rtu/rtu_serial_io.h"

#include "../system/include/davis_vantage/davis.h"
#include "../system/include/davis_vantage/davis_parsers.h"

#ifdef _SENSOR_MS5611
#include "drivers/ms5611.h"
#endif

#ifdef _SENSOR_BME280
#include <drivers/bme280.h>
#endif

#ifdef _UMB_MASTER
#include "umb_master/umb_master.h"
#include "umb_master/umb_channel_pool.h"
#include "umb_master/umb_0x26_status.h"
#endif

#endif	// _METEO

#ifdef _DALLAS_AS_TELEM
#include "drivers/dallas.h"
#endif

#include "KissCommunication.h"

#define SOH 0x01

//#define SERIAL_TX_TEST_MODE

// Niebieska dioda -> DCD
// Zielona dioda -> anemometr albo TX

// backup registers
// 2 -> 4bit hard-faults | 4bit boot-counter
// 3 -> hard fault PC LSB
// 4 -> hard fault PC MSB
// 5 -> hard fault LR LSB
// 6 -> hard fault LR MSB

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wempty-body"


// global variable incremented by the SysTick handler to measure time in miliseconds
uint32_t master_time = 0;

// global variable used as a timer to trigger meteo sensors mesurements
int32_t main_wx_sensors_pool_timer = 65500;

// global variable used as a timer to trigger packet sending
int32_t main_packet_tx_pool_timer = 60000;

// one second pool interval
int32_t main_one_second_pool_timer = 1000;

// two second pool interval
int32_t main_two_second_pool_timer = 2000;

// ten second pool interval
int32_t main_ten_second_pool_timer = 10000;

// serial context for UART used to KISS
srl_context_t main_kiss_srl_ctx;

#if defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
// serial context for UART used for comm with wx sensors
srl_context_t main_wx_srl_ctx;
#endif

// a pointer to KISS context
srl_context_t* main_kiss_srl_ctx_ptr;

// a pointer to wx comms context
srl_context_t* main_wx_srl_ctx_ptr;

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
char main_own_aprs_msg[160];

// global variable used to store return value from various functions
volatile uint8_t retval = 100;

uint16_t buffer_len = 0;
#ifdef _VICTRON
#endif

#ifdef _UMB_MASTER
// return value from UMB related functions
umb_retval_t main_umb_retval = UMB_UNINITIALIZED;
#endif

#ifdef _MODBUS_RTU
rtu_pool_queue_t main_rtu_pool_queue;
#endif

char after_tx_lock;

unsigned short rx10m = 0, tx10m = 0, digi10m = 0, digidrop10m = 0, kiss10m = 0;


static void message_callback(struct AX25Msg *msg) {

}

int main(int argc, char* argv[]){

  int32_t ln = 0;

  uint8_t button_inhibit = 0;

  RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM7EN | RCC_APB1ENR_TIM4EN);
  RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_TIM1EN);

  memset(main_own_aprs_msg, 0x00, 128);

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

  rte_main_reboot_req = 0;

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

  rte_main_hardfault_pc = (BKP->DR3 | (BKP->DR4 << 16));
  rte_main_hardfault_lr = (BKP->DR5 | (BKP->DR6 << 16));

  BKP->DR3 = 0;
  BKP->DR4 = 0;
  BKP->DR5 = 0;
  BKP->DR6 = 0;

  // disabling access to BKP registers
  RCC->APB1ENR &= (0xFFFFFFFF ^ (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN));
  PWR->CR &= (0xFFFFFFFF ^ PWR_CR_DBP);

#if defined _RANDOM_DELAY
  // configuring a default delay value
  delay_set(_DELAY_BASE, 1);
#elif !defined _RANDOM_DELAY
  delay_set(_DELAY_BASE, 0);

#endif

  // initialize sensor power control and switch off supply voltage
  wx_pwr_init();

  // call periodic handle to wait for 1 second and then switch on voltage
  wx_pwr_periodic_handle();

  // Configure I/O pins for USART1 (Kiss modem)
  Configure_GPIO(GPIOA,10,PUD_INPUT);		// RX
  Configure_GPIO(GPIOA,9,AFPP_OUTPUT_2MHZ);	// TX
#if defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
  // Configure I/O pins for USART2 (wx meteo comm)
  Configure_GPIO(GPIOA,3,PUD_INPUT);		// RX
  Configure_GPIO(GPIOA,2,AFPP_OUTPUT_2MHZ);	// TX
#endif

#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B)
  Configure_GPIO(GPIOA,7,GPPP_OUTPUT_2MHZ);	// re/te
  GPIO_ResetBits(GPIOA, GPIO_Pin_7);
#endif
#if defined(PARATNC_HWREV_C)
  Configure_GPIO(GPIOA,8,GPPP_OUTPUT_2MHZ);	// re/te
  GPIO_ResetBits(GPIOA, GPIO_Pin_8);
#endif

  // enabling the clock for both USARTs
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

#if defined(PARATNC_HWREV_A)
  main_kiss_srl_ctx_ptr = &main_kiss_srl_ctx;
  main_wx_srl_ctx_ptr = &main_kiss_srl_ctx;

  main_target_kiss_baudrate = 9600u;
#if defined(_UMB_MASTER)
  main_target_kiss_baudrate = _SERIAL_BAUDRATE;
#endif
#endif
#if defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)
  main_kiss_srl_ctx_ptr = &main_kiss_srl_ctx;
  main_wx_srl_ctx_ptr = &main_wx_srl_ctx;

  main_target_kiss_baudrate = 9600u;
  main_target_wx_baudrate = _SERIAL_BAUDRATE;
#endif
#if !defined(PARATNC_HWREV_A) && !defined(PARATNC_HWREV_B) && !defined(PARATNC_HWREV_C)
  main_kiss_srl_ctx_ptr = &main_kiss_srl_ctx;
  main_wx_srl_ctx_ptr = &main_kiss_srl_ctx;

  main_target_kiss_baudrate = _SERIAL_BAUDRATE;
#endif

#if (defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)) && defined(_DAVIS_SERIAL)
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

#elif (defined(PARATNC_HWREV_B) || defined(PARATNC_HWREV_C)) && defined(_MODBUS_RTU)

  rtu_serial_init(&main_rtu_pool_queue);

  main_target_wx_baudrate = _RTU_SLAVE_SPEED;

  srl_init(main_kiss_srl_ctx_ptr, USART1, srl_usart1_rx_buffer, RX_BUFFER_1_LN, srl_usart1_tx_buffer, TX_BUFFER_1_LN, main_target_kiss_baudrate, 1);
  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer, RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN, main_target_wx_baudrate, _RTU_SLAVE_STOP_BITS);

  main_modbus_rtu_master_enabled = 1;

#else
  // initializing UART drvier
  srl_init(main_kiss_srl_ctx_ptr, USART1, srl_usart1_rx_buffer, RX_BUFFER_1_LN, srl_usart1_tx_buffer, TX_BUFFER_1_LN, main_target_kiss_baudrate, 1);
  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer, RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN, main_target_wx_baudrate, 1);


#endif


#if defined(PARATNC_HWREV_A) || defined(PARATNC_HWREV_B)
  main_wx_srl_ctx_ptr->te_pin = GPIO_Pin_7;
  main_wx_srl_ctx_ptr->te_port = GPIOA;
#endif
#if defined(PARATNC_HWREV_C)
  main_wx_srl_ctx_ptr->te_pin = GPIO_Pin_8;
  main_wx_srl_ctx_ptr->te_port = GPIOA;
#endif


  // configuring an APRS path used to transmit own packets (telemetry, wx, beacons)
  main_own_path_ln = ConfigPath(main_own_path);

#ifdef INTERNAL_WATCHDOG
  // enable write access to watchdog registers
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  // Set watchdog prescaler
  IWDG_SetPrescaler(IWDG_Prescaler_128);

  // Set the counter value to program watchdog for about 13 seconds
  IWDG_SetReload(0xFFF);

  // enable the watchdog
  IWDG_Enable();

  // do not disable the watchdog when the core is halted on a breakpoint
  DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE); // TODO

  // reload watchdog counter
  IWDG_ReloadCounter();
#endif

#ifdef _METEO
  // initialize i2c controller
  i2cConfigure();
#endif

  // initialize GPIO pins leds are connecting to
  led_init();

  // initalizing separated Open Collector output
  io_oc_init();

  // initialize AX25 & APRS stuff
  AFSK_Init(&main_afsk);
  ax25_init(&main_ax25, &main_afsk, 0, 0x00);
  DA_Init();

  // initialize Watchdog output
  Configure_GPIO(GPIOA,12,GPPP_OUTPUT_50MHZ);

  // initialize variables & arrays in rte_wx
  rte_wx_init();

#ifdef _METEO

  // initialize humidity sensor
  dht22_init();
	#ifndef _DALLAS_SPLIT_PIN
	  dallas_init(GPIOC, GPIO_Pin_6, GPIO_PinSource6, &rte_wx_dallas_average);
	#else
	  dallas_init(GPIOC, GPIO_Pin_11, GPIO_PinSource11, &rte_wx_dallas_average);
	#endif

	#if defined(_UMB_MASTER)
	  	// UMB client cannot be used in the same time with TX20 or analogue anemometer
		#undef _ANEMOMETER_TX20
		#undef _ANEMOMETER_ANALOGUE

	  // client initialization
	  umb_master_init(&rte_wx_umb_context, main_wx_srl_ctx_ptr);
	#endif

	#ifdef  _ANEMOMETER_TX20
	  tx20_init();
	#endif
	#ifdef _ANEMOMETER_ANALOGUE
	  analog_anemometer_init(10, 38, 100, 1);
	#endif

#endif
#ifdef _DALLAS_AS_TELEM
	#ifndef _DALLAS_SPLIT_PIN
	  dallas_init(GPIOC, GPIO_Pin_6, GPIO_PinSource6, &rte_wx_dallas_average);
	#else
	  dallas_init(GPIOC, GPIO_Pin_11, GPIO_PinSource11, &rte_wx_dallas_average);
	#endif
#endif

  // configuring interrupt priorities
  it_handlers_set_priorities();

#if (defined _METEO && defined _SENSOR_MS5611)
 ms5611_reset(&rte_wx_ms5611_qf);
 ms5611_read_calibration(SensorCalData, &rte_wx_ms5611_qf);
 ms5611_trigger_measure(0, 0);
#endif

#if (defined _METEO && defined _SENSOR_BME280)
 bme280_reset(&rte_wx_bme280_qf);
 bme280_setup();
 bme280_read_calibration(bme280_calibration_data);
#endif

  // preparing initial beacon which will be sent to host PC using KISS protocol via UART
  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%07.2f%c%c%08.2f%c%c %s", (float)_LAT, _LATNS, _SYMBOL_F, (float)_LON, _LONWE, _SYMBOL_S, _COMMENT);

  // terminating the aprs message
  main_own_aprs_msg[main_own_aprs_msg_len] = 0;

  // 'sending' the message which will only encapsulate it inside AX25 protocol (ax25_starttx is not called here)
  //ax25_sendVia(&main_ax25, main_own_path, (sizeof(main_own_path) / sizeof(*(main_own_path))), main_own_aprs_msg, main_own_aprs_msg_len);
  ln = ax25_sendVia_toBuffer(main_own_path, (sizeof(main_own_path) / sizeof(*(main_own_path))), main_own_aprs_msg, main_own_aprs_msg_len, srl_usart1_tx_buffer, TX_BUFFER_1_LN);

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
//		  while(srl_rx_state != SRL_RX_DONE);

		  GPIOC->ODR = (GPIOC->ODR ^ GPIO_Pin_9);

		  if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE) {
			  GPIOC->ODR = (GPIOC->ODR ^ GPIO_Pin_8);

			  retval = 200;
		  }
	  }
#endif
  }

  // reinitializing AFSK and AX25 driver
  AFSK_Init(&main_afsk);

  ADCStartConfig();
  DACStartConfig();
  AFSK_Init(&main_afsk);
  ax25_init(&main_ax25, &main_afsk, 0, message_callback);

  // getting all meteo measuremenets to be sure that WX frames want be sent with zeros
  wx_get_all_measurements();

#if defined _VICTRON && !defined _UMB_MASTER
  // initializing protocol parser
  ve_direct_parser_init(&rte_pv_struct, &rte_pv_average);

  // enabling timeout handling for serial port. This is required because VE protocol frame may vary in lenght
  // and serial port driver could finish reception only either on stop character or when declared number of bytes
  // has been received.
  srl_switch_timeout(main_wx_srl_ctx_ptr, 1, 100);

  // switching UART to receive mode to be ready for data from charging controller
  srl_receive_data(main_wx_srl_ctx_ptr, VE_DIRECT_MAX_FRAME_LN, 0x0D, 0, 0, 0, 0);

#elif !defined _VICTRON && defined _UMB_MASTER

//  srl_receive_data(8, SOH, 0x00, 0, 6, 12);


#elif ! defined _VICTRON && !defined _UMB_MASTER
  // switching UART to receive mode to be ready for KISS frames from host
  srl_receive_data(main_kiss_srl_ctx_ptr, 100, FEND, FEND, 0, 0, 0);
#endif

  io_oc_output_low();
  GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);

  // configuting system timers
  TimerConfig();

#ifdef _BCN_ON_STARTUP
	SendStartup();
#endif

#if defined(_UMB_MASTER)
	umb_0x26_status_request(&rte_wx_umb, &rte_wx_umb_context);
#endif

#ifdef INTERNAL_WATCHDOG
   // reload watchdog counter
   IWDG_ReloadCounter();
#endif

#ifdef EXTERNAL_WATCHDOG
   Configure_GPIO(GPIOA,12,GPPP_OUTPUT_2MHZ);	// external watchdog

   GPIOA->ODR ^= GPIO_Pin_12; // Flip the watchdog pin

#endif

  // Infinite loop
  while (1)
    {
	    if (rte_main_reboot_req == 1)
	    	NVIC_SystemReset();

	  	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {

	  		if (main_afsk.sending == false && button_inhibit == 0) {

	  			while(main_ax25.dcd == true);

#ifndef _METEO
	  			//telemetry_send_values(rx10m, tx10m, digi10m, kiss10m, rte_wx_temperature_dallas_valid, rte_wx_dallas_qf, rte_wx_ms5611_qf, rte_wx_dht.qf);
	  			SendOwnBeacon();
#else

	  			//SendWXFrame(rte_wx_average_windspeed, rte_wx_max_windspeed, rte_wx_average_winddirection, rte_wx_temperature_dallas_valid, rte_wx_pressure_valid, rte_wx_humidity);

	  			srl_wait_for_tx_completion(main_kiss_srl_ctx_ptr);

	  			SendWXFrameToBuffer(rte_wx_average_windspeed, rte_wx_max_windspeed, rte_wx_average_winddirection, rte_wx_temperature_average_dallas_valid, rte_wx_pressure_valid, rte_wx_humidity, srl_usart1_tx_buffer, TX_BUFFER_1_LN, &ln);

	  			srl_start_tx(main_kiss_srl_ctx_ptr, ln);
#endif // #ifndef _METEO
	  		}

	  		button_inhibit = 1;
	  	}
	  	else {
	  		button_inhibit = 0;
	  	}

	  	// if new packet has been received from radio channel
		if(ax25_new_msg_rx_flag == 1) {
			memset(srl_usart1_tx_buffer, 0x00, sizeof(srl_usart1_tx_buffer));

			if (main_kiss_enabled == 1) {
				// convert message to kiss format and send it to host
				srl_start_tx(main_kiss_srl_ctx_ptr, SendKISSToHost(ax25_rxed_frame.raw_data, (ax25_rxed_frame.raw_msg_len - 2), srl_usart1_tx_buffer, TX_BUFFER_1_LN));
			}

			main_ax25.dcd = false;
#ifdef _DBG_TRACE
			trace_printf("APRS-RF:RadioPacketFrom=%.6s-%d,FirstPathEl=%.6s-%d\r\n", ax25_rxed_frame.src.call, ax25_rxed_frame.src.ssid, ax25_rxed_frame.rpt_lst[0].call, ax25_rxed_frame.rpt_lst[0].ssid);
#endif
#ifdef _DIGI
			digi_check_with_viscous(&ax25_rxed_frame);

			// check if this packet needs to be repeated (digipeated) and do it if it is necessary
			digi_process(&ax25_rxed_frame);
#endif
			ax25_new_msg_rx_flag = 0;
			rx10m++;
		}

#if defined _VICTRON
		// if new KISS message has been received from the host
		if (srl_rx_state == SRL_RX_DONE || srl_rx_state == SRL_RX_ERROR) {

			// cutting received string to Checksum, everything after will be skipped
			ve_direct_cut_to_checksum(srl_get_rx_buffer(), RX_BUFFER_LN, &buffer_len);

			// checking if this frame is ok
			ve_direct_validate_checksum(srl_get_rx_buffer(), buffer_len, &retval);

			if (retval == 1) {
				// parsing data from input serial buffer to
				retval = ve_direct_parse_to_raw_struct(srl_get_rx_buffer(), buffer_len, &rte_pv_struct);

				if (retval == 0) {
					ve_direct_add_to_average(&rte_pv_struct, &rte_pv_average);

					ve_direct_get_averages(&rte_pv_average, &rte_pv_battery_current, &rte_pv_battery_voltage, &rte_pv_cell_voltage, &rte_pv_load_current);

					ve_direct_set_sys_voltage(&rte_pv_struct, &rte_pv_sys_voltage);

					ve_direct_store_errors(&rte_pv_struct, &rte_pv_last_error);
				}
			}

			srl_receive_data(main_wx_srl_ctx_ptr, VE_DIRECT_MAX_FRAME_LN, 0x0D, 0, 0, 0, 0);
		}
#elif defined _UMB_MASTER
		// if some UMB data have been received
		if (main_wx_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE) {
			umb_pooling_handler(&rte_wx_umb_context, REASON_RECEIVE_IDLE, master_time);
		}

		// if there were an error during receiving frame from host, restart rxing once again
		if (main_wx_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR) {
			umb_pooling_handler(&rte_wx_umb_context, REASON_RECEIVE_ERROR, master_time);
		}

		if (main_wx_srl_ctx_ptr->srl_tx_state == SRL_TX_IDLE) {
			umb_pooling_handler(&rte_wx_umb_context, REASON_TRANSMIT_IDLE, master_time);
		}
#else
		// if new KISS message has been received from the host
		if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE && main_kiss_enabled == 1) {
			// parse incoming data and then transmit on radio freq
			short res = ParseReceivedKISS(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), srl_get_num_bytes_rxed(main_kiss_srl_ctx_ptr), &main_ax25, &main_afsk);
			if (res == 0)
				kiss10m++;	// increase kiss messages counter

			// restart KISS receiving to be ready for next frame
			srl_receive_data(main_kiss_srl_ctx_ptr, 120, FEND, FEND, 0, 0, 0);
		}

		// if there were an error during receiving frame from host, restart rxing once again
		if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR && main_kiss_enabled == 1) {
			srl_receive_data(main_kiss_srl_ctx_ptr, 120, FEND, FEND, 0, 0, 0);
		}
#endif

		// if Davis wx station is enabled and it is alive
		if (main_davis_serial_enabled == 1) {

			// pool the Davis wx station driver for LOOP packet
			davis_loop_packet_pooler(&rte_wx_davis_loop_packet_avaliable);

			davis_rxcheck_packet_pooler();
		}

		// if modbus rtu master is enabled
		if (main_modbus_rtu_master_enabled == 1) {
#ifdef _MODBUS_RTU
			rtu_serial_pool(&main_rtu_pool_queue, main_wx_srl_ctx_ptr);
#endif
		}

		// get all meteo measuremenets each 65 seconds. some values may not be
		// downloaded from sensors if _METEO and/or _DALLAS_AS_TELEM aren't defined
		if (main_wx_sensors_pool_timer < 10) {

			if (main_modbus_rtu_master_enabled == 1) {
				rtu_serial_start();
			}

			wx_get_all_measurements();

			#if defined(_UMB_MASTER)
			umb_0x26_status_request(&rte_wx_umb, &rte_wx_umb_context);
			#endif

			if (main_davis_serial_enabled == 1) {
				davis_trigger_rxcheck_packet();
			}

			main_wx_sensors_pool_timer = 65500;
		}

		if (main_packet_tx_pool_timer < 10) {

			packet_tx_handler();

			main_packet_tx_pool_timer = 60000;
		}

		if (main_one_second_pool_timer < 10) {

			digi_pool_viscous();

			#ifdef _ANEMOMETER_ANALOGUE
			analog_anemometer_direction_handler();
			#endif

			main_one_second_pool_timer = 1000;
		}
		else if (main_one_second_pool_timer < -10) {

			#ifdef _ANEMOMETER_ANALOGUE
			analog_anemometer_direction_reset();
			#endif

			main_one_second_pool_timer = 1000;
		}

		if (main_two_second_pool_timer < 10) {

			wx_pwr_periodic_handle();

			#ifdef INTERNAL_WATCHDOG
			IWDG_ReloadCounter();
			#endif

			main_two_second_pool_timer = 2000;
		}

		if (main_ten_second_pool_timer < 10) {

			#if defined(_UMB_MASTER)
			umb_channel_pool(&rte_wx_umb, &rte_wx_umb_context);
			#endif

			#if defined(_UMB_MASTER)
			rte_wx_umb_qf = umb_get_current_qf(&rte_wx_umb_context, master_time);
			#endif

			wx_pool_anemometer();

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

#ifdef _METEO
		// dht22 sensor communication pooling
		wx_pool_dht22();
#endif
    }
  // Infinite loop, never return.
}

uint16_t main_get_adc_sample(void) {
	return (uint16_t) ADC1->DR;
}



#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
