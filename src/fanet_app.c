/*
 * fanet_app.c
 *
 *  Created on: Feb 28, 2025
 *      Author: mateusz
 */

#include "fanet_app.h"

#ifdef STM32L471xx

#include "drivers/sx1262/sx1262_modes.h"
#include "drivers/sx1262/sx1262_status.h"
#include "drivers/sx1262/sx1262_rf.h"
#include "drivers/sx1262/sx1262_irq_dio.h"
#include "drivers/sx1262/sx1262_data_io.h"

#include "skytrax_fanet/fanet_factory_frames.h"
#include "skytrax_fanet/fanet_serialization.h"

#include "delay.h"
#include "io.h"
#include "LedConfig.h"

#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#ifdef SX1262_SHMIDT_NOT_GATE
#define SX1262_BUSY_ACTIVE 		0U
#define SX1262_BUSY_NOTACTIVE	1U
#else
#define SX1262_BUSY_ACTIVE 		1U
#define SX1262_BUSY_NOTACTIVE	0U
#endif

#define SX1262_FUCK_THIS_AND_WAIT			delay_fixed(10);

#define FANET_TURN_OFF()	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12)
#define FANET_TURN_ON()		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);

#define SX1262_CHECK_NOK_RESULT(res, code)	if(res != SX1262_API_OK) {			\
												i = code;						\
												goto nok;						\
											}									\


/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

#ifdef SX1262_IMPLEMENTATION
static uint8_t fanet_test_array[64];

static const uint8_t fanet_test_2[7] = {0x42u, 0x12u, 0x34u, 0x56u, 0x31u, 0x32u, 0x33u};

volatile static uint16_t last_interrupt_mask = 0;

int  fanet_success_cnt = 0;
int  fanet_fail_cnt = 0;

#endif

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

// this also should be moved to sx1262 driver
static uint8_t fanet_wait_not_busy(int _unused)
{
	(void)_unused;
	// PC6
	if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_ACTIVE)
	{
		while(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_ACTIVE);
		SX1262_FUCK_THIS_AND_WAIT
		return 0;
	}
	else
	{
		while(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_NOTACTIVE)
		{
			_unused--;
			if (_unused < 0)
			{
				SX1262_FUCK_THIS_AND_WAIT
				return 1;
			}
		}
		while(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_ACTIVE);
		SX1262_FUCK_THIS_AND_WAIT
		return 2;
	}

}

// this should be moved to sx1262 driver
static void fanet_reset(void)
{
	FANET_TURN_OFF();
	delay_fixed(10);
	FANET_TURN_ON();
	delay_fixed(10);
	fanet_wait_not_busy(10);
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void fanet_test_init(void)
{
	//!< Used across this file to configure I/O pins
	LL_GPIO_InitTypeDef GPIO_InitTypeDef;

	// INTERRUPT
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_6;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	// IS BUSY
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_7;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	// RESET output - A12
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_12;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);

	// keep RESET output hi-z
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);
}

/**
 * Only a test
 */
