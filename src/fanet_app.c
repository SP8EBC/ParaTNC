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
#include "drivers/sx1262/sx1262.h"

#include "skytrax_fanet/fanet_factory_frames.h"
#include "skytrax_fanet/fanet_serialization.h"

#include "delay.h"
#include "io.h"
#include "LedConfig.h"

#include "main_freertos_externs.h"

#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>

#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SX1262_FUCK_THIS_AND_WAIT

								//  0x1FFFFE
#define FANET_DLY_TRANSMIT			2000
#define FANET_DLY_WRITE				200
#define FANET_DLY_READ				100

#define FANET_TURN_OFF()	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12)
#define FANET_TURN_ON()		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);

#define SX1262_CHECK_NOK_RESULT(res, code)	if(res != SX1262_API_OK) {			\
												i = code;						\
												goto nok;						\
											}									\


#define FANET_SX_WAIT_RES_TYPE		uint32_t
#define FANET_SX_WAIT_NUMBER		33
#define FANET_SX_RESULT_HISTORY_LN	12

#define FANET_SX_WHAT_TO_TRANSMIT		fanet_test_array
#define FANET_SX_WAHT_TO_TRANSMIT_LN	fanet_serialized_frame_out_ln

//#define FANET_SX_WHAT_TO_TRANSMIT		fanet_test_2
//#define FANET_SX_WAHT_TO_TRANSMIT_LN	strlen(fanet_test_2)

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

#ifdef SX1262_IMPLEMENTATION
static uint8_t fanet_test_array[64];

static const uint8_t fanet_test_2[7] = {0x42u, 0x12u, 0x34u, 0x56u, 0x31u, 0x32u, 0x33u, 0x00};

volatile static uint16_t last_interrupt_mask = 0;

volatile FANET_SX_WAIT_RES_TYPE fanet_wait_ret_history[FANET_SX_RESULT_HISTORY_LN][FANET_SX_WAIT_NUMBER];
uint8_t fanet_api_it_history = 0;

int  fanet_success_cnt = 0;
int  fanet_fail_cnt = 0;
int  fanet_tx_success_cnt = 0;

const fanet_mac_adress_t fanet_src = { .manufacturer = 0xDD, .id = 0x2233 };
const fanet_mac_adress_t fanet_dest = {0u}; //!< zero for broadcast

uint32_t fanet_serialized_frame_out_ln = 0;
fanet_frame_t fanet_frame_out = {0u};
const fanet_aircraft_t fanet_type = FANET_AIRCRAFT_PARAGLIDER;
const fanet_aircraft_stv_t fanet_stv = {
								.state = FANET_AIRCRAFT_STATUS_NEED_RIDE,
								.latitude = 49.7828f,
								.longitude = 19.0567f,
								.altitude = 512.0f,
								.speed = 1.0f,
								.climb = 0.5f,
								.heading = 128.0f,
								.turnrate = 1.0f,
								.qne_offset = 0.0f,
								.has_turnrate = 0
								};

volatile int fanet_i_value[FANET_SX_RESULT_HISTORY_LN] = {0xEEu};

extern volatile uint32_t sx1262_busy_counter;
volatile uint32_t fanet_last_busy_counter = 0xFFFFFFFFu;

#endif

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

// this also should be moved to sx1262 driver
/**
 *
 * @param _unused
 * @return  0x0 < x < 0x7FFFFFFF 		if at the call modem was busy and became idle after waiting
 * 			0xFFFFFFFF 					if the call modem was idle and wait for busy condition timeouted
 * 			0x80000000 < x < 0xFFFFFFFF if at the call modem was idle and then became busy for some time
 */
