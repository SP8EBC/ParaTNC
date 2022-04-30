
#include "drivers/l4/pwm_input_stm32l4x.h"

#include <stm32l4xx.h>
#include <stm32l4xx_ll_tim.h>
#include <stm32l4xx_ll_gpio.h>

uint8_t pwm_input_current_channel = 0;
uint32_t pwm_first_channel = 0;
uint32_t pwm_second_channel = 0;


void pwm_input_io_init(void) {

	LL_GPIO_InitTypeDef GPIO_InitTypeDef;

	// PC6 - PWM_CH1
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_6;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_DOWN;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_3;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	// PC7 - PWM_CH2
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_7;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

}

void pwm_input_init(uint8_t channel) {

	// check if user provided a channel which make any sense
	if (channel == 0 || channel > 2) {
		return;
	}

	// save current channel what is needed to pooling
	pwm_input_current_channel = channel;

	// disable timer
	TIM8->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);

	// this will produce 10kHz timebase, counter will overflow every ~6.5 seconds
	TIM8->PSC = 2399;

	// reset CCER value
	TIM8->CCER = 0;

	// reset CCMR configuration
	TIM8->CCMR1 = 0;

	if (channel == 1) {
		// 01: CC1 channel is configured as input, IC1 is mapped on TI1
		TIM8->CCMR1 |= TIM_CCMR1_CC1S_0;

		// 10: CC2 channel is configured as input, IC2 is mapped on TI1
		TIM8->CCMR1 |= TIM_CCMR1_CC2S_1;
	}
	else if (channel == 2) {
		// 10: CC1 channel is configured as input, IC1 is mapped on TI2
		TIM8->CCMR1 |= TIM_CCMR1_CC1S_1;

		// 01: CC2 channel is configured as input, IC2 is mapped on TI2
		TIM8->CCMR1 |= TIM_CCMR1_CC2S_0;
	}

	/**
	 * 1: OC1 active low (output mode) / Edge sensitivity selection (input mode, see below)
		When CC1 channel is configured as input, both CC1NP/CC1P bits select the active polarity
		of TI1FP1 and TI2FP1 for trigger or capture operations.

		CC1NP=0, CC1P=0: non-inverted/rising edge. The circuit is sensitive to TIxFP1 rising edge
		(capture or trigger operations in reset, external clock or trigger mode),
		TIxFP1 is not inverted (trigger operation in gated mode or encoder
		mode).

		CC1NP=0, CC1P=1: inverted/falling edge. The circuit is sensitive to TIxFP1 falling edge
		(capture or trigger operations in reset, external clock or trigger mode),
		TIxFP1 is inverted (trigger operation in gated mode or encoder mode).

		CC1NP=1, CC1P=1: non-inverted/both edges/ The circuit is sensitive to both TIxFP1 rising
		and falling edges (capture or trigger operations in reset, external clock
		or trigger mode), TIxFP1is not inverted (trigger operation in gated
		mode). This configuration must not be used in encoder mode.
	 *
	 *
	 */
	TIM8->CCER |= TIM_CCER_CC2P;

	// This bit-field selects the trigger input to be used to synchronize the counter.
	// 101: Filtered Timer Input 1 (TI1FP1)
	TIM8->SMCR |= (TIM_SMCR_TS_2 | TIM_SMCR_TS_0);

	// Slave mode selection
	// 0100: Reset Mode - Rising edge of the selected trigger input (TRGI) reinitializes the counter
	// and generates an update of the registers.
	TIM8->SMCR |= TIM_SMCR_SMS_2;

	// 1: Capture mode enabled / OC1 signal is output on the corresponding output pin
	TIM8->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

	// enable timer
	TIM8->CR1 |= TIM_CR1_CEN;

}

void pwm_input_pool(void) {

	uint32_t pwm_rising_edge = TIM8->CCR1 + 1;
	uint32_t pwm_falling_edge = TIM8->CCR2 + 1;
	uint32_t pwm_result = 0;

	// check preconditions
	if ((pwm_rising_edge != 0) && (pwm_falling_edge != 0)) {
		if (pwm_rising_edge > pwm_falling_edge) {
			// result value is in percents scaled * 10 (100 means 10 percents)
			pwm_falling_edge *= 1000;

			pwm_result = (pwm_falling_edge / pwm_rising_edge );
		}
	}
	else {
		; // if not do nothing and keeps result to zero
	}

	if (pwm_input_current_channel == 1) {

		// save a result of PWM measurement
		pwm_first_channel = pwm_result;

		// switch the channel to second
		pwm_input_init(2);
	}
	else if (pwm_input_current_channel == 1) {
		pwm_second_channel = pwm_result;

		pwm_input_init(1);


	}
}
