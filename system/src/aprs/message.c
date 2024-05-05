/*
 * message.c
 *
 *  Created on: Apr 20, 2024
 *      Author: mateusz
 */

#include "message.h"
#include "variant.h"
#include "string.h"
#include "stdio.h"

#ifdef UNIT_TEST
#define STATIC
#else
#define STATIC static
#endif

#define MESSAGE_RECIPIENT_FIELD_SIZE    9

#define MESSAGE_SSID_CHARS_LN           2

#define MESSAGE_SENDER_CALL_SSID_MAXLEN 9

#define MESSAGE_CURRENT_CHAR            *(message + content_position + i)

#define MESSAGE_CURRENT_SENDER_CHAR     *(message + i)

#define MESSAGE_IS_HEX_DIGIT(c)         ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))

#define MESSAGE_IS_DIGIT(c)             ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))

#define MESSAGE_IS_DIGIT_OR_LETTER(c)   ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))    

#define MESSAGE_ACK_REMAINING_BUF       (out_buffer_ln - current_pos)

#define MESSAGE_ACK_CURRENT_POS         (char*)(out_buffer + current_pos)

/**
 * Lookup table used to string-to-int conversion for HEX strings. Value of 0x30 should be
 * subtracted from ASCII scancode and this value should be used to intex this table 
*/
static const int8_t message_atoi_lookup[] = {   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 
                                                    -1, -1, -1, -1, -1, -1, -1,
                                                10, 11, 12, 13, 14, 15,
                                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                                -1, -1, -1, -1, -1, -1,
                                                10, 11, 12, 13, 14, 15};


/**
 * Convert string with message number to integer, with automatic detection of decimal or hex base. It operates
 * only on positive integers up to 32 bit wide.
 * @param string
 * @param string_ln
 * @param output optional pointer to message structure when output will be also put
 * @return unsigned integer with decoded number
 */
STATIC uint32_t message_atoi_message_counter(const uint8_t const * string, uint8_t string_ln, message_t * output) {

    // sum to be returned
    uint32_t sum = 0u;

    // temporary buffer
    int8_t conversion_table[6];

    // set to non zero if any digit from A fo F was found in input string
    int8_t is_hex = 0;

    // checck if input string is located in legoit RAM memory
    if (variant_validate_is_within_ram(string) == 1 && string_ln <= 6) {

        // clear intermediate convertion buffer
        memset(conversion_table, 0x00, 6);

        // iterator over conversion_table array
        int8_t conversion_it = string_ln;

        // iterate from the end of a string
        for (int8_t string_it = string_ln; string_it >= 0; string_it--) {

            // currently processed character
        	const uint8_t current_char = *(string + string_it - 1);

            // check if current character is hex or dec base digit
			if (MESSAGE_IS_DIGIT(current_char)) {

                // calculate index to lookup table basing on current character
				const uint8_t index = current_char - 0x30u;

                // check lookup table for converting character with a digit to a number
                conversion_table[string_ln - conversion_it] = message_atoi_lookup[index];

                // decrement an interator
                conversion_it--;

                if (MESSAGE_IS_HEX_DIGIT(current_char)) {
                    is_hex = 1;
                }
            }
        }

        if (is_hex == 1) {
            sum =   (conversion_table[0]);
            sum +=  (conversion_table[1] * 0x10);
            sum +=  (conversion_table[2] * 0x100);
            sum +=  (conversion_table[3] * 0x1000);
            sum +=  (conversion_table[4] * 0x10000);
            sum +=  (conversion_table[5] * 0x100000);
        }
        else {
            sum =   (conversion_table[0]);
            sum +=  (conversion_table[1] * 10);
            sum +=  (conversion_table[2] * 100);
            sum +=  (conversion_table[3] * 1000);
            sum +=  (conversion_table[4] * 10000);
            sum +=  (conversion_table[5] * 100000);          
        }
    }

    if (variant_validate_is_within_ram(output) == 1) {
        output->number = sum & 0xFFu;

        if (is_hex == 1) {
            if (output->source == MESSAGE_SOURCE_APRSIS) {
                output->source = MESSAGE_SOURCE_APRSIS_HEXCNTR;
            }
            else if (output->source == MESSAGE_SOURCE_RADIO) {
                output->source = MESSAGE_SOURCE_RADIO_HEXCNTR;
            }
        }
    }

    return sum;
}

/**
 * Decode received data to look for an APRS message and put it into a structure
 * @param message pointer to data received from APRS-IS
 * @param message_ln lenght of a buffer content
 * @param content_position optional position of an APRS message content (recipient callsign). if set to zero function will look for it 
 * @param output parsed APRS message content
 * @return zero if message has been found and decoded, non zero if parsing failed
 */
