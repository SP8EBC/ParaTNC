/*
 * button.h
 *
 * 	Handles presses of buttons on ParaTNC or ParaMETEO pcb. IO pins
 * 	initialization is implemented in io.c and io.h
 *
 *  Created on: Jun 10, 2023
 *      Author: mateusz
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <stored_configuration_nvm/configuration_handler.h>

/**
 * Should be called from main for loop or in quite short interval, like
 * 10 to 100 milliseconds. It reads a state of input pins coming from
 * connected push buttons and then, depending on configuration, takes
 * an appropriate action.
 */
void button_check_all(configuration_button_function_t left, configuration_button_function_t right);

/**
 * Resets debouncing inhibiter which enables button key presses back
 */
void button_debounce(void);

#endif /* BUTTON_H_ */
