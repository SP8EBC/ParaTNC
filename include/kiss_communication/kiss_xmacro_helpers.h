/*
 * kiss_xmacro_helpers.h
 *
 *  Created on: Jun 21, 2023
 *      Author: mateusz
 */

#ifndef KISS_XMACRO_HELPERS_H_
#define KISS_XMACRO_HELPERS_H_


/**
 * This macro is used to define
 */
#define DID_NUMERIC_DEFINITION_EXPANDER(id, first_data_pointer, second_data_pointer, third_data_pointer)	\
		{	\
			.identifier = id, \
			.first_data = (void*)first_data_pointer, \
			.first_data_size = sizeof(first_data_pointer), \
			.second_data = (void*)second_data_pointer, \
			.second_data_size = sizeof(second_data_pointer),	\
			.third_data = (void*)third_data_pointer, \
			.third_data_size = sizeof(third_data_pointer) \
		},


#endif /* KISS_XMACRO_HELPERS_H_ */
