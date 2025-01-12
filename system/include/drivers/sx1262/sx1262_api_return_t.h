/*
 * sx1262_api_return_t.h
 *
 *  Created on: Dec 15, 2024
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_SX1262_API_RETURN_T_H_
#define INCLUDE_DRIVERS_SX1262_SX1262_API_RETURN_T_H_

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

typedef enum sx1262_api_return_t {
	SX1262_API_OK,			//!< Request has been accepted and it is processed now
	SX1262_API_SPI_BUSY,	//!< SPI bus is currently busy on other transfer from/to modem.
	SX1262_API_MODEM_BUSY,	//!< Modem is currently busy on something and cannot process this request
	SX1262_API_OUT_OF_RNG,	//!< Values provided to the API are out of range, so the request was rejected
	SX1262_API_LIB_NOINIT,	//!< Driver hasn't been initialized yet, thus it is inoperative
	SX1262_API_DAMAGED_RESP	//!< Response received from the modem is completly malformed and damaged
}sx1262_api_return_t;



#endif /* INCLUDE_DRIVERS_SX1262_SX1262_API_RETURN_T_H_ */