uint8_t message_decode(const uint8_t * const message, const uint16_t message_ln, uint16_t content_position, message_source_t src, message_t * output) {

    // example message::  SP8EBC>APX216,TCPIP*,qAC,NINTH::SR9WXZ   :tedt{0s}\r\n

    // result returned by the function, although shamesly it is also used as local aux variable
    // in few places of this function :( this is what You sometimes do to save some stack.
    uint8_t result = 0xFF;

    uint16_t i = 0;

    if (output == 0x00) {
        return result;
    }

    if (message_ln < (MESSAGE_SENDER_CALL_SSID_MAXLEN + MESSAGE_RECIPIENT_FIELD_SIZE)) {
        return result;
    }

    memset(output->number_str, 0x00, MESSAGE_NUMBER_STRING_BUFFER);

    // if start position of APRS message (position of recipient callsign) has not been provided 
    if ((src == MESSAGE_SOURCE_APRSIS) && (content_position == 0)) {

        // look for it
        for (i = 0; i < message_ln; i++) {
            const uint8_t * this_character = message + i;

            const uint8_t * next_character = message + i + 1;

            // break on an end of input string
            if (*this_character == 0x00) {
                break;
            }

            // check if double semicolon has been found
            if ((*this_character == ':') && (*next_character == ':')) {
                // APRS message starts after second semicolong
                content_position = i + 2;
                break;
            }
	    }
    }

    // clear the iterator, it will be used across this function
    i = 0;

    // check content position one more time to verify
    // if input data contains APRS message at all
    if (
        ((src == MESSAGE_SOURCE_APRSIS) && (content_position != 0)) ||
        (src == MESSAGE_SOURCE_RADIO)
        ) {

        // set this variable to zero as now it will be used as a local
        result = 0;

        // clear output structure to make room for new data 
        memset(output, 0x00, sizeof(message_t));

        //extract sender call and SSID only if this message has been received from APRS-IS in pure text form
        if (src == MESSAGE_SOURCE_APRSIS) {
            // fast forward any potential whitespace at the begining
            while (MESSAGE_CURRENT_SENDER_CHAR == ' ') {
                i++;

                if (i >= message_ln) {
                    break;
                }
            }

            // extract sender callsign
            for (; i < MESSAGE_SENDER_CALL_SSID_MAXLEN; i++) {

                // if SSID separator or sender end character ('>') has been reached
                if (MESSAGE_CURRENT_SENDER_CHAR == '-' || MESSAGE_CURRENT_SENDER_CHAR == '>') {
                    break;
                }

                output->from.call[result++] = MESSAGE_CURRENT_SENDER_CHAR;
            }

            // check if sender has SSID
            if (MESSAGE_CURRENT_SENDER_CHAR == '-') {
                // jumps to next character. otherwise separating '-'
                // will be interpreted as a minus/negitive sign
                i++;
                
                result = 0; // here used as a local iterator

                // extract SSID
                for (; i < MESSAGE_RECIPIENT_FIELD_SIZE; i++) {
                    // copy characters to aux buffer
                    output->number_str[result++] = MESSAGE_CURRENT_SENDER_CHAR;

                    // check if there isn't enough characters
                    if (result == MESSAGE_NUMBER_STRING_BUFFER) {
                        break;
                    }
                }

                // convert SSID to int
                output->from.ssid = atoi(output->number_str);
            }

            // clear the iterator, it will be used across this function
            i = 0;
        }
        else {
            // clear content_position iterator
            content_position = 0;

            // find a begining of the message in data from radio channel
            while (MESSAGE_CURRENT_CHAR != ':') {
                content_position++;

                if (content_position >= message_ln) {
                    break;
                }
            }
            // jump over ':'
            content_position++;
        }

        // extract recipient
        for (; i < MESSAGE_RECIPIENT_FIELD_SIZE; i++) {

            // look if end of callsign or separating '-' has been found
            if (MESSAGE_CURRENT_CHAR == ' ' || MESSAGE_CURRENT_CHAR == '-') {
                break;  // and go for SSID reading
            }

            // copy recipient callsign character per character
            output->to.call[i] = MESSAGE_CURRENT_CHAR;
        }

        // check if callsign has SSID set
        if (MESSAGE_CURRENT_CHAR == '-') {
        	// jumps to next character. otherwise separating '-'
        	// will be interpreted as a minus/negitive sign
        	i++;
            
            result = 0; // here used as a local iterator

            // extract SSID
            for (; i < MESSAGE_RECIPIENT_FIELD_SIZE; i++) {
                // copy characters to aux buffer
                output->number_str[result++] = MESSAGE_CURRENT_CHAR;

                // check if there isn't enough characters
                if (result == MESSAGE_NUMBER_STRING_BUFFER) {
                    break;
                }
            }

            // convert SSID to int
            output->to.ssid = atoi(output->number_str);
        }
        
        if (result != MESSAGE_NUMBER_STRING_BUFFER && /* if SSID extraction was OK and end of a buffer hasn't been reached */
        		(
                    (i < MESSAGE_RECIPIENT_FIELD_SIZE) || /* if recipient and callsign has been read before ':' separating from message content */
                    ((i == MESSAGE_RECIPIENT_FIELD_SIZE) && (MESSAGE_CURRENT_CHAR == ':')) /* if a position of separating ':' was reached and ':' is there */
                )
            ) {

            // reinitialize buffer before next usage
            memset(output->number_str, 0x00, MESSAGE_NUMBER_STRING_BUFFER);

			// check if the iterator is set now to position of ':' separating
			// recipient an the message itself
			while(MESSAGE_CURRENT_CHAR != ':' && i <= MESSAGE_RECIPIENT_FIELD_SIZE) {
				i++;
			}

			// one more incrementation to jump over ':' and land on the first character of the message
			i++;

            result = 0;

            // then copy message, which ends on a counter, something like '{1'
            while(MESSAGE_CURRENT_CHAR != ':' && i + content_position < message_ln) {
                output->content[result++] = MESSAGE_CURRENT_CHAR;
                i++;

                // break on message counter separator
                if (MESSAGE_CURRENT_CHAR == '{') {
                    i++;        // move to first digit of a counter 
                    break;
                }
            }

            // check which condition has ended previous 'while' loop and if an end of the buffer has been reached
            if (i + content_position < message_ln) {
                result = 0;

                // now iterator is set (should be set) on a first digit of message counter
                // copy everything until first non digit character is found
                while (MESSAGE_IS_DIGIT_OR_LETTER(MESSAGE_CURRENT_CHAR)) {
                    output->number_str[result++] = MESSAGE_CURRENT_CHAR;

                    i++;

                    // check if there isn't enough characters
                    if (result == MESSAGE_NUMBER_STRING_BUFFER) {
                        break;
                    }
                }

                output->source = src;

                // convert message counter from string to int
                message_atoi_message_counter(output->number_str, result, output);

                if (result < MESSAGE_NUMBER_STRING_BUFFER) {
                    // new we are done (??)
                    result = 0;
                }
            }
            else {
            	result = 0xFF;
            }
        }
        else {
        	result = 0xFF;
        }
    }

    return result;
}

