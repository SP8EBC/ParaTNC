/*
 * it_handlers.c
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#include "it_handlers.h"
#include "station_config_target_hw.h"

#include <delay.h>

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#endif
#ifdef STM32L471xx
#include "cmsis/stm32l4xx/system_stm32l4xx.h"
#include "drivers/sx1262/sx1262.h"
#include "pwr_save.h"
#include <stm32l471xx.h>
#include <stm32l4xx.h>
#include <stm32l4xx_ll_dma.h>
#include <stm32l4xx_ll_tim.h>
#endif

#include "LedConfig.h"
#include "aprs/beacon.h"
#include "aprs/telemetry.h"
#include "aprs/wx.h"
#include "drivers/analog_anemometer.h"
#include "drivers/dallas.h"
#include "drivers/i2c.h"
#include "drivers/ms5611.h"
#include "drivers/serial.h"
#include "drivers/spi.h"
#include "main.h"
// #include "afsk.h"
#include "backup_registers.h"
#include "button.h"
#include "diag/Trace.h"
#include "io.h"
#include "supervisor.h"

#include "main_freertos_externs.h"
#include "rte_main.h"

#include "station_config.h"
#include <stored_configuration_nvm/configuration_handler.h>

/*
 * TIMERS
 *
 * TIM2_IRQHandler
 * TIM4 - windspeed
 * TIM5_IRQHandler
 * TIM7_IRQHandler
 * TIM17 - TIM1_TRG_COM_TIM17_IRQHandler
 */

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/* Zmienne używane do oversamplingu */
static int adc_sample_count = 0, adc_sample_c2 = 0; // Zmienna odliczająca próbki
unsigned short int AdcBuffer[4]; // Bufor przechowujący kolejne wartości rejestru DR
static short int AdcValue;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

uint8_t it_handlers_cpu_load_pool = 0;

uint8_t it_handlers_inhibit_radiomodem_dcd_led = 0;

volatile uint32_t it_handlers_freertos_proxy = 0;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Sets interrupt priorities
 */
void it_handlers_set_priorities (void)
{
	NVIC_SetPriority (TIM2_IRQn, 1); // one-wire delay
	NVIC_SetPriority (I2C1_EV_IRQn, 2);
#ifdef STM32F10X_MD_VL
	NVIC_SetPriority (TIM4_IRQn, 3); // DAC
#else
	NVIC_SetPriority (TIM5_IRQn, 3); // DAC
#endif
	NVIC_SetPriority (DMA2_Channel5_IRQn, 4); // ADC dma transfer (was ADC)
	NVIC_SetPriority (SPI2_IRQn, 5);
	NVIC_SetPriority (USART1_IRQn, 6);		  // kiss
	NVIC_SetPriority (USART2_IRQn, 7);		  // wx
	NVIC_SetPriority (TIM1_UP_TIM16_IRQn, 8); // periodic counters (former Systick)
	NVIC_SetPriority (EXTI4_IRQn, 9);		  // DHT22 humidity sensor
	NVIC_SetPriority (I2C1_ER_IRQn, 10);
	NVIC_SetPriority (EXTI0_IRQn, 11); // FreeRTOS api proxy
	HAL_NVIC_SetPriority (SysTick_IRQn, 15, 0U);
}

#ifdef STM32L471xx
void RTC_WKUP_IRQHandler (void)
{

	rte_main_woken_up = RTE_MAIN_WOKEN_UP_RTC_INTERRUPT;

	backup_reg_set_monitor (13);

	main_reload_internal_wdg ();

	// clear pending interrupt
	NVIC_ClearPendingIRQ (RTC_WKUP_IRQn);

	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;

	// enable write access to RTC registers by writing two magic words
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// disable wakeup interrupt
	RTC->CR &= (0xFFFFFFFF ^ RTC_CR_WUTIE);

	// disable wakeup timer
	RTC->CR &= (0xFFFFFFFF ^ RTC_CR_WUTE);

	// clear Wakeup timer flag
	// This flag is set by hardware when the wakeup auto-reload counter reaches 0.
	// This flag is cleared by software by writing 0.
	// This flag must be cleared by software at least 1.5 RTCCLK periods before WUTF is set to 1
	// again.
	RTC->ISR &= (0xFFFFFFFF ^ RTC_ISR_WUTF_Msk);

	// wait for wakeup timer to disable
	while ((RTC->ISR & RTC_ISR_WUTWF) == 0)
		;

	EXTI->PR1 |= EXTI_PR1_PIF20;

	// disable access do backup domain
	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_DBP);
}

