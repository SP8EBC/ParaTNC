/*
 * main_getters_for_task.h
 *
 *  Created on: Aug 3, 2025
 *      Author: mateusz
 */

#ifndef MAIN_GETTERS_FOR_TASK_H_
#define MAIN_GETTERS_FOR_TASK_H_

#include "drivers/serial.h"

//! a pointer to KISS context
srl_context_t* main_get_kiss_srl_ctx_ptr(void);

////! a pointer to wx comms context
//srl_context_t* main_get_wx_srl_ctx_ptr(void);
//
////! a pointer to gsm context
//srl_context_t* main_get_gsm_srl_ctx_ptr(void);

#else
#error "this file can be include only one time, within task_main.c!"
#endif /* MAIN_GETTERS_FOR_TASK_H_ */
