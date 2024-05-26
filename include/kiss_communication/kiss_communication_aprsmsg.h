/*
 * kiss_communication_aprsmsg.h
 *
 *  Created on: May 5, 2024
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_KISS_COMMUNICATION_APRSMSG_H_
#define KISS_COMMUNICATION_KISS_COMMUNICATION_APRSMSG_H_

#include <stdint.h>
#include "kiss_communication/types/kiss_communication_transport_t.h"

/**
 *
 * @param message_payload
 * @param message_payload_ln
 * @return
 */
kiss_communication_transport_t kiss_communication_aprsmsg_check_type(uint8_t * message_payload, uint16_t message_payload_ln);

/**
 *
 * @param message_payload pointer to characters buffer with APRS text message, containing encoded UDS diag request
 * @param message_payload_ln length of buffer pointed by message_payload. processing will be done until first non-hex character (non 0-9 / a-f) or this length
 * @param output_binary_buffer pointer to binary buffer where decoded data will be copied to.
 * @param output_ln
 * @return length of decoded UDS request or zero if message_payload doesn't contain valid hexstring with UDS request
 */
uint16_t kiss_communication_aprsmsg_decode_hexstring(uint8_t * message_payload, uint16_t message_payload_ln, uint8_t * output_binary_buffer, uint16_t output_ln);

/**
 *
 * @param input_binary_buffer
 * @param input_ln
 * @param output_message
 * @param output_message_max_ln
 * @return
 */
uint16_t kiss_communication_aprsmsg_encode_hexstring(uint8_t * input_binary_buffer, uint16_t input_ln, uint8_t * output_message, uint16_t output_message_max_ln);


#endif /* KISS_COMMUNICATION_KISS_COMMUNICATION_APRSMSG_H_ */
