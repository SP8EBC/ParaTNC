#ifndef KISSCALLBACK_H_
#define KISSCALLBACK_H_

#include "kiss_communication.h"

#include <stdint.h>

void kiss_callback_get_running_config();
int16_t kiss_pool_callback_get_running_config(uint8_t * output_buffer, uint16_t buffer_size, uint8_t current_segment );

#endif
