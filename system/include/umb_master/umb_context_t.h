/*
 * umb_context_t.h
 *
 *  Created on: 23.02.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_MASTER_UMB_CONTEXT_T_H_
#define INCLUDE_UMB_MASTER_UMB_CONTEXT_T_H_

#include <stdint.h>
#include "umb_state_t.h"
#include "../drivers/serial.h"

typedef struct umb_context_t {

	int16_t current_routine;

	umb_state_t state;

} umb_context_t;

#endif /* INCLUDE_UMB_MASTER_UMB_CONTEXT_T_H_ */