/**
 * Interrupt handler used as a proxy to call FreeRTOS interrupt safe functions, from ISR context
 * at priority higher than a value of configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
 */
void EXTI0_IRQHandler (void)
{

	// clear pending interrupt
	NVIC_ClearPendingIRQ (EXTI0_IRQn);

	if ((IT_HANDLERS_PROXY_KISS_UART_EV & it_handlers_freertos_proxy) != 0) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		BaseType_t xResult = pdFAIL;
		xResult = xEventGroupSetBitsFromISR (main_eventgroup_handle_serial_kiss,
											 MAIN_EVENTGROUP_SERIAL_KISS_RX_DONE,
											 &xHigherPriorityTaskWoken);

		it_handlers_freertos_proxy &= (0xFFFFFFFFu ^ IT_HANDLERS_PROXY_KISS_UART_EV);

		if (xResult != pdFAIL)

		{
			/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
			   switch should be requested. The macro used is port specific and will
			   be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
			   the documentation page for the port being used. */
			portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
		}
	}

	if ((IT_HANDLERS_PROXY_KISS_TX_UART_EV & it_handlers_freertos_proxy) != 0) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		BaseType_t xResult = pdFAIL;
		xResult = xEventGroupSetBitsFromISR (main_eventgroup_handle_serial_kiss,
											 MAIN_EVENTGROUP_SERIAL_KISS_TX_DONE,
											 &xHigherPriorityTaskWoken);

		it_handlers_freertos_proxy &= (0xFFFFFFFFu ^ IT_HANDLERS_PROXY_KISS_TX_UART_EV);

		if (xResult != pdFAIL)

		{
			/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
			   switch should be requested. The macro used is port specific and will
			   be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
			   the documentation page for the port being used. */
			portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
		}
	}

	if ((IT_HANDLERS_PROXY_GSM_RX_UART_EV & it_handlers_freertos_proxy) != 0) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		BaseType_t xResult = pdFAIL;
		xResult = xEventGroupSetBitsFromISR (main_eventgroup_handle_serial_gsm,
											 MAIN_EVENTGROUP_SERIAL_GSM_RX_DONE,
											 &xHigherPriorityTaskWoken);

		it_handlers_freertos_proxy &= (0xFFFFFFFFu ^ IT_HANDLERS_PROXY_GSM_RX_UART_EV);

		if (xResult != pdFAIL)

		{
			/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
			   switch should be requested. The macro used is port specific and will
			   be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
			   the documentation page for the port being used. */
			portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
		}
	}

	if ((IT_HANDLERS_PROXY_GSM_TX_UART_EV & it_handlers_freertos_proxy) != 0) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		BaseType_t xResult = pdFAIL;
		xResult = xEventGroupSetBitsFromISR (main_eventgroup_handle_serial_gsm,
											 MAIN_EVENTGROUP_SERIAL_GSM_TX_DONE,
											 &xHigherPriorityTaskWoken);

		it_handlers_freertos_proxy &= (0xFFFFFFFFu ^ IT_HANDLERS_PROXY_GSM_TX_UART_EV);

		if (xResult != pdFAIL)

		{
			/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
			   switch should be requested. The macro used is port specific and will
			   be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
			   the documentation page for the port being used. */
			portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
		}
	}

	if ((IT_HANDLERS_PROXY_NEW_RADIO_MESSAGE_EV & it_handlers_freertos_proxy) != 0) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		BaseType_t xResult = pdFAIL;
		xResult = xEventGroupSetBitsFromISR (main_eventgroup_handle_radio_message,
											 MAIN_EVENTGROUP_RADIO_MESSAGE_RXED,
											 &xHigherPriorityTaskWoken);

		it_handlers_freertos_proxy &= (0xFFFFFFFFu ^ IT_HANDLERS_PROXY_NEW_RADIO_MESSAGE_EV);

		if (xResult != pdFAIL)

		{
			/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
			   switch should be requested. The macro used is port specific and will
			   be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
			   the documentation page for the port being used. */
			portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
		}
	}
}