/**
 *
 * @param config_data
 * @param message
 * @return zero if this is a message to us, non zero otherwise
 */
uint8_t message_is_for_me(const char * const callsign, const uint8_t ssid, const message_t * const message)
{
    const int _callsign = strncmp(callsign, message->to.call, 6);

    if (_callsign == 0 && (ssid == message->to.ssid)) {
        return 0;
    }
    else {
        return 1;
    }
}

/**
 *
 * @param out_buffer
 * @param out_buffer_ln
 * @param message
 * @param src how this message has been received
 */
int message_create_ack_for(uint8_t * out_buffer, const uint16_t out_buffer_ln, const message_t * const message)
{
    int current_pos = 0;

    uint8_t call_position = 0;

    const message_source_t src = message->source;

    // clear output buffer
    memset(out_buffer, 0x00, out_buffer_ln);

    if (src == MESSAGE_SOURCE_APRSIS || src == MESSAGE_SOURCE_APRSIS_HEXCNTR) {
        
        // put my callsign
        for (; call_position < 6; call_position++) {
            // break on null character
            if (message->to.call[call_position] == 0x00) {
                break;
            }

            // copy callsign data
            out_buffer[current_pos + call_position] = message->to.call[call_position];
        }

        current_pos += call_position;

        call_position = 0;

        // check if I have a SSID
        if (message->to.ssid != 0) {
            current_pos += snprintf(MESSAGE_ACK_CURRENT_POS, MESSAGE_ACK_REMAINING_BUF, "-%d", message->to.ssid);
        }

        // constant part
        current_pos += snprintf(MESSAGE_ACK_CURRENT_POS, MESSAGE_ACK_REMAINING_BUF, ">AKLPRZ::");

        // put sender callsign, station I received this message from 
        for (; call_position < 6; call_position++) {
            // break on null character
            if (message->from.call[call_position] == 0x00) {
                break;
            }

            // copy callsign data
            out_buffer[current_pos + call_position] = message->from.call[call_position];
        }

        // put sender SSID, station I received this message from 
        if (message->from.ssid != 0) {
            call_position += snprintf(MESSAGE_ACK_CURRENT_POS + call_position, MESSAGE_ACK_REMAINING_BUF, "-%d", message->from.ssid);
        }

        // check if callsign was shorter than 6 characters
        while (call_position < 9) {
            // copy callsign data
            out_buffer[current_pos + call_position] = ' ';

            call_position++;
        }

        // callsign + ssid + padding must be exactly 9 characters long
        current_pos += 9;

        // then put 'ackXX' where X is message number
        current_pos += snprintf(MESSAGE_ACK_CURRENT_POS, MESSAGE_ACK_REMAINING_BUF, ":ack%s\r\n", message->number_str);


    }
    else {

    }

    return current_pos;
}
