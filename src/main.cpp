//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x_rcc.h>

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


#include "Timer.h"
#include "BlinkLed.h"

#ifdef _METEO
#include "drivers/dallas.h"
#include "drivers/ms5611.h"
#include "drivers/i2c.h"
#include "drivers/tx20.h"
#include "aprs/wx.h"
#endif

#ifdef _DALLAS_AS_TELEM
#include "drivers/dallas.h"
#endif

#include "KissCommunication.h"


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

AX25Ctx ax25;
Afsk a;

AX25Call path[3];
uint8_t path_len = 0;
uint8_t aprs_msg_len;
char aprs_msg[128];

char after_tx_lock;

unsigned char BcnInterval, WXInterval, BcnI = _BCN_INTERVAL - 2, WXI = _WX_INTERVAL - 1, TelemInterval, TelemI = 1;
unsigned short rx10m = 0, tx10m = 0, digi10m = 0, kiss10m = 0;
int t = 0;

float temperature;
float td;
double pressure = 0.0;

static void message_callback(struct AX25Msg *msg) {

}

int
main(int argc, char* argv[])
{
  // Send a greeting to the trace device (skipped on Release).
//  trace_puts("Hello ARM World!");

  char rsoutput[20];

  RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM7EN | RCC_APB1ENR_TIM4EN);
  RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_TIM1EN);

  memset(aprs_msg, 0x00, 128);

  path_len = ConfigPath(path);

#ifdef _METEO
  i2cConfigure();
#endif
  LedConfig();
  AFSK_Init(&a);
  ax25_init(&ax25, &a, 0, 0x00);
  DA_Init();

  TimerConfig();
#ifdef _METEO
  DallasInit(GPIOC, GPIO_Pin_6, GPIO_PinSource6);
  TX20Init();
#endif
#ifdef _DALLAS_AS_TELEM
  DallasInit(GPIOC, GPIO_Pin_6, GPIO_PinSource6);
#endif
  SrlConfig();

  td = 0.0;
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

  aprs_msg_len = sprintf(aprs_msg, "=%07.2f%c%c%08.2f%c%c %s\0", (float)_LAT, _LATNS, _SYMBOL_F, (float)_LON, _LONWE, _SYMBOL_S, _COMMENT);
  aprs_msg[aprs_msg_len] = 0;
  ax25_sendVia(&ax25, path, (sizeof(path) / sizeof(*(path))), aprs_msg, aprs_msg_len);
  SrlStartTX(SendKISSToHost(0x00, a.tx_buf + 1, a.tx_fifo.tail - a.tx_fifo.head - 4, srlTXData));
  while(srlTXing == 1);
  AFSK_Init(&a);

  ADCStartConfig();
  DACStartConfig();
  AFSK_Init(&a);
  ax25_init(&ax25, &a, 0, message_callback);

	SrlReceiveData(100, FEND, FEND, 0, 0, 0);


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

	  	}

		if(new_msg_rx == 1) {
			memset(srlTXData, 0x00, sizeof(srlTXData));
			SrlStartTX(SendKISSToHost(0x00, msg.raw_data, (msg.raw_msg_len - 2), srlTXData));
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

		if (srlIdle == 1) {
			short res = ParseReceivedKISS(srlRXData, &ax25, &a);
			if (res == 0)
				kiss10m++;

			SrlReceiveData(120, FEND, FEND, 0, 0, 0);
		}

    }
  // Infinite loop, never return.
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