void EXTI9_5_IRQHandler (void)
{
	NVIC_ClearPendingIRQ (EXTI9_5_IRQn);

	const uint32_t current_pending = EXTI->PR1;

	// // IS BUSY
	if (current_pending & EXTI_PR1_PIF7) {
		sx1262_busy_released_callback ();
		EXTI->PR1 |= EXTI_PR1_PIF7;
	}

	// INTERRUPT
	if (current_pending & EXTI_PR1_PIF6) {
		sx1262_interrupt_callback ();
		EXTI->PR1 |= EXTI_PR1_PIF6;
	}
}

void SPI2_IRQHandler (void)
{
	NVIC_ClearPendingIRQ (SPI2_IRQn);

	spi_irq_handler ();
}

#endif

// TIM1_UP_TIM16_IRQn
void TIM1_UP_TIM16_IRQHandler (void)
{
	NVIC_ClearPendingIRQ (TIM1_UP_TIM16_IRQn);

	TIM1->SR = 0;

	// systick interrupt is generated every 10ms
	master_time += SYSTICK_TICKS_PERIOD;

	if (master_time > SYSTICK_TICKS_PER_SECONDS * SYSTICK_TICKS_PERIOD * 86400) {
		if (configuration_get_reboot_after_24_hours () == 1) {
			if (pwr_save_is_currently_cutoff () == 0) {
				NVIC_SystemReset ();
			}
		}
	}

	if ((it_handlers_cpu_load_pool++) > SYSTICK_TICKS_PER_SECONDS) {
		main_service_cpu_load_ticks ();
		it_handlers_cpu_load_pool = 0;
	}

	if (supervisor_service () != 0) {
		NVIC_SystemReset ();
	}

	// decrementing a timer to trigger meteo measuremenets
	main_wx_decremenet_counter ();

	main_packets_tx_decremenet_counter ();

	main_one_second_pool_decremenet_counter ();

	main_two_second_pool_decrement_counter ();

	main_four_second_pool_decrement_counter ();

	main_ten_second_pool_decremenet_counter ();

	led_service_blink ();

	srl_keep_timeout (main_kiss_srl_ctx_ptr);
	srl_keep_timeout (main_wx_srl_ctx_ptr);
	srl_keep_timeout (main_gsm_srl_ctx_ptr);

	srl_keep_tx_delay (main_wx_srl_ctx_ptr);

	i2cKeepTimeout ();

	delay_decrement_counter ();

	if (it_handlers_inhibit_radiomodem_dcd_led == 0) {
		led_control_led1_upper (main_ax25.dcd);
	}
}

void USART1_IRQHandler (void)
{
	NVIC_ClearPendingIRQ (USART1_IRQn);
	srl_irq_handler (main_kiss_srl_ctx_ptr);
}

void USART2_IRQHandler (void)
{
	NVIC_ClearPendingIRQ (USART2_IRQn);
	srl_irq_handler (main_wx_srl_ctx_ptr);
}

#ifdef STM32L471xx
void USART3_IRQHandler ()
{
	NVIC_ClearPendingIRQ (USART3_IRQn);
	srl_irq_handler (main_gsm_srl_ctx_ptr);
}

#endif

void I2C1_EV_IRQHandler (void)
{
	NVIC_ClearPendingIRQ (I2C1_EV_IRQn);

	i2cIrqHandler ();
}

void I2C1_ER_IRQHandler (void)
{
	i2cErrIrqHandler ();
}

void TIM2_IRQHandler (void)
{
	TIM2->SR &= ~(1 << 0);
	if (delay_5us > 0) {
		delay_5us--;
	}
}

void TIM1_TRG_COM_TIM17_IRQHandler (void)
{
	NVIC_ClearPendingIRQ (TIM1_TRG_COM_TIM17_IRQn);
	TIM17->SR &= ~(1 << 0);

#if defined(_ANEMOMETER_ANALOGUE) || defined(_ANEMOMETER_ANALOGUE_SPARKFUN)
	analog_anemometer_timer_irq ();
#endif
}

