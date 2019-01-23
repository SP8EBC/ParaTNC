
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x.h>

#include "main.h"

#include "station_config.h"

#include "diag/Trace.h"
#include "antilib_adc.h"
#include "afsk_pr.h"
#include "drivers/serial.h"
#include "TimerConfig.h"
#include "LedConfig.h"
#include "PathConfig.h"

#include "aprs/digi.h"
#include "aprs/telemetry.h"
#include "aprs/dac.h"
#include "aprs/beacon.h"


//#include "Timer.h"
//#include "BlinkLed.h"

#ifdef _METEO
#include "drivers/dallas.h"
#include "drivers/ms5611.h"
#include "drivers/i2c.h"
#include "drivers/tx20.h"
#include "drivers/_dht22.h"
#include "aprs/wx.h"
#endif

#ifdef _DALLAS_AS_TELEM
#include "drivers/dallas.h"
#endif

#include "KissCommunication.h"

//#define SERIAL_TX_TEST_MODE

// Niebieska dioda -> DCD
// Zielona dioda -> anemometr albo TX

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

// global variables represending the AX25/APRS stack
AX25Ctx ax25;
Afsk a;


AX25Call path[3];
uint8_t path_len = 0;
uint8_t aprs_msg_len;
char aprs_msg[128];

// global variable used to store return value from various functions
volatile uint8_t retval = 100;

char after_tx_lock;

unsigned char BcnInterval, WXInterval, BcnI = _BCN_INTERVAL - 2, WXI = _WX_INTERVAL - 1, TelemInterval, TelemI = 1;
unsigned short rx10m = 0, tx10m = 0, digi10m = 0, kiss10m = 0;
int t = 0;

float temperature;
float td;
double pressure = 0.0;

#ifdef _METEO
dht22Values dht, dht_valid;
#endif

static void message_callback(struct AX25Msg *msg) {

}

int
main(int argc, char* argv[])
{
  // Send a greeting to the trace device (skipped on Release).
//  trace_puts("Hello ARM World!");

  int32_t ln = 0;

  RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM7EN | RCC_APB1ENR_TIM4EN);
  RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_TIM1EN);

  memset(aprs_msg, 0x00, 128);

  // choosing the signal source for the SysTick timer.
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  // Configuring the SysTick timer to generate interrupt 100x per second (one interrupt = 10ms)
  SysTick_Config(SystemCoreClock / 100);

  path_len = ConfigPath(path);

#ifdef _METEO
//  DHT22_Init();
  i2cConfigure();
#endif
  LedConfig();
  AFSK_Init(&a);
  ax25_init(&ax25, &a, 0, 0x00);
  DA_Init();

#ifdef _METEO
  dht22_init();
  DallasInit(GPIOC, GPIO_Pin_6, GPIO_PinSource6);
  TX20Init();
#endif
#ifdef _DALLAS_AS_TELEM
  DallasInit(GPIOC, GPIO_Pin_6, GPIO_PinSource6);
#endif
  srl_init();

  td = 0.0f;
  temperature = 0.0f;

  BcnInterval = _BCN_INTERVAL;
  WXInterval = _WX_INTERVAL;
  TelemInterval = 10;

#ifdef _METEO
 SensorReset(0xEC);
 td = DallasQuery();
 SensorReadCalData(0xEC, SensorCalData);
 SensorStartMeas(0);
#endif

  // preparing initial beacon which will be sent to host PC using KISS protocol via UART
  aprs_msg_len = sprintf(aprs_msg, "=%07.2f%c%c%08.2f%c%c %s", (float)_LAT, _LATNS, _SYMBOL_F, (float)_LON, _LONWE, _SYMBOL_S, _COMMENT);

  // terminating the aprs message
  aprs_msg[aprs_msg_len] = 0;

  // 'sending' the message which will only encapsulate it inside AX25 protocol (ax25_starttx is not called here)
  ax25_sendVia(&ax25, path, (sizeof(path) / sizeof(*(path))), aprs_msg, aprs_msg_len);

  // SendKISSToHost function cleares the output buffer hence routine need to wait till the UART will be ready for next transmission.
  // Here this could be omitted because UART isn't used before but general idea
  while(srl_tx_state != SRL_TX_IDLE && srl_tx_state != SRL_TX_ERROR);

  // converting AX25 with beacon to KISS format
  ln = SendKISSToHost(a.tx_buf + 1, a.tx_fifo.tail - a.tx_fifo.head - 4, srl_tx_buffer, TX_BUFFER_LN);

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

		  if (srl_rx_state == SRL_RX_DONE) {
			  retval = 200;
		  }
	  }
#endif
  }

  // reinitializing AFSK and AX25 driver
  AFSK_Init(&a);

  ADCStartConfig();
  DACStartConfig();
  AFSK_Init(&a);
  ax25_init(&ax25, &a, 0, message_callback);

  srl_receive_data(100, FEND, FEND, 0, 0, 0);


#ifdef _METEO
  temperature = SensorBringTemperature();
  td = DallasQuery();
#ifdef _DBG_TRACE
  trace_printf("temperatura DS: %d\r\n", (int)td);
#endif
  pressure = (float)SensorBringPressure();
#ifdef _DBG_TRACE
  trace_printf("cisnienie MS: %d\r\n", (int)pressure);
#endif
#endif

  GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);

  TimerConfig();

#ifdef _BCN_ON_STARTUP
	SendOwnBeacon();
#endif

  // Infinite loop
  while (1)
    {
	  	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {
			SendSimpleTelemetry(1);
	  	}
	  	else {
	  		;
	  	}

		if(new_msg_rx == 1) {
			memset(srl_tx_buffer, 0x00, sizeof(srl_tx_buffer));
			srl_start_tx(SendKISSToHost(msg.raw_data, (msg.raw_msg_len - 2), srl_tx_buffer, TX_BUFFER_LN));

			ax25.dcd = false;
#ifdef _DBG_TRACE
			trace_printf("APRS-RF:RadioPacketFrom=%.6s-%d,FirstPathEl=%.6s-%d\r\n", msg.src.call, msg.src.ssid, msg.rpt_lst[0].call, msg.rpt_lst[0].ssid);
#endif
#ifdef _DIGI
			Digi(&msg);
#endif
			new_msg_rx = 0;
			rx10m++;
		}

		if (srl_rx_state == SRL_RX_DONE) {
			short res = ParseReceivedKISS(srl_get_rx_buffer(), &ax25, &a);
			if (res == 0)
				kiss10m++;

			srl_receive_data(120, FEND, FEND, 0, 0, 0);
		}
#ifdef _METEO
		dht22_timeout_keeper();

		switch (dht22State) {
			case DHT22_STATE_IDLE:
				dht22_comm(&dht);
				break;
			case DHT22_STATE_DATA_RDY:
				dht22_decode(&dht);
				break;
			case DHT22_STATE_DATA_DECD:
				dht_valid = dht;			// powrot do stanu DHT22_STATE_IDLE jest w TIM3_IRQHandler
				dht22State = DHT22_STATE_DONE;

#ifdef _DBG_TRACE
				trace_printf("DHT22: temperature=%d,humi=%d\r\n", dht_valid.scaledTemperature, dht_valid.humidity);
#endif
				break;
			default: break;
		}
#endif
    }
  // Infinite loop, never return.
}

uint16_t main_get_adc_sample(void) {
	return (uint16_t) ADC1->DR;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
