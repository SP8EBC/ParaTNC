/*
 * rtu_serial_io.h
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_SERIAL_IO_H_
#define INCLUDE_MODBUS_RTU_RTU_SERIAL_IO_H_

#include <configuration_nvm/config_data.h>
#include <stdint.h>

#include "./drivers/serial.h"

#include "./modbus_rtu/rtu_pool_queue_t.h"


extern uint16_t rtu_serial_previous_crc;

inline void rtu_serial_reset_crc(void) {
	rtu_serial_previous_crc = 0xFFFF;
}

uint8_t rtu_serial_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter);

int32_t rtu_serial_init(rtu_pool_queue_t* queue, uint8_t io_mode, srl_context_t* serial_context, const config_data_rtu_t * const config);
int32_t rtu_serial_pool(void);
int32_t rtu_serial_blocking_io(srl_context_t* serial_context, uint8_t query_ln);
int32_t rtu_serial_start(void);

int32_t rtu_serial_get_status_string(rtu_pool_queue_t* queue, srl_context_t* srl_ctx, char* out, uint16_t out_buffer_ln, uint8_t* generated_string_ln);


#endif /* INCLUDE_MODBUS_RTU_RTU_SERIAL_IO_H_ */
