/*
 * kiss_xmacro_helpers.h
 *
 *  Created on: Jun 21, 2023
 *      Author: mateusz
 */

#ifndef KISS_XMACRO_HELPERS_H_
#define KISS_XMACRO_HELPERS_H_

#define DID_EMPTY	did_dummy_data

// clang-format off
/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

/**
 * Expands diagnostic routine definition from big #define table into const def in flash
 */
#define KISS_ROUTINE_DEF_EXPAND(_routine_number, _start_fn, _stop_fn, _get_result_fn, _synchro)		\
	{													\
		.routine_number = _routine_number,				\
		.start_fn = _start_fn,							\
		.stop_fn = _stop_fn,							\
		.get_result_fn = _get_result_fn,				\
		.asynchronous = _synchro						\
	},													\

/**
 * Creates an array of status type for each defined diagnostics routine
 */
#define KISS_ROUTINE_STATUS_EXPAND(_routine_number, _start_fn, _stop_fn, _get_result_fn, _synchro)		\
	{													\
		.routine_number = _routine_number,				\
		.running = 0									\
	},													\

/**
 * Creates an enum, used mostly to count how many diagnostic routines are defined
 */
#define KISS_DIAGNOSTIC_ROUTINE_ENUM_EXPAND(_routine_number, _start_fn, _stop_fn, _get_result_fn, _synchro)		\
	KISS_ROUTINE_NUMBER_##_routine_number,														\

/**
 * This macro is used to define DIDs which return integer values
 */
#define DID_NUMERIC_DEFINITION_EXPANDER(id, first_data_pointer, second_data_pointer, third_data_pointer)	\
		{	\
			.identifier = id, \
			.first_data = (void*)&first_data_pointer, \
			.first_data_size = sizeof(first_data_pointer), \
			.second_data = (void*)&second_data_pointer, \
			.second_data_size = sizeof(second_data_pointer),	\
			.third_data = (void*)&third_data_pointer, \
			.third_data_size = sizeof(third_data_pointer) \
		},

/**
 * Macro to define DID which return float data. Require separate handling as a size
 * of single precision float number is the same than 32 bit integer
 */
#define DID_NUMERIC_FLOAT_DEFINITION_EXPANDER(id, first_data_pointer, second_data_pointer, third_data_pointer)	\
		{	\
			.identifier = id, \
			.first_data = (void*)&first_data_pointer, \
			.first_data_size = 0, \
			.second_data = (void*)&second_data_pointer, \
			.second_data_size = 0,	\
			.third_data = (void*)&third_data_pointer, \
			.third_data_size = 0 \
		},

#define DID_NUMERIC_STRING_DEFINITION_EXPANDER(id, string_pointer )	\
		{	\
			.identifier = id, \
			.first_data = (void*)string_pointer, \
			.first_data_size = sizeof(string_pointer), \
			.second_data = (void*)0xDEADBEEFu, \
			.second_data_size = 0,	\
			.third_data = (void*)0xDEADBEEFu, \
			.third_data_size = 0 \
		},

#endif /* KISS_XMACRO_HELPERS_H_ */

// clang-format on
