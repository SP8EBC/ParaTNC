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

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

#ifdef SX1262_IMPLEMENTATION
static uint8_t fanet_test_array[64];

static const uint8_t fanet_test_2[7] = {0x42u, 0x12u, 0x34u, 0x56u, 0x31u, 0x32u, 0x33u};
#endif

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

static void fanet_wait_not_busy(int _unused)
{
	(void)_unused;
	// PC6
	if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_ACTIVE)
	{
		while(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_ACTIVE);
	}
	else
	{
		while(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_NOTACTIVE)
		{
			_unused--;
			if (_unused < 0)
			{
				break;
			}
		}
		while(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_ACTIVE);
	}
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
	   sx1262_status_chip_mode_t mode;
	   sx1262_status_last_command_t command_status;
	   uint8_t errors;

	   sx1262_rf_packet_type_t type;

	   fanet_wx_input_t wx_input = {0u};
	   wx_input.temperature = 23.3;

	   fanet_frame_t out = {0u};
	   out.source.id = 0x5000;
	   out.source.manufacturer = 0xEE;

	   volatile uint8_t regi = 0u;

	   volatile sx1262_api_return_t sx_result = SX1262_API_UNINIT;

//	   fanet_factory_frames_weather(19.0298f, 49.8277f, &wx_input, &out);
//	   const int fanet_ln = fanet_serialize(&out, (uint8_t*)fanet_test_array, 64u);



	   //io___cntrl_vbat_c_disable();
	   //fanet_wait_not_busy(300);
	   //io___cntrl_vbat_c_enable();

	   sx1262_modes_set_standby(0);
	   fanet_wait_not_busy(300);

	   sx1262_irq_dio_enable_disable_on_pin_dio1(1, 1, 1, 1);
	   fanet_wait_not_busy(10);
	   sx1262_irq_dio_set_dio2_as_rf_switch(1);
	   fanet_wait_not_busy(10);
	   sx1262_irq_dio_set_dio3_as_tcxo_ctrl(SX1262_IRQ_DIO_TCXO_VOLTAGE_3_3, 0xF000);
	   fanet_wait_not_busy(10);
	   sx1262_status_get_device_errors(&mode, &command_status, &errors);
	   fanet_wait_not_busy(10);
	   sx1262_modes_set_regulator_mode(1);
	   fanet_wait_not_busy(1300);		// 1200ms seems to be a minimum value
	   sx1262_status_get(&mode, &command_status);
	   if (mode == SX1262_CHIP_MODE_STDBY_RC && command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK) {
		   //sx1262_status_get_device_errors(&mode, &command_status, &errors);
		   fanet_wait_not_busy(10);

		   //sx1262_modes_set_calibrate_function(1, 1, 1, 1, 1, 1, 1);
		   sx1262_rf_packet_type(SX1262_RF_PACKET_TYPE_LORA);
		   // 0xFF - this gives approx 100us of delay
		   fanet_wait_not_busy(10);
		   sx1262_rf_packet_type_get(&type);
		   if (type == SX1262_RF_PACKET_TYPE_LORA) {
			   fanet_wait_not_busy(10);
			   sx1262_rf_frequency(868500);
			   fanet_wait_not_busy(10);
			   sx_result = sx1262_status_get(&mode, &command_status);
			   fanet_wait_not_busy(10);
			   if (command_status != SX1262_LAST_COMMAND_FAIL_TO_EXEC && sx_result == SX1262_API_OK) {
				   sx1262_modes_set_pa_config(5);
				   fanet_wait_not_busy(10);
				   sx_result = sx1262_status_get(&mode, &command_status);
				   fanet_wait_not_busy(10);
				   sx_result = sx1262_rf_tx_params(14, SX1262_RF_TX_RAMPTIME_200US);
				   fanet_wait_not_busy(10);
				   if (sx_result == SX1262_API_OK) {
					   sx_result = sx1262_status_get(&mode, &command_status);
					   fanet_wait_not_busy(300);

//					   sx1262_data_io_write_register_byte(0x08E7u, 0x38u);
//					   sx1262_data_io_read_register(0x08E7u, 1u, &regi);

					   sx1262_rf_buffer_base_addresses(0, 128);
					   fanet_wait_not_busy(10);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   fanet_wait_not_busy(10);
					   sx1262_data_io_write_buffer(0, 7, (const uint8_t*)fanet_test_2);
					   fanet_wait_not_busy(10);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   fanet_wait_not_busy(10);
					   sx1262_rf_lora_modulation_params(SX1262_RF_LORA_SF7, SX1262_RF_LORA_BW250, SX1262_RF_LORA_CR_45, SX1262_RF_LORA_OPTIMIZE_OFF);
					   fanet_wait_not_busy(10);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   fanet_wait_not_busy(10);
					   sx1262_rf_lora_packet_params(12, 7, SX1262_RF_LORA_HEADER_VARIABLE_LN_PACKET,1,0);
					   fanet_wait_not_busy(10);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   fanet_wait_not_busy(10);

					   if (command_status != SX1262_LAST_COMMAND_FAIL_TO_EXEC && sx_result == SX1262_API_OK) {
						   sx1262_data_io_write_register_byte(0x740, 0xF4u);
						   fanet_wait_not_busy(300);
						   sx1262_data_io_write_register_byte(0x741, 0x14u);
						   fanet_wait_not_busy(300);
						   sx1262_data_io_read_register(0x740, 1, (uint8_t*)&i);
						   fanet_wait_not_busy(10);
						   sx1262_data_io_read_register(0x8E7, 1, (uint8_t*)&current_limit);
						   fanet_wait_not_busy(10);
						   if (((uint8_t)(i & 0xFFu) == 0xF4u) && ((uint8_t)(current_limit & 0xFFu) == 0x38u)) {
							   sx1262_data_io_read_register(0x741, 1, (uint8_t*)&i);
							   if ((uint8_t)(i & 0xFFu) == 0x14u) {
		//						   sx1262_modes_set_standby(1);
		//						   sx1262_modes_set_fs();
								   fanet_wait_not_busy(1200);
								   const sx1262_api_return_t tx_res = sx1262_modes_set_tx(0x0);
		//						   sx1262_modes_set_tx_infinite_preamble();
								   //sx1262_modes_set_tx_cw();
								  // const sx1262_api_return_t tx_res = SX1262_API_OK;
								   fanet_wait_not_busy(1200);
									while(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == 0U);
								   sx_result = sx1262_status_get(&mode, &command_status);
								   if (tx_res == SX1262_API_OK && command_status == SX1262_LAST_COMMAND_TX_DONE)
								   {
									   i = 0;
								   }
								   else
								   {
									   i = -1;
								   }

								   sx_result = sx1262_irq_dio_get_mask(&interrupt_mask);

								   sx1262_irq_dio_clear_all();
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
			   i = -3;
		   }
	   }
	   else
	   {
		i = -2;
	   }
#endif

	   if ((interrupt_mask & 0x1) != 0)
	   {
		   led_blink_led2_botoom();
		   i = 0;
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

