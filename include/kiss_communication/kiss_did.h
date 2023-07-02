/*
 * kiss_did.h
 *
 *  Created on: Jul 2, 2023
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_KISS_DID_H_
#define KISS_COMMUNICATION_KISS_DID_H_

#include <stdint.h>

uint8_t kiss_did_response(uint16_t identifier, uint8_t * output_buffer, uint16_t buffer_ln);



#endif /* KISS_COMMUNICATION_KISS_DID_H_ */
