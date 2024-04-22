/*
 * ax25_t.h
 *
 *  Created on: Apr 20, 2024
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_AX25_T_H_
#define INCLUDE_APRS_AX25_T_H_

#include "stdint.h"
#include "ax25_config.h"

/**
 * Maximum number of Repeaters in a AX25 message.
 */
#define AX25_MAX_RPT 8

/**
 * AX25 Call sign.
 */
typedef struct AX25Call
{
	char call[6]; ///< Call string, max 6 character
	uint8_t ssid; ///< SSID (secondary station ID) for the call
} AX25Call;

/**
 * AX25 Message.
 * Used to handle AX25 sent/received messages.
 */
typedef struct AX25Msg
{

	AX25Call src;  ///< Source adress
	AX25Call dst;  ///< Destination address
	AX25Call rpt_lst[AX25_MAX_RPT]; ///< List of repeaters
	uint8_t rpt_cnt; ///< Number of repeaters in this message
	uint8_t rpt_flags; ///< Has-been-repeated flags for each repeater (bit-mapped)
	#define AX25_REPEATED(msg, idx) ((msg)->rpt_flags & BV(idx))
	uint16_t ctrl; ///< AX25 control field
	uint8_t pid;   ///< AX25 PID field
	const uint8_t *info; ///< Pointer to the info field (payload) of the message
	uint16_t len;    ///< Payload length
	uint8_t raw_data[CONFIG_AX25_FRAME_BUF_LEN];   /// Surowa zawarto�� ramki przekopiowana z Ctx->buff
	short int raw_msg_len;				// wielkosc surowej ramki

} AX25Msg;

/**
 * Type for AX25 messages callback.
 */
typedef void (*ax25_callback_t)(struct AX25Msg *ax25_rxed_frame);

/**
 * Type for channel free wait timeout callback
 */
typedef void (*ax25_ch_free_timeout_callback_t)(void);


#endif /* INCLUDE_APRS_AX25_T_H_ */