#ifdef STM32F10X_MD_VL
void DMA1_Channel7_IRQHandler ()
{ // DMA1_Channel7_IRQn
#else
void DMA1_Channel5_IRQHandler ()
{ // DMA1_Channel7_IRQn
#endif
#ifdef STM32F10X_MD_VL
	NVIC_ClearPendingIRQ (DMA1_Channel7_IRQn);
	DMA_ClearITPendingBit (DMA1_IT_GL7);
#endif

#ifdef STM32L471xx
	LL_DMA_ClearFlag_TC5 (DMA1);
#endif

#if defined(_ANEMOMETER_ANALOGUE) || defined(_ANEMOMETER_ANALOGUE_SPARKFUN)
	analog_anemometer_dma_irq ();
#endif
}

#ifdef STM32F10X_MD_VL
void TIM4_IRQHandler (void)
{
	// obsluga przerwania cyfra-analog
	TIM4->SR &= ~(1 << 0);

	DAC->DHR8R1 = AFSK_DAC_ISR (&main_afsk);
	DAC->SWTRIGR |= 1;

#ifdef STM32L471xx
	DAC->DHR8R2 = AFSK_DAC_ISR (&main_afsk);
	DAC->SWTRIGR |= 2;
#endif

	if ((main_config_data_mode->wx & WX_ENABLED) == 0) {
		led_control_led2_bottom (main_afsk.sending);
	}
}
#else
void TIM5_IRQHandler (void)
{
	// obsluga przerwania cyfra-analog
	TIM5->SR &= ~(1 << 0);

	DAC->DHR8R2 = AFSK_DAC_ISR (&main_afsk);
	DAC->SWTRIGR |= 2;

	if ((main_config_data_mode->wx & WX_ENABLED) == 0) {
		led_control_led2_bottom (main_afsk.sending);
	}
}
#endif

extern LL_DMA_InitTypeDef timer_config_DMA_InitStruct;

void DMA2_Channel5_IRQHandler ()
{
#define ASC2 adc_sample_c2

	LL_DMA_ClearFlag_TC5 (DMA2);

	AdcValue = (short int)((AdcBuffer[0] + AdcBuffer[1] + AdcBuffer[2] + AdcBuffer[3]) >> 1);
	AFSK_ADC_ISR (&main_afsk, (AdcValue - 4095));

	if (ASC2++ == 2) {
		// pooling AX25 musi być tu bo jak z przerwania wyskoczy nadawanie WX, BCN, TELEM przy
		// dcd == true to bedzie wisialo w nieskonczonosc bo ustawiania dcd jest w srodku
		// ax25_poll
		ax25_poll (&main_ax25);
		ASC2 = 0;
	}
	else {
	}

	NVIC_ClearPendingIRQ (DMA2_Channel5_IRQn);

	LL_DMA_DeInit(DMA2, LL_DMA_CHANNEL_5);
	LL_DMA_Init(DMA2, LL_DMA_CHANNEL_5, &timer_config_DMA_InitStruct);

	LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_5);

	LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_5);
}

// void TIM7_IRQHandler (void)
//{
//	// obsluga przetwarzania analog-cyfra. Wersja z oversamplingiem
//	TIM7->SR &= ~(1 << 0);
//	NVIC_ClearPendingIRQ (TIM7_IRQn);
// #define ASC	 adc_sample_count
// #define ASC2 adc_sample_c2
//	AdcBuffer[ASC] = ADC1->DR;
//	if (ASC == 3) {
//		//		io_ext_watchdog_service();
//		AdcValue = (short int)((AdcBuffer[0] + AdcBuffer[1] + AdcBuffer[2] + AdcBuffer[3]) >> 1);
//		AFSK_ADC_ISR (&main_afsk, (AdcValue - 4095));
//		ASC = 0;
//
//		if (ASC2++ == 2) {
//			// pooling AX25 musi być tu bo jak z przerwania wyskoczy nadawanie WX, BCN, TELEM przy
//			// dcd == true to bedzie wisialo w nieskonczonosc bo ustawiania dcd jest w srodku
//			// ax25_poll
//			ax25_poll (&main_ax25);
//			ASC2 = 0;
//		}
//		else {
//		}
//	}
//	else {
//		ASC++;
//	}
// }
