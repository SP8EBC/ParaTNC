/*
 * it_handlers.c
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#include <stm32f10x.h>
#include "drivers/dallas.h"
#include "drivers/tx20.h"
#include "drivers/ms5611.h"
#include "drivers/_dht22.h"
#include "aprs/wx.h"
#include "aprs/telemetry.h"
#include "aprs/beacon.h"
#include "main.h"

#include "diag/Trace.h"

#include "station_config.h"



// TIM1 w TX20

/* Zmienne używane do oversamplingu */
char adc_sample_count = 0, adc_sample_c2 = 0;				// Zmienna odliczająca próbki
unsigned short int AdcBuffer[4];		// Bufor przechowujący kolejne wartości rejestru DR
short int AdcValue;

void USART1_IRQHandler(void) {
	NVIC_ClearPendingIRQ(USART1_IRQn);
	srl_irq_handler();
}

void I2C1_EV_IRQHandler(void) {
	NVIC_ClearPendingIRQ(I2C1_EV_IRQn);

	i2cIrqHandler();

}

void I2C1_ER_IRQHandler(void) {
	i2cErrIrqHandler();
}

void EXTI4_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR4;
  dht22_timeout_keeper();
}

void TIM2_IRQHandler( void ) {
	TIM2->SR &= ~(1<<0);
	if (delay_5us > 0)
		delay_5us--;

}

void TIM3_IRQHandler(void) {
// wysylanie wlasnej pozycji i danych WX
	TIM3->SR &= ~(1<<0);
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
	if (dht22State == DHT22_STATE_DONE || dht22State == DHT22_STATE_TIMEOUT)
		dht22State = DHT22_STATE_IDLE;

#ifndef _MUTE_OWN
		if (WXInterval != 0 && WXI >= WXInterval) {
			trace_printf("Pogoda\r\n");

			  temperature = SensorBringTemperature();
			  td = DallasQuery();
			  trace_printf("temperatura DS: %d\r\n", (int)td);
			  pressure = (float)SensorBringPressure();
			  trace_printf("cisnienie MS: %d\r\n", (int)pressure);
//		while(ax25.afsk->hdlc.raw_dcd == TRUE);
		SendWXFrame(&VNAME, td, pressure);
		while (a.sending == 1);
		WXI = 1;
		}
	else
		WXI++;
#endif
#endif
		if (BcnInterval != 0 && BcnI >= BcnInterval) {
#ifndef _MUTE_OWN
//		while(ax25.afsk->hdlc.raw_dcd == TRUE);
		trace_printf("Wlasny beacon\r\n");
		SendOwnBeacon();
		while (a.sending == 1);
#endif
		BcnI = 1;
	}
	else
		BcnI++;
	if (TelemInterval != 0 && TelemI >= TelemInterval) {
#ifndef _MUTE_OWN
		trace_printf("Telemetria\r\n");
//		ch14 = ADCReturnChannel(14);
//		ch15 = ADCReturnChannel(15);
//		ADCStartConfig();
//		while(ax25.afsk->hdlc.raw_dcd == TRUE);
		if (t % 12 == 0 || t == 0)
			SendSimpleTelemetry(1);
		else
			SendSimpleTelemetry(0);
		while (a.sending == 1);
#endif
		TelemI = 1;
	}
	else
		TelemI++;
}

void TIM4_IRQHandler( void ) {
	// obsluga przerwania cyfra-analog
	TIM4->SR &= ~(1<<0);
	if (timm == 0) {
		DAC->DHR8R1 = AFSK_DAC_ISR(&a);
		DAC->SWTRIGR |= 1;
	}
	else {
			if (delay_5us > 0)
				delay_5us--;
	}

}

void TIM7_IRQHandler(void) {
// obsluga przetwarzania analog-cyfra. Wersja z oversamplingiem
	TIM7->SR &= ~(1<<0);
	#define ASC adc_sample_count
	#define ASC2 adc_sample_c2
	AdcBuffer[ASC] =  ADC1->DR;
	if(ASC == 3) {
		AdcValue = (short int)(( AdcBuffer[0] + AdcBuffer[1] + AdcBuffer[2] + AdcBuffer[3]) >> 1);
		AFSK_ADC_ISR(&a, (AdcValue - 4095) );
		if(ax25.dcd == true) {		// niebieska dioda
			GPIOC->BSRR |= GPIO_BSRR_BS8;
		}
		else {
			GPIOC->BSRR |= GPIO_BSRR_BR8;
		}
		ASC = 0;

		if (ASC2++ == 2) {
			// pooling AX25 musi być tu bo jak z przerwania wyskoczy nadawanie WX, BCN, TELEM przy dcd == true
			// to bedzie wisialo w nieskonczonosc bo ustawiania dcd jest w srodku ax25_poll
			ax25_poll(&ax25);
			ASC2 = 0;
		}
		else {

		}

	}
	else
		ASC++;

}

