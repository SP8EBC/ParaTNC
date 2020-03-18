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

#define UMB_CONTEXT_ERR_HISTORY_LN 4

typedef struct umb_context_t {

	int16_t current_routine;

	uint8_t nok_error_codes[UMB_CONTEXT_ERR_HISTORY_LN];

	uint8_t nok_error_it;

	uint32_t time_of_last_nok;

	umb_state_t state;

} umb_context_t;

#endif /* INCLUDE_UMB_MASTER_UMB_CONTEXT_T_H_ */
