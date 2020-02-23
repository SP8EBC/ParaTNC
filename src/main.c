#include <delay.h>
#include <LedConfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x.h>

#include "main.h"
#include "packet_tx_handler.h"
#include "station_config.h"

#include "diag/Trace.h"
#include "antilib_adc.h"
#include "afsk_pr.h"
#include "drivers/serial.h"
#include "TimerConfig.h"
#include "PathConfig.h"

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

//#include "Timer.h"
//#include "BlinkLed.h"

#ifdef _METEO
#include <wx_handler.h>
#include "drivers/dallas.h"
#include "drivers/ms5611.h"
#include "drivers/i2c.h"
#include "drivers/tx20.h"
#include "drivers/analog_anemometer.h"
#include "aprs/wx.h"
#endif

#ifdef _DALLAS_AS_TELEM
#include "drivers/dallas.h"
#endif

#include "KissCommunication.h"

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

char after_tx_lock;

unsigned short rx10m = 0, tx10m = 0, digi10m = 0, kiss10m = 0;


static void message_callback(struct AX25Msg *msg) {

}

int
main(int argc, char* argv[])
{
  // Send a greeting to the trace device (skipped on Release).
//  trace_puts("Hello ARM World!");

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

  // configuring an APRS path used to transmit own packets (telemetry, wx, beacons)
  main_own_path_ln = ConfigPath(main_own_path);

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

  // initialize variables & arrays in rte_wx
  rte_wx_init();

#ifdef _METEO
  // initialize sensor power control and switch off supply voltage
  wx_pwr_init();

  // call periodic handle to wait for 1 second and then switch on voltage
  wx_pwr_periodic_handle();

  // initialize humidity sensor
  dht22_init();
	#ifndef _DALLAS_SPLIT_PIN
	  dallas_init(GPIOC, GPIO_Pin_6, GPIO_PinSource6, &rte_wx_dallas_average);
	#else
	  dallas_init(GPIOC, GPIO_Pin_11, GPIO_PinSource11, &rte_wx_dallas_average);
	#endif

	#ifdef  _ANEMOMETER_TX20
	  TX20Init();
	#endif
	#ifdef _ANEMOMETER_ANALOGUE
	  analog_anemometer_init(0, 10, 100, 1);
	#endif

#endif
#ifdef _DALLAS_AS_TELEM
	#ifndef _DALLAS_SPLIT_PIN
	  dallas_init(GPIOC, GPIO_Pin_6, GPIO_PinSource6, &rte_wx_dallas_average);
	#else
	  dallas_init(GPIOC, GPIO_Pin_11, GPIO_PinSource11, &rte_wx_dallas_average);
	#endif
#endif

  // initializing UART drvier
  srl_init();

  // configuring interrupt priorities
  it_handlers_set_priorities();

#ifdef _METEO
 ms5611_reset(&rte_wx_ms5611_qf);
 ms5611_read_calibration(SensorCalData, &rte_wx_ms5611_qf);
 ms5611_trigger_measure(0, 0);
#endif

  // preparing initial beacon which will be sent to host PC using KISS protocol via UART
  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%07.2f%c%c%08.2f%c%c %s", (float)_LAT, _LATNS, _SYMBOL_F, (float)_LON, _LONWE, _SYMBOL_S, _COMMENT);

  // terminating the aprs message
  main_own_aprs_msg[main_own_aprs_msg_len] = 0;

  // 'sending' the message which will only encapsulate it inside AX25 protocol (ax25_starttx is not called here)
  //ax25_sendVia(&main_ax25, main_own_path, (sizeof(main_own_path) / sizeof(*(main_own_path))), main_own_aprs_msg, main_own_aprs_msg_len);
  ln = ax25_sendVia_toBuffer(main_own_path, (sizeof(main_own_path) / sizeof(*(main_own_path))), main_own_aprs_msg, main_own_aprs_msg_len, srl_tx_buffer, TX_BUFFER_LN);

  // SendKISSToHost function cleares the output buffer hence routine need to wait till the UART will be ready for next transmission.
  // Here this could be omitted because UART isn't used before but general idea
  while(srl_tx_state != SRL_TX_IDLE && srl_tx_state != SRL_TX_ERROR);

  // converting AX25 with beacon to KISS format
  //ln = SendKISSToHost(main_afsk.tx_buf + 1, main_afsk.tx_fifo.tail - main_afsk.tx_fifo.head - 4, srl_tx_buffer, TX_BUFFER_LN);

  // checking if KISS-framing was done correctly
  if (ln != KISS_TOO_LONG_FRM) {
#ifdef SERIAL_TX_TEST_MODE
	  // infinite loop for testing UART transmission
	  for (;;) {

		  retval = srl_receive_data(100, FEND, FEND, 0, 0, 0);
#endif
		  retval = srl_start_tx(ln);

#ifdef SERIAL_TX_TEST_MODE
		  while(srl_tx_state != SRL_TX_IDLE);

		  GPIOC->ODR = (GPIOC->ODR ^ GPIO_Pin_9);

		  if (srl_rx_state == SRL_RX_DONE) {
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

#ifdef _VICTRON
  // initializing protocol parser
  ve_direct_parser_init(&rte_pv_struct, &rte_pv_average);

  // enabling timeout handling for serial port. This is required because VE protocol frame may vary in lenght
  // and serial port driver could finish reception only either on stop character or when declared number of bytes
  // has been received.
  srl_switch_timeout(1, 100);

  // switching UART to receive mode to be ready for data from charging controller
  srl_receive_data(VE_DIRECT_MAX_FRAME_LN, 0x0D, 0, 0, 0, 0);
#else
  // switching UART to receive mode to be ready for KISS frames from host
  srl_receive_data(100, FEND, FEND, 0, 0, 0);
#endif

  GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);

  // configuting system timers
  TimerConfig();

#ifdef _BCN_ON_STARTUP
	SendStartup();
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
		#ifdef _ANEMOMETER_TX20

	  			SendWXFrame(&VNAME, rte_wx_temperature_dallas_valid, rte_wx_pressure_valid);
		#else
	  			SendWXFrame(rte_wx_average_windspeed, rte_wx_max_windspeed, rte_wx_average_winddirection, rte_wx_temperature_dallas_valid, rte_wx_pressure_valid);

		#endif // #ifdef _ANEMOMETER_TX20
#endif // #ifndef _METEO
	  		}

	  		button_inhibit = 1;
	  	}
	  	else {
	  		button_inhibit = 0;
	  	}

	  	// if new packet has been received from radio channel
		if(ax25_new_msg_rx_flag == 1) {
			memset(srl_tx_buffer, 0x00, sizeof(srl_tx_buffer));

			// convert message to kiss format and send it to host
			srl_start_tx(SendKISSToHost(ax25_rxed_frame.raw_data, (ax25_rxed_frame.raw_msg_len - 2), srl_tx_buffer, TX_BUFFER_LN));

			main_ax25.dcd = false;
#ifdef _DBG_TRACE
			trace_printf("APRS-RF:RadioPacketFrom=%.6s-%d,FirstPathEl=%.6s-%d\r\n", ax25_rxed_frame.src.call, ax25_rxed_frame.src.ssid, ax25_rxed_frame.rpt_lst[0].call, ax25_rxed_frame.rpt_lst[0].ssid);
#endif
#ifdef _DIGI
			// check if this packet needs to be repeated (digipeated) and do it if it is neccessary
			Digi(&ax25_rxed_frame);
#endif
			ax25_new_msg_rx_flag = 0;
			rx10m++;
		}

#ifdef _VICTRON
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

			srl_receive_data(VE_DIRECT_MAX_FRAME_LN, 0x0D, 0, 0, 0, 0);
		}
