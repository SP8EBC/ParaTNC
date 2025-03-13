/*
 * fanet_app.c
 *
 *  Created on: Feb 28, 2025
 *      Author: mateusz
 */

#include "fanet_app.h"

#include "drivers/sx1262/sx1262_modes.h"
#include "drivers/sx1262/sx1262_status.h"
#include "drivers/sx1262/sx1262_rf.h"
#include "drivers/sx1262/sx1262_irq_dio.h"
#include "drivers/sx1262/sx1262_data_io.h"

#include "skytrax_fanet/fanet_factory_frames.h"
#include "skytrax_fanet/fanet_serialization.h"

#include "delay.h"
#include "io.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

#ifdef SX1262_IMPLEMENTATION
static uint8_t fanet_test_array[64];

static const uint8_t * fanet_test_2 = {0x42u, 0x12u, 0x34u, 0x56u, 0x31u, 0x32u, 0x33u};
#endif

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Only a test
 */
int fanet_test(void)
{
		int i = 1;
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
	   //delay_fixed(300);
	   //io___cntrl_vbat_c_enable();

	   sx1262_modes_set_standby(0);
	   delay_fixed(300);

	   sx1262_irq_dio_enable_disable_on_pin_dio1(1, 1, 1, 1);
	   delay_fixed(10);
	   sx1262_irq_dio_set_dio2_as_rf_switch(1);
	   delay_fixed(10);
	   sx1262_irq_dio_set_dio3_as_tcxo_ctrl(SX1262_IRQ_DIO_TCXO_VOLTAGE_3_3, 0xF000);
	   delay_fixed(10);
	   sx1262_status_get_device_errors(&mode, &command_status, &errors);
	   delay_fixed(10);
	   sx1262_modes_set_regulator_mode(1);
	   delay_fixed(1300);		// 1200ms seems to be a minimum value
	   sx1262_status_get(&mode, &command_status);
	   if (mode == SX1262_CHIP_MODE_STDBY_RC && command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK) {
		   //sx1262_status_get_device_errors(&mode, &command_status, &errors);
		   delay_fixed(10);

		   //sx1262_modes_set_calibrate_function(1, 1, 1, 1, 1, 1, 1);
		   sx1262_rf_packet_type(SX1262_RF_PACKET_TYPE_LORA);
		   // 0xFF - this gives approx 100us of delay
		   delay_fixed(10);
		   sx1262_rf_packet_type_get(&type);
		   if (type == SX1262_RF_PACKET_TYPE_LORA) {
			   delay_fixed(10);
			   sx1262_rf_frequency(868500);
			   delay_fixed(10);
			   sx_result = sx1262_status_get(&mode, &command_status);
			   delay_fixed(10);
			   if (command_status != SX1262_LAST_COMMAND_FAIL_TO_EXEC && sx_result == SX1262_API_OK) {
				   sx1262_modes_set_pa_config(5);
				   delay_fixed(10);
				   sx_result = sx1262_status_get(&mode, &command_status);
				   delay_fixed(10);
				   sx_result = sx1262_rf_tx_params(14, SX1262_RF_TX_RAMPTIME_200US);
				   delay_fixed(10);
				   if (sx_result == SX1262_API_OK) {
					   sx_result = sx1262_status_get(&mode, &command_status);
					   delay_fixed(300);

//					   sx1262_data_io_write_register_byte(0x08E7u, 0x38u);
//					   sx1262_data_io_read_register(0x08E7u, 1u, &regi);

					   sx1262_rf_buffer_base_addresses(0, 128);
					   delay_fixed(10);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   delay_fixed(10);
					   sx1262_data_io_write_buffer(0, 7, (const uint8_t*)fanet_test_2);
					   delay_fixed(10);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   delay_fixed(10);
					   sx1262_rf_lora_modulation_params(SX1262_RF_LORA_SF7, SX1262_RF_LORA_BW250, SX1262_RF_LORA_CR_45, SX1262_RF_LORA_OPTIMIZE_OFF);
					   delay_fixed(10);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   delay_fixed(10);
					   sx1262_rf_lora_packet_params(12, 7, SX1262_RF_LORA_HEADER_VARIABLE_LN_PACKET,1,0);
					   delay_fixed(10);
					   sx_result = sx1262_status_get(&mode, &command_status);
					   delay_fixed(10);

					   if (command_status != SX1262_LAST_COMMAND_FAIL_TO_EXEC && sx_result == SX1262_API_OK) {
						   sx1262_data_io_write_register_byte(0x740, 0xF4u);
						   delay_fixed(300);
						   sx1262_data_io_write_register_byte(0x741, 0x14u);
						   delay_fixed(300);
						   sx1262_data_io_read_register(0x740, 1, (uint8_t*)&i);
						   delay_fixed(10);
						   if ((uint8_t)(i & 0xFFu) == 0xF4u) {
							   sx1262_data_io_read_register(0x741, 1, (uint8_t*)&i);
							   if ((uint8_t)(i & 0xFFu) == 0x14u) {
		//						   sx1262_modes_set_standby(1);
		//						   sx1262_modes_set_fs();
								   delay_fixed(1200);
//								   sx_result = sx1262_status_get(&mode, &command_status);
//								   sx1262_status_get_device_errors(&mode, &command_status, &errors);
								   const sx1262_api_return_t tx_res = sx1262_modes_set_tx(0x12345);
		//						   sx1262_modes_set_tx_infinite_preamble();
		//						   sx1262_modes_set_tx_cw();
								   delay_fixed(1200);
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
								   //sx1262_status_get_device_errors(&mode, &command_status, &errors);
								   //						   delay_fixed(15);
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
		   i = 0;
	   }
	   else
	   {
		   i = 2;
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

