/*
 * message.h
 *
 *  Created on: Apr 20, 2024
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_MESSAGE_H_
#define INCLUDE_APRS_MESSAGE_H_

#include "ax25_t.h"
#include "stdint.h"
#include "./stored_configuration_nvm/config_data.h"

#define MESSAGE_MAX_LENGHT	67

typedef struct message_t {
	AX25Call from;
	AX25Call to;
	uint8_t content[MESSAGE_MAX_LENGHT];
	uint8_t number;
}message_t;

typedef enum message_source_t {
	MESSAGE_SOURCE_UNINITIALIZED = 0x0,
	MESSAGE_SOURCE_APRSIS = 0x1,
	MESSAGE_SOURCE_APRSIS_HEXCNTR = 0x2,
	MESSAGE_SOURCE_RADIO = 0x11,
	MESSAGE_SOURCE_RADIO_HEXCNTR = 0x12
}message_source_t;

/**
 * Decode received data to look for an APRS message and put it into a structure
 * @param message pointer to data received from APRS-IS
 * @param message_ln lenght of a buffer content
 * @param content_position optional position of an APRS message content (recipient callsign). if set to zero function will look for it 
 * @param src 
 * @param output parsed APRS message content
 * @return zero if message has been found and decoded, non zero if parsing failed
 */
uint8_t message_decode(const uint8_t * const message, const uint16_t message_ln, uint16_t content_position, message_source_t src, message_t * output);

/**
 *
 * @param config_data
 * @param message
 * @return zero if this is a message to us, non zero otherwise
 */
uint8_t message_is_for_me(const char * const callsign, const uint8_t ssid, const message_t * const message);

/**
 *
 * @param out_buffer
 * @param out_buffer_ln
 * @param message
 * @param src how this message has been received
 */
int message_create_ack_for(uint8_t * out_buffer, const uint16_t out_buffer_ln, const message_t * const message, const message_source_t src);

#endif /* INCLUDE_APRS_MESSAGE_H_ */
