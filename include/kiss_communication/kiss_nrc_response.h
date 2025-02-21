/*
 * kiss_nrc_generator.h
 *
 *  Created on: Jul 2, 2023
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_KISS_NRC_RESPONSE_H_
#define KISS_COMMUNICATION_KISS_NRC_RESPONSE_H_

#include <stdint.h>

int kiss_nrc_response_fill_unknown_service(uint8_t * buffer);
int kiss_nrc_response_fill_subfunction_not_supported(uint8_t * buffer) ;
int kiss_nrc_response_fill_request_out_of_range(uint8_t * buffer);
int kiss_nrc_response_fill_incorrect_message_lenght_or_format(uint8_t * buffer);
int kiss_nrc_response_fill_security_access_denied(uint8_t * buffer);
int kiss_nrc_response_fill_conditions_not_correct(uint8_t * buffer);
int kiss_nrc_response_fill_sequence_error(uint8_t * buffer);
int kiss_nrc_response_fill_general_reject(uint8_t * buffer);

#endif /* KISS_COMMUNICATION_KISS_NRC_RESPONSE_H_ */