#else
		// if new KISS message has been received from the host
		if (srl_rx_state == SRL_RX_DONE) {
			// parse incoming data and then transmit on radio freq
			short res = ParseReceivedKISS(srl_get_rx_buffer(), srl_get_num_bytes_rxed(), &main_ax25, &main_afsk);
			if (res == 0)
				kiss10m++;	// increase kiss messages counter

			// restart KISS receiving to be ready for next frame
			srl_receive_data(120, FEND, FEND, 0, 0, 0);
		}

		// if there were an error during receiving frame from host, restart rxing once again
		if (srl_rx_state == SRL_RX_ERROR) {
			srl_receive_data(120, FEND, FEND, 0, 0, 0);
		}
#endif

		// get all meteo measuremenets each 65 seconds. some values may not be
		// downloaded from sensors if _METEO and/or _DALLAS_AS_TELEM aren't defined
		if (main_wx_sensors_pool_timer < 10) {

			wx_get_all_measurements();

			main_wx_sensors_pool_timer = 65500;
		}

		if (main_packet_tx_pool_timer < 10) {

			packet_tx_handler();

			main_packet_tx_pool_timer = 60000;
		}

		if (main_one_second_pool_timer < 10) {

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

			main_two_second_pool_timer = 2000;
		}

		if (main_ten_second_pool_timer < 10) {

			wx_pool_analog_anemometer();

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
