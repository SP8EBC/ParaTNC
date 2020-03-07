/*
 * umb_call_reason.h
 *
 *  Created on: 07.03.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_MASTER_UMB_CALL_REASON_H_
#define INCLUDE_UMB_MASTER_UMB_CALL_REASON_H_

typedef enum umb_call_reason_t {

	REASON_TRANSMIT_IDLE,
	REASON_RECEIVE_IDLE,
	REASON_RECEIVE_ERROR,
	REASON_IDLE
}umb_call_reason_t;

#endif /* INCLUDE_UMB_MASTER_UMB_CALL_REASON_H_ */
