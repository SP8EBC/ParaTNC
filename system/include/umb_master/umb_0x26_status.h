/*
 * umb_0x26_status_request.h
 *
 *  Created on: 23.02.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_MASTER_UMB_0X26_STATUS_H_
#define INCLUDE_UMB_MASTER_UMB_0X26_STATUS_H_

#include "../umb_master/umb_retval_t.h"
#include "../umb_master/umb_context_t.h"
#include "../umb_master/umb_frame_t.h"

umb_retval_t umb_0x26_status_request(umb_frame_t* frame, umb_context_t* ctx);
umb_retval_t umb_0x26_status_callback(umb_frame_t* frame, umb_context_t* ctx);

#endif /* INCLUDE_UMB_MASTER_UMB_0X26_STATUS_H_ */
