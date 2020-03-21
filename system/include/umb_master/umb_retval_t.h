/*
 * umb_return_values.h
 *
 *  Created on: 23.02.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_MASTER_UMB_RETVAL_T_H_
#define INCLUDE_UMB_MASTER_UMB_RETVAL_T_H_

typedef enum umb_retval_t {
	UMB_UNINITIALIZED = 127,
	UMB_OK = 0,
	UMB_FRAME_TOO_LONG_FOR_TX = -1,
	UMB_NOT_VALID_FRAME = -2,
	UMB_TO_ANOTHER_MASTER = -3,
	UMB_RECV_FRAME_TOO_LONG	= -4,
	UMB_WRONG_CRC = -5,
	UMB_BUSY = -6,
	UMB_NOK_STATUS_GOT_WITH_RESP = -7,
	UMB_VALUE_OUT_OF_RANGE = -8,
	UMB_GENERAL_ERROR = -63
}umb_retval_t;

#endif /* INCLUDE_UMB_MASTER_UMB_RETVAL_T_H_ */
