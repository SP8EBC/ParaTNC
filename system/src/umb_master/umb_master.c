/*
 * umb_client.c
 *
 *  Created on: 22.02.2020
 *      Author: mateusz
 */

#include <string.h>
#include <umb_master/umb_0x26_status.h>
#include <umb_master/umb_master.h>

#ifdef _UMB_MASTER

#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04

#define V10 0x10

#define MASTER_ID 0x01
#define MASTER_CLASS 0xF0

umb_context_t umb_context;

void umb_master_init() {
	umb_context.current_routine = -1;
	umb_context.state = UMB_STATUS_IDLE;
}

umb_retval_t umb_parse_serial_buffer_to_frame(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame) {

	uint16_t crc = 0xFFFFu;
	uint16_t crc_from_frame;

	if (serial_buffer[0] != SOH && serial_buffer[1] != V10)
		return UMB_NOT_VALID_FRAME;

	if (serial_buffer[2] != MASTER_CLASS && serial_buffer[3] != MASTER_ID)
		return UMB_TO_ANOTHER_MASTER;

	frame->slave_class 		= serial_buffer[4];
	frame->slave_id 		= serial_buffer[5];
	frame->lenght 			= serial_buffer[6] - 2;

	if (serial_buffer[8] != STX)
		return UMB_NOT_VALID_FRAME;

	frame->command_id		= serial_buffer[8];
	frame->protocol_version = serial_buffer[9];

	// checking if payload isn't too big to fit into structure
	if (frame->lenght >= UMB_FRAME_MAX_PAYLOAD_LN)
		return UMB_RECV_FRAME_TOO_LONG;

	// Copying payload of the frame from a serial buffer
	for (int i = 0; (i < frame->lenght && i < buffer_ln); i++) {
		frame->payload[i] = serial_buffer[10 + i];
	}

	// Retrieving the crc value from frame
	crc_from_frame = serial_buffer[frame->lenght + 9] | (serial_buffer[frame->lenght + 10] << 8);

	// recalculating crc from frame content
	for (int j = 0; j < frame->lenght + 8 + 2; j++) {
		crc = umb_calc_crc(crc, serial_buffer[j]);
	}

	frame->calculated_checksum_lsb = crc & 0xFF;
	frame->calculated_checksum_msb = (crc & 0xFF00) >> 8;

	if (	serial_buffer[frame->lenght + 9 + 2] != frame->calculated_checksum_lsb ||
			serial_buffer[frame->lenght + 10 + 2] != frame->calculated_checksum_msb)
		return UMB_WRONG_CRC;

	return UMB_OK;
}

umb_retval_t umb_parse_frame_to_serial_buffer(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame) {

	int i = 0;
	uint16_t crc = 0xFFFFu;

	if (frame->lenght - 2 >= buffer_ln)
		return UMB_FRAME_TOO_LONG_FOR_TX;

	memset(serial_buffer, 0x00, buffer_ln);

	serial_buffer[i++] = SOH;
	serial_buffer[i++] = V10;
	serial_buffer[i++] = _UMB_SLAVE_ID;
	serial_buffer[i++] = _UMB_SLAVE_CLASS;
	serial_buffer[i++] = MASTER_ID;
	serial_buffer[i++] = MASTER_CLASS;
	serial_buffer[i++] = frame->lenght + 2;
	serial_buffer[i++] = STX;
	serial_buffer[i++] = frame->command_id;
	serial_buffer[i++] = V10;

	for (int j = 0; j < frame->lenght; j++) {
		serial_buffer[i++] = frame->payload[j];
	}

	serial_buffer[i++] = ETX;

	for (int j = 0; j < i; j++) {
		crc = umb_calc_crc(crc, serial_buffer[j]);
	}

	serial_buffer[i++] = (uint8_t) crc & 0xFF;
	serial_buffer[i++] = (uint8_t) (crc & 0xFF00) >> 8;
	serial_buffer[i++] = EOT;

	return UMB_OK;
}

uint16_t umb_calc_crc(uint16_t crc_buff, uint8_t input) {
	uint8_t i;
	uint16_t x16;
	for	(i=0; i<8; i++)
	{
		// XOR current D0 and next input bit to determine x16 value
		if		( (crc_buff & 0x0001) ^ (input & 0x01) )
			x16 = 0x8408;
		else
			x16 = 0x0000;
		// shift crc buffer
		crc_buff = crc_buff >> 1;
		// XOR in the x16 value
		crc_buff ^= x16;
		// shift input for next iteration
		input = input >> 1;
	}
	return (crc_buff);
}

/**
 * This function is called globally after receiving
 */
umb_retval_t umb_master_callback(umb_frame_t* frame) {

	// looking for a callback to this response
	switch (frame->command_id) {
		case 0x26: {
			umb_0x26_status_callback(frame);
			break;
		}
	}

	return UMB_OK;
}

#endif
