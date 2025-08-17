/*
 * text.h
 *
 * Auxiliary functions to do weird things with text strings
 *
 *  Created on: Jun 10, 2023
 *      Author: mateusz
 */

#ifndef INCLUDE_TEXT_H_
#define INCLUDE_TEXT_H_

#include <stdint.h>

/**
 * Converts first four character of a string to uint32_t which is returned
 * Example:
 * 			    char * test = "DUPA";
 *
 *    			uint32_t out = text_get_uint_from_string(test, 4);
 *
 *    			printf("out: 0x%X", out);
 * 
 * Output:
 * 	ASM generation compiler returned: 0
 *	Execution build compiler returned: 0
 *	Program returned: 0
 *	out: 0x44555041
 * 
 * @param str
 * @param str_ln
 * @return
 */
inline static uint32_t text_get_uint32_from_string(char *str, uint8_t str_ln) {

	uint32_t out = 0u;

	if (str_ln > 3)
	{
		out |= (*(str + 3));
	}
	if (str_ln > 2)
	{
		out |= (*(str + 2) << 8);
	}
	if (str_ln > 1)
	{
		out |= (*(str + 1) << 16);
	}
	if (str_ln > 0)
	{
		out |= (*(str + 0) << 24);
	}
	//out = (*(str + 3)) | (*(str + 2) << 8) | (*(str + 1) << 16) | (*(str + 0) << 24);

	return out;
}

/**
 * Converts first four character of a string to uint32_t which is returned.
 * Example:     
 * 			uint32_t out = text_get_uint_from_string(test, 4);
 * @param str
 * @param str_ln
 * @return
 */
inline static uint16_t text_get_uint16_from_string(char *str, uint8_t str_ln) {

	uint16_t out = 0u;

	if (str_ln > 1)
	{
		out |= (*(str + 3));
	}
	if (str_ln > 0)
	{
		out |= (*(str + 2) << 8);
	}

	return out;
}

/**
 * Replace all non printable TABs, NEWLINEs etc with a space. Stops on null terminator
 * or a size of an input string
 * @param str	pointer to a string to be modified
 * @param size	its size
 */
inline static void text_replace_non_printable_with_space(char * str, uint16_t size) {
	for (int i = 0; i < size; i++) {
		// currently processed character
		char current = *(str + i);

		if (current != 0x00) {
			if (current < 0x21 || current > 0x7A) {
				*(str + i) = ' ';
			}
		}
		else {
			// stop processing on null terminator
			break;
		}
	}
}

/**
 * Goes from the end of the buffer and replace all spaces with null terminator.
 * Stops on " (double quotation mark) which is also replaced with null, or
 * at the begining of an string
 * @param str
 * @param size
 */
inline static void text_replace_space_with_null(char * str, uint16_t size) {

	// it goes from the end of a buffer towards its begining
	for (int i = size - 1; i > 0; i--) {
		char current = *(str + i);

		if (current == '\"') {
			// also replace this with null terminator
			*(str + i) = 0x00;
			break;
		}

		if (current == 0x20) {
			*(str + i) = 0x00;
		}
	}
}

/**
 * Fast forward text string to first printable character if it not begin from it.
 * NULL terminator ends processing
 * @param str pointer to string to process
 * @param size 	lenght of a text although if null terminator will be detected before
 * 				processing will finish before reaching this size.
 * @return	an offset from the beginning of the text where first printable character is.
 * 			if the text starts from printable character zero is returned.
 * 			if null is detected before first printable character negative is returned.
 */
inline static int text_fast_forward_to_first_printable(char * str, uint16_t size) {

	int out = 0;

	uint8_t* ptr = (uint8_t*)(str);

	// check if input pointer is set to something
	if (str != 0) {

		while (*(ptr + out) < 0x21 || *(ptr + out) > 0x7F) {

			// if null terminator is detected before a printable character
			if (*(str + out) == 0x00) {
				out = -1;

				break;
			}

			// the same if an end of string is reached
			if (out >= size) {
				out = -1;

				break;
			}

			// move iterator forward
			out++;
		}
	}

	return out;
}

inline static int text_rewind_front_end_till_first_printable(char * str, uint16_t size) {

	int out = size;

	uint8_t* ptr = (uint8_t*)(str);

	if (str != 0) {

		while (*(ptr + out) < 0x21 || *(ptr + out) > 0x7F) {
			// end and return if a begining of a text is reached
			// and no printable character is found
			if (out < 0) {
				break;
			}

			// if null terminator is detected before a printable character
			if (*(str + out) == 0x00) {
				out = -1;

				break;
			}

			// move iterator forward
			out--;
		}
	}

	return out;
}

#endif /* INCLUDE_TEXT_H_ */
