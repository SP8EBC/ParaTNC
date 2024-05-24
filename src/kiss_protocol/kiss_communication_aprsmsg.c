/*
 * kiss_communication_aprsmsg.c
 *
 *  Created on: May 5, 2024
 *      Author: mateusz
 */


#include "kiss_communication/kiss_communication_aprsmsg.h"

#define KISS_COMMUNICATION_APRSMSG_DEC_CURRENT_CHAR     *(message_payload + payload_it)

#define KISS_COMMUNICATION_APRSMSG_DEC_NEXT_CHAR        *(message_payload + payload_it + 1)

#define KISS_COMMUNICATION_APRSMSG_DEC_OUTPUT_IT        ((payload_it - 1) / 2)

#define KISS_COMMUNICATION_APRSMSG_IS_DIGIT(c)          ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))

#define KISS_COMMUNICATION_APRSMSG_ENC_OUTPUT_L_IT(i)   (((i + 1) * 2) + 1) 

#define KISS_COMMUNICATION_APRSMSG_ENC_OUTPUT_H_IT(i)   ((i + 1) * 2)

#define KISS_COMMUNICATION_APRSMSG_GET_CHAR(b)          kiss_communication_aprsmsg_tohexstr_lookup_table[b]

#define KISS_COMMUNICATION_APRSMSG_IS_HEXSTRING(s)      ((*(s) == 'H') && (*(s + 1) == 'S'))

#define KISS_COMMUNICATION_APRSMSG_IS_ENCR_HEXSTRING(s) (*(s) == 'P')

static uint8_t kiss_communication_aprsmsg_lookup_table[] = 
                    {   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 
                        -1, -1, -1, -1, -1, -1, -1,
                        10, 11, 12, 13, 14, 15,
                        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1,
                        10, 11, 12, 13, 14, 15};

static uint8_t kiss_communication_aprsmsg_tohexstr_lookup_table[] = 
                    {
                        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
                    };

/**
 *
 * @param message_payload
 * @param message_payload_ln
 * @return
 */
kiss_communication_transport_t kiss_communication_aprsmsg_check_type(uint8_t * message_payload, uint16_t message_payload_ln)
{
    kiss_communication_transport_t out = KISS_TRANSPORT_UNINITIALIZED;

    if (variant_validate_is_within_ram(message_payload) == 1 && message_payload_ln > 6 && message_payload_ln <= 67) {
        if (KISS_COMMUNICATION_APRSMSG_IS_HEXSTRING(message_payload)) {
            out = KISS_TRANSPORT_HEXSTRING;
        }
        else if (KISS_COMMUNICATION_APRSMSG_IS_ENCR_HEXSTRING(message_payload)) {
            out = KISS_TRANSPORT_ERROR_UNSUPPORTED;
        }
        else {
            out = KISS_TRANSPORT_NOT_KISS;
        }
    }   

    return out;
}

/**
 *
 * @param message_payload pointer to characters buffer with APRS text message, containing encoded UDS diag request
 * @param message_payload_ln length of buffer pointed by message_payload. processing will be done until first non-hex character (non 0-9 / a-f) or this length
 * @param output_binary_buffer pointer to binary buffer where decoded data will be copied to.
 * @param output_ln
 * @return length of decoded UDS request or zero if message_payload doesn't contain valid hexstring with UDS request
 */
uint16_t kiss_communication_aprsmsg_decode_hexstring(uint8_t * message_payload, uint16_t message_payload_ln, uint8_t * output_binary_buffer, uint16_t output_ln)
{
    uint8_t out = 0;

    // iterator to go through 'message_payload' input buffer
    uint16_t payload_it = 0;

    // high nibble
    uint8_t hn = 0;

    // low nibble
    uint8_t ln = 0;

    // checck if input string is located in legoit RAM memory
    if (variant_validate_is_within_ram(message_payload) == 1 && message_payload_ln > 0) {

        // check second time of string begins with 'HS'
        if (KISS_COMMUNICATION_APRSMSG_DEC_CURRENT_CHAR == 'H' && KISS_COMMUNICATION_APRSMSG_DEC_NEXT_CHAR == 'S') {
            
            // move iterator to begining of hex-characters
            payload_it += 2;

            while (payload_it < message_payload_ln) {

                // check if current char resembles hex base number
                if (KISS_COMMUNICATION_APRSMSG_IS_DIGIT(KISS_COMMUNICATION_APRSMSG_DEC_CURRENT_CHAR) == 0) {
                    break;  // if not break conversion
                }

                if (KISS_COMMUNICATION_APRSMSG_DEC_OUTPUT_IT >= output_ln) {
                    break;  // no more room for output data
                }

                hn = KISS_COMMUNICATION_APRSMSG_DEC_CURRENT_CHAR;
                ln = KISS_COMMUNICATION_APRSMSG_DEC_NEXT_CHAR;

                // calculate index to lookup table basing on current character
				const uint8_t index_hn = hn - 0x30u;

                // calculate index to lookup table basing on current character
				const uint8_t index_ln = ln - 0x30u;

                // conveted byte
                const uint8_t converted_byte = (kiss_communication_aprsmsg_lookup_table[index_hn] * 0x10u) + 
                                                kiss_communication_aprsmsg_lookup_table[index_ln];

                // put converted byte into output array
                output_binary_buffer[KISS_COMMUNICATION_APRSMSG_DEC_OUTPUT_IT] = converted_byte;

                payload_it += 2;
            }

            // go through 

            out = KISS_COMMUNICATION_APRSMSG_DEC_OUTPUT_IT;
        }
    }

    return out;
}


uint16_t kiss_communication_aprsmsg_encode_hexstring(uint8_t * input_binary_buffer, uint16_t input_ln, uint8_t * output_message, uint16_t output_message_max_ln)
{
    uint16_t out = 0;

    // iterator across input buffer
    uint16_t iterator = 0;

    // expected lenght of output buffer. Prefix 'HS', then hex encoded binary data and null termimator at the end
    const uint16_t expected_output_ln = 2 + (input_ln * 2) + 1;

    // if output buffer is big enought to fit 
    if (output_message_max_ln > expected_output_ln) {

        // make a room for output data
        memset(output_message, 0x00, expected_output_ln);

        // put prefix
        *output_message         = 'H';
        *(output_message + 1)   = 'S';

        while (iterator < input_ln) {

            // extract high and low nibbled for processed byte
            const uint8_t low_nibble = input_binary_buffer[iterator] & 0xFu;
            const uint8_t high_nibble = (input_binary_buffer[iterator] & 0xF0u) >> 4;

            output_message[KISS_COMMUNICATION_APRSMSG_ENC_OUTPUT_L_IT(iterator)] = KISS_COMMUNICATION_APRSMSG_GET_CHAR(low_nibble);
            output_message[KISS_COMMUNICATION_APRSMSG_ENC_OUTPUT_H_IT(iterator)] = KISS_COMMUNICATION_APRSMSG_GET_CHAR(high_nibble);

            out = KISS_COMMUNICATION_APRSMSG_ENC_OUTPUT_L_IT(iterator) + 1;

            iterator++;
            
        }
    }

    return out;
}
