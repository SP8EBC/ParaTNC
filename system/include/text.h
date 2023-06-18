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

	// check if input pointer is set to something
	if (str != 0) {

		while (*(str + out) < 0x21 || *(str + out) > 0x7F) {

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

	if (str != 0) {

		while (*(str + out) < 0x21 || *(str + out) > 0x7F) {
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