int fanet_test(void)
{
		int i = 1;
		int current_limit = 0;
		uint16_t interrupt_mask = 0;
#ifdef SX1262_IMPLEMENTATION
	   sx1262_status_chip_mode_t mode = SX1262_CHIP_MODE_UNINIT;
	   volatile sx1262_status_chip_mode_t initial_mode = SX1262_CHIP_MODE_UNINIT;
	   sx1262_status_last_command_t command_status = SX1262_LAST_COMMAND_UNINIT;
	   volatile sx1262_status_last_command_t initial_status = SX1262_LAST_COMMAND_UNINIT;
	   uint8_t errors;
	   volatile uint8_t initial_errors = 0xFF;

	   sx1262_rf_packet_type_t type;

	   fanet_wx_input_t wx_input = {0u};
	   wx_input.temperature = 23.3;

	   fanet_frame_t out = {0u};
	   out.source.id = 0x5000;
	   out.source.manufacturer = 0xEE;

	   volatile uint8_t regi = 0u;
	   volatile uint8_t waiting_res = 255u, additional_waiting_res = 255u;

	   volatile sx1262_api_return_t sx_result = SX1262_API_UNINIT;

//	   fanet_factory_frames_weather(19.0298f, 49.8277f, &wx_input, &out);
//	   const int fanet_ln = fanet_serialize(&out, (uint8_t*)fanet_test_array, 64u);

	   fanet_reset();

	   //io___cntrl_vbat_c_disable();
	   //fanet_wait_not_busy(300);
	   //io___cntrl_vbat_c_enable();

	   //sx_result = sx1262_status_get(&initial_mode, &initial_status);		// command 0xC0  -- response 0xA2, 0x22
	   sx_result = sx1262_status_get_device_errors(&initial_mode, &initial_status, &initial_errors);		// command 0x17 -- response 0xA2, 0xA2, 0x00, 0x20
	   SX1262_CHECK_NOK_RESULT(sx_result, -15);
	   waiting_res = fanet_wait_not_busy(0xFF);
	   sx_result = sx1262_irq_dio_get_mask(&interrupt_mask);	// command 0x12
	   waiting_res = fanet_wait_not_busy(0xFF);

	   sx_result = sx1262_modes_set_standby(0);						// opcode 0x80
	   waiting_res = fanet_wait_not_busy(0xFF);

	   sx_result = sx1262_irq_dio_enable_disable_on_pin_dio1(1, 1, 1, 1);  // command 0x08, 0x02, 0x43, 0x02 -- response 0xA2
	   SX1262_CHECK_NOK_RESULT(sx_result, -10);
	   waiting_res = fanet_wait_not_busy(0xFF);		// 150usec
	   sx_result = sx1262_irq_dio_set_dio2_as_rf_switch(1);			// command 0x9D, 0x01
	   SX1262_CHECK_NOK_RESULT(sx_result, -11);
	   waiting_res = fanet_wait_not_busy(0xFF);		// 15 msec
	   sx_result = sx1262_irq_dio_set_dio3_as_tcxo_ctrl(SX1262_IRQ_DIO_TCXO_VOLTAGE_3_3, 0xF000);
	   SX1262_CHECK_NOK_RESULT(sx_result, -12);
	   waiting_res = fanet_wait_not_busy(0xFF);
	   sx_result = sx1262_status_get_device_errors(&mode, &command_status, &errors);		// command 0x17 -- response 0xA2, 0xA2, 0x00, 0x20
	   SX1262_CHECK_NOK_RESULT(sx_result, -13);
	   waiting_res = fanet_wait_not_busy(0xFF);
//	   sx_result = sx1262_modes_set_regulator_mode(1);				// command 0x96
//	   SX1262_CHECK_NOK_RESULT(sx_result, -14);
//	   waiting_res = fanet_wait_not_busy(0xFFFF);		// 1200ms seems to be a minimum value
//	   SX1262_FUCK_THIS_AND_WAIT
//	   SX1262_FUCK_THIS_AND_WAIT
//	   SX1262_FUCK_THIS_AND_WAIT
//	   SX1262_FUCK_THIS_AND_WAIT
//	   SX1262_FUCK_THIS_AND_WAIT
//	   SX1262_FUCK_THIS_AND_WAIT
	   sx_result = sx1262_status_get(&mode, &command_status);		// command 0xC0  -- response 0xA2, 0x22
	   if (SX1262_API_OK == sx_result && mode == SX1262_CHIP_MODE_STDBY_RC/* && command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK*/) {
		   //sx1262_status_get_device_errors(&mode, &command_status, &errors);
		   //waiting_res = fanet_wait_not_busy(0xFFFF);

		   //sx1262_modes_set_calibrate_function(1, 1, 1, 1, 1, 1, 1);
		   sx1262_rf_packet_type(SX1262_RF_PACKET_TYPE_LORA);		// command 0x8A, 0x3
		   // 0xFF - this gives approx 100us of delay
		   waiting_res = fanet_wait_not_busy(0xFFFF);
		   sx_result = sx1262_rf_packet_type_get(&type);
		   if (type == SX1262_RF_PACKET_TYPE_LORA) {
			   waiting_res = fanet_wait_not_busy(0xF);
			   sx1262_rf_frequency(868500);
			   waiting_res = fanet_wait_not_busy(0xFFFF);
			   sx_result = sx1262_status_get(&mode, &command_status);
			   waiting_res = fanet_wait_not_busy(0xF);
			   if (command_status != SX1262_LAST_COMMAND_FAIL_TO_EXEC && sx_result == SX1262_API_OK) {
				   sx1262_modes_set_pa_config(5);
				   waiting_res = fanet_wait_not_busy(0xFFFF);
				   sx_result = sx1262_status_get(&mode, &command_status);
				   waiting_res = fanet_wait_not_busy(0xF);
				   sx_result = sx1262_rf_tx_params(14, SX1262_RF_TX_RAMPTIME_200US);
				   waiting_res = fanet_wait_not_busy(0xFFFF);
				   if (sx_result == SX1262_API_OK) {
					   sx_result = sx1262_status_get(&mode, &command_status);
					   fanet_wait_not_busy(0xFFFF);

//					   sx1262_data_io_write_register_byte(0x08E7u, 0x38u);
//					   sx1262_data_io_read_register(0x08E7u, 1u, &regi);

					   sx1262_rf_buffer_base_addresses(0, 128);
					   waiting_res = fanet_wait_not_busy(0xFFFF);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   waiting_res = fanet_wait_not_busy(0xFFFF);
					   sx1262_data_io_write_buffer(0, 7, (const uint8_t*)fanet_test_2);
					   waiting_res = fanet_wait_not_busy(0xF);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   waiting_res = fanet_wait_not_busy(0xF);
					   sx1262_rf_lora_modulation_params(SX1262_RF_LORA_SF7, SX1262_RF_LORA_BW250, SX1262_RF_LORA_CR_45, SX1262_RF_LORA_OPTIMIZE_OFF);
					   waiting_res = fanet_wait_not_busy(0xFFFF);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   waiting_res = fanet_wait_not_busy(0xF);
					   sx1262_rf_lora_packet_params(12, 7, SX1262_RF_LORA_HEADER_VARIABLE_LN_PACKET,1,0);
					   waiting_res = fanet_wait_not_busy(0xFFFF);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   waiting_res = fanet_wait_not_busy(0xFFFF);

					   if (command_status != SX1262_LAST_COMMAND_FAIL_TO_EXEC && sx_result == SX1262_API_OK) {
						   sx1262_data_io_write_register_byte(0x740, 0xF4u);
						   waiting_res = fanet_wait_not_busy(0xF);
						   sx1262_data_io_write_register_byte(0x741, 0x14u);
						   waiting_res = fanet_wait_not_busy(0xF);
						   sx1262_data_io_read_register(0x740, 1, (uint8_t*)&i);
						   waiting_res = fanet_wait_not_busy(0xFFFF);
						   sx1262_data_io_read_register(0x8E7, 1, (uint8_t*)&current_limit);
						   waiting_res = fanet_wait_not_busy(0xFFFF);
						   if (((uint8_t)(i & 0xFFu) == 0xF4u) && ((uint8_t)(current_limit & 0xFFu) == 0x38u)) {
							   sx1262_data_io_read_register(0x741, 1, (uint8_t*)&i);
							   if ((uint8_t)(i & 0xFFu) == 0x14u) {
		//						   sx1262_modes_set_standby(1);
		//						   sx1262_modes_set_fs();
								   waiting_res = fanet_wait_not_busy(0xFFFF);
								   const sx1262_api_return_t tx_res = sx1262_modes_set_tx(0x0);
		//						   sx1262_modes_set_tx_infinite_preamble();
								   //sx1262_modes_set_tx_cw();
								  // const sx1262_api_return_t tx_res = SX1262_API_OK;
								   waiting_res = fanet_wait_not_busy(0xFFFFFF);
								   sx_result = sx1262_irq_dio_get_mask(&interrupt_mask);	// command 0x12

								   if ((interrupt_mask & 0x1) != 0)
								   {
									   i = 0;
								   }
								   else
								   {
									   additional_waiting_res = fanet_wait_not_busy(0xFFFFFF);
									   sx_result = sx1262_irq_dio_get_mask(&interrupt_mask);	// command 0x12
								   }
									//while(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == 0U);
								   waiting_res = fanet_wait_not_busy(0xF);
								   sx_result = sx1262_status_get(&mode, &command_status);		// command: 0xC0
								   if (tx_res == SX1262_API_OK && command_status == SX1262_LAST_COMMAND_TX_DONE)
								   {
									   i = 0;
								   }
								   else
								   {
									   i = -1;
								   }

								   last_interrupt_mask = interrupt_mask;

								   waiting_res = fanet_wait_not_busy(0xF);
								   sx1262_irq_dio_clear_all();
								   waiting_res = fanet_wait_not_busy(0xF);

								   //sx1262_status_get_device_errors(&mode, &command_status, &errors);
								   //						   fanet_wait_not_busy(15);
								   //sx1262_data_io_write_register(start_address, data_ln, data)
							   }
							   else
							   {
								   i = -8;
							   }
						   }
						   else
						   {
							   i = -7;
						   }
					   }
					   else
					   {
						   i = -6;
					   }
				   }
				   else
				   {
					   i = -5;
				   }
			   }
			   else
			   {
				   i = -4;
			   }
		   }
		   else
		   {
			   i = -3;	// if (type == SX1262_RF_PACKET_TYPE_LORA)
		   }
	   }
	   else
	   {
		i = -2;	// if (mode == SX1262_CHIP_MODE_STDBY_RC && command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK)
	   }
#endif

nok:

	   if (i == 0) {
		   if ((interrupt_mask & 0x1) != 0)
		   {
			   led_blink_led2_botoom();
			   i = 0;
			   fanet_success_cnt++;
		   }
	   }
	   else
	   {
		   sx_result = SX1262_API_UNINIT;
		   fanet_fail_cnt++;
	   }

	   return i;
}

/**
 *
 * OK, nie mogę znaleźć, wyślij rekord numer 2 czyli "imię pilota", czyli będzie jakoś tak:

0x42 0x12 0x34 0x56 i potem wpisze sobie jakiekolwiek cokolwike w kodach ASCII

choćby 0x31 0x32 0x33 co oznacza 123

 *
 *
 *
 */

#endif