static uint32_t fanet_wait_not_busy(int timeoutMilisecond)
{
	uint32_t out = 0;

	// sane defaults
	if (timeoutMilisecond <= 0)
	{
		timeoutMilisecond = 100;
	}

#ifdef SX1262_IMPLEMENTATION
	fanet_last_busy_counter = sx1262_busy_counter;

	EventBits_t bits = xEventGroupWaitBits (main_eventgroup_handle_sx1262,
											MAIN_EVENTGROUP_SX1262_ISBUSY,
											pdTRUE,
											pdTRUE,
											timeoutMilisecond / portTICK_PERIOD_MS);

	if (bits == 0)
	{
		out = 0xFFFFFFFF;
	}
	else
	{
		out = (uint32_t) bits;
	}
//	while (fanet_last_busy_counter <= sx1262_busy_counter)
//	{
//		out++;
//
//		if (out >= 0x3FFFFF)
//		{
//			break;
//		}
//	}
#endif

	return out;

}



// this should be moved to sx1262 driver
static void fanet_reset(void)
{
	FANET_TURN_OFF();
	delay_fixed(10);
	FANET_TURN_ON();
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void fanet_test_init(void)
{
#ifdef SX1262_IMPLEMENTATION

	memset(fanet_wait_ret_history, 0x00, sizeof(FANET_SX_WAIT_RES_TYPE) * FANET_SX_WAIT_NUMBER * FANET_SX_RESULT_HISTORY_LN);

	const fanet_wx_input_t fanet_wx = {
			.temperature = 22,
			.wind_direction = 90,
			.wind_average_speed = 12,
			.wind_gusts = 34,
			.humidity = 70,
			.qnh = 10130,
	};
	const uint32_t frame_out_ln = fanet_factory_frames_weather(49.7828f, 19.0567f, &fanet_wx, &fanet_frame_out);
	//const uint32_t frame_out_ln =  fanet_factory_frames_tracking (fanet_type, &fanet_stv, &fanet_frame_out);
	fanet_frame_out.payload_length = frame_out_ln;
	fanet_frame_out.type = FANET_FRAME_SERVICE;
	fanet_frame_out.source = fanet_src;
	fanet_frame_out.destination = fanet_dest;

	FANET_SX_WAHT_TO_TRANSMIT_LN = fanet_serialize(&fanet_frame_out, FANET_SX_WHAT_TO_TRANSMIT, 64);
#endif
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

	   sx1262_rf_packet_type_t type = SX1262_RF_PACKET_TYPE_UNINIT;
	   
	   volatile sx1262_api_return_t sx_result = SX1262_API_UNINIT;

	   fanet_reset();

	   //io___cntrl_vbat_c_disable();
	   //fanet_wait_not_busy(300);
	   //io___cntrl_vbat_c_enable();

	   fanet_wait_ret_history[fanet_api_it_history][0] = fanet_wait_not_busy(FANET_DLY_WRITE);		// 0x22d8

	   sx_result = sx1262_modes_set_standby(0);						// opcode 0x80
	   fanet_wait_ret_history[fanet_api_it_history][3] = fanet_wait_not_busy(FANET_DLY_WRITE);		// 0x7
	   SX1262_CHECK_NOK_RESULT(sx_result, -12);
	   ////
	   // 10. Configure DIO and IRQ: use the command SetDioIrqParams(...) to select TxDone IRQ and map this IRQ to a DIO (DIO1,
	   sx_result = sx1262_irq_dio_enable_disable_on_pin_dio1(1, 1, 1, 1);  // command 0x08, 0x02, 0x43, 0x02 -- response 0xA2
	   SX1262_CHECK_NOK_RESULT(sx_result, -13);
	   ////
	   sx_result = sx1262_irq_dio_set_dio2_as_rf_switch(1);			// command 0x9D, 0x01
	   fanet_wait_ret_history[fanet_api_it_history][5] = fanet_wait_not_busy(FANET_DLY_WRITE);		// 15 msec	0x7
	   SX1262_CHECK_NOK_RESULT(sx_result, -14);
	   ////
	   sx_result = sx1262_irq_dio_set_dio3_as_tcxo_ctrl(SX1262_IRQ_DIO_TCXO_VOLTAGE_3_3, 0xF000);
	   fanet_wait_ret_history[fanet_api_it_history][6] = fanet_wait_not_busy(FANET_DLY_WRITE);		// 0x21
	   SX1262_CHECK_NOK_RESULT(sx_result, -15);
	   ////
	   sx_result = sx1262_status_get(&mode, &command_status);		// command 0xC0  -- response 0xA2, 0x22
	   SX1262_CHECK_NOK_RESULT(sx_result, -17);
	   if (SX1262_API_OK == sx_result && mode == SX1262_CHIP_MODE_STDBY_RC && command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK) {
		   // 2. Define the protocol (LoRa® or FSK) with the command SetPacketType(...)
		   sx1262_rf_packet_type(SX1262_RF_PACKET_TYPE_LORA);		// command 0x8A, 0x3
		   fanet_wait_ret_history[fanet_api_it_history][8] = fanet_wait_not_busy(FANET_DLY_WRITE);	// 0x2B
		   ////
		   sx_result = sx1262_rf_packet_type_get(&type);
		   ////
		   if (type == SX1262_RF_PACKET_TYPE_LORA && sx_result == SX1262_API_OK) {
			   // 3. Define the RF frequency with the command SetRfFrequency(...)
			   sx1262_rf_frequency(868200);
			   fanet_wait_ret_history[fanet_api_it_history][10] = fanet_wait_not_busy(FANET_DLY_WRITE);	// 0x9a
			   ////
			   sx_result = sx1262_status_get(&mode, &command_status);
			   if (command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK && sx_result == SX1262_API_OK) {
				   // 4. Define the Power Amplifier configuration with the command SetPaConfig(...)
				   sx1262_modes_set_pa_config(5);
				   fanet_wait_ret_history[fanet_api_it_history][12] = fanet_wait_not_busy(FANET_DLY_WRITE);	// 0x20
				   ////
				   // 5. Define output power and ramping time with the command SetTxParams(...)
				   sx_result = sx1262_rf_tx_params(14, SX1262_RF_TX_RAMPTIME_200US);
				   fanet_wait_ret_history[fanet_api_it_history][14] = fanet_wait_not_busy(FANET_DLY_WRITE);	// 0x13
				   ////
				   if (sx_result == SX1262_API_OK) {
					   // 6. Define where the data payload will be stored with the command SetBufferBaseAddress(...)
					   sx1262_rf_buffer_base_addresses(0, 128);
					   fanet_wait_ret_history[fanet_api_it_history][15] = fanet_wait_not_busy(FANET_DLY_WRITE);	// 0xD
					   ////
					   // 7. Send the payload to the data buffer with the command WriteBuffer(...)
					   sx1262_data_io_write_buffer(0, FANET_SX_WAHT_TO_TRANSMIT_LN, (const uint8_t*)FANET_SX_WHAT_TO_TRANSMIT);
					   fanet_wait_ret_history[fanet_api_it_history][15] = fanet_wait_not_busy(FANET_DLY_WRITE);	// 0xD
					   ////
					   sx_result = sx1262_status_get(&mode, &command_status);
					   if (command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK && sx_result == SX1262_API_OK) {
						   // 8. Define the modulation parameter according to the chosen protocol with the command SetModulationParams(...)1
						   // These parameters are set using the command SetModulationParams(...) which must be called after SetPacketType(...).
						   sx1262_rf_lora_modulation_params(SX1262_RF_LORA_SF7, SX1262_RF_LORA_BW250, SX1262_RF_LORA_CR_45, SX1262_RF_LORA_OPTIMIZE_OFF);
						   fanet_wait_ret_history[fanet_api_it_history][19] = fanet_wait_not_busy(FANET_DLY_WRITE);		// 0x27
						   ////
						   sx_result = sx1262_status_get(&mode, &command_status);
						   if (command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK && sx_result == SX1262_API_OK) {
							   // 9. Define the frame format to be used with the command SetPacketParams(...)2
							   sx1262_rf_lora_packet_params(5, FANET_SX_WAHT_TO_TRANSMIT_LN, SX1262_RF_LORA_HEADER_VARIABLE_LN_PACKET,1,0);
							   fanet_wait_ret_history[fanet_api_it_history][21] = fanet_wait_not_busy(FANET_DLY_WRITE);	// 0x27
							   ////
							   sx_result = sx1262_status_get(&mode, &command_status);
							   if (command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK && sx_result == SX1262_API_OK) {
								   // 11. Define Sync Word value: use the command WriteReg(...) to write the value of the register via direct register access
								   sx1262_data_io_write_register_byte(0x740, 0xF4u);			// Lora sync word MSB
								   sx1262_data_io_write_register_byte(0x741, 0x14u);			// Lora sync word LSB
								   sx1262_data_io_read_register(0x740, 1, (uint8_t*)&i);			// read this sync word MSB back for verification
								   sx1262_data_io_read_register(0x8E7, 1, (uint8_t*)&current_limit);
								   ////
								   if (((uint8_t)(i & 0xFFu) == 0xF4u) && ((uint8_t)(current_limit & 0xFFu) == 0x38u)) {
									   sx1262_data_io_read_register(0x741, 1, (uint8_t*)&i);		// read this sync word LSB back for verification
									   if ((uint8_t)(i & 0xFFu) == 0x14u) {
										   // 12. Set the circuit in transmitter mode to start transmission with the command SetTx(). Use the parameter to enable
										   const sx1262_api_return_t tx_res = sx1262_modes_set_tx(0x0);	// TRANSMIT HERE
										   fanet_wait_ret_history[fanet_api_it_history][28] = fanet_wait_not_busy(FANET_DLY_TRANSMIT);	// 0x32eaf5
										   ////
										   sx_result = sx1262_irq_dio_get_mask(&interrupt_mask);	// command 0x12

										   if (sx1262_is_interrrupt_flag_active() && (interrupt_mask & 0x1) != 0)
										   {
											   fanet_tx_success_cnt ++;
											   i = 0;
											   fanet_wait_ret_history[fanet_api_it_history][FANET_SX_WAIT_NUMBER - 1] = 0x80000000;
										   }
										   else
										   {
											   fanet_wait_ret_history[fanet_api_it_history][FANET_SX_WAIT_NUMBER - 1] = fanet_wait_not_busy(FANET_DLY_TRANSMIT);
											   sx_result = sx1262_irq_dio_get_mask(&interrupt_mask);	// command 0x12
											   if (sx1262_is_interrrupt_flag_active() && (interrupt_mask & 0x1) != 0)
											   {
												   fanet_tx_success_cnt ++;
											   }
										   }
										   sx_result = sx1262_status_get(&mode, &command_status);		// command: 0xC0
										   if (tx_res == SX1262_API_OK && command_status == SX1262_LAST_COMMAND_TX_DONE)
										   {
											   i = 0;
										   }
										   else
										   {
											   i = -1;
										   }

										   sx1262_irq_dio_clear_all();
										   sx_result = sx1262_irq_dio_get_mask(&last_interrupt_mask);	// command 0x12

										   sx1262_status_get_device_errors(&mode, &command_status, &errors);
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
							   i = -21;
						   }
						   //////////
					   }
					   else
					   {
						   i = -20;
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

nok:

	   if (i == 0) {
		   if ((interrupt_mask & 0x1) != 0)
		   {
			   led_blink_led2_botoom();
			   i = 0;
			   fanet_success_cnt++;
			   //fanet_tx_success_cnt++;
		   }
		   else
		   {
			   fanet_fail_cnt++;
		   }
	   }
	   else
	   {
		   sx_result = SX1262_API_UNINIT;
		   fanet_fail_cnt++;
	   }

	   fanet_i_value[fanet_api_it_history] = i;

	   fanet_api_it_history++;
	   if (fanet_api_it_history >= FANET_SX_RESULT_HISTORY_LN) {
		   fanet_api_it_history = 0;

	   }
#endif

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

