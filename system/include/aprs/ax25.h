#ifndef AX25_H_
#define AX25_H_

#include <ax25_config.h>
#include <stdbool.h>

#include "main_master_time.h"

#include "ax25_t.h"
#include "cfifo.h"
#include "afsk.h"

#include "macros.h"

/**
 * Macro to wait for channel to be free
 */
#define WAIT_FOR_CHANNEL_FREE()									\
		ax25_ch_wait_start = main_get_master_time();			\
		while (main_ax25.dcd == 1) {							\
																\
			if (main_get_master_time() - ax25_ch_wait_start > 	\
				CONFIG_AX25_MAX_WAIT_FOR_CH_FREE	)			\
				{												\
					if (main_ax25.timeout_hook != 0) {			\
						main_ax25.timeout_hook();				\
					}											\
					break;										\
				}												\
																\
		}														\

/**
 * Maximum size of a AX25 frame.
 */
#define AX25_MIN_FRAME_LEN 18

/**
 * CRC computation on correct AX25 packets should
 * give this result (don't ask why).
 */
#define AX25_CRC_CORRECT 0xF0B8

#define AX25_CTRL_UI      0x03
#define AX25_PID_NOLAYER3 0xF0

#define HDLC_FLAG  0x7E
#define HDLC_RESET 0x7F
#define AX25_ESC   0x1B


struct AX25Msg; // fwd declaration


/**
 * Create an AX25Call structure on the fly.
 * \param str callsign, can be 6 characters or shorter.
 * \param id  ssid associated with the callsign.
 */
#define AX25_CALL(str, id) {.call = (str), .ssid = (id) }
#define AX25_PATH(dst, src, ...) { dst, src, ## __VA_ARGS__ }


typedef struct AX25Ctx
{

	uint8_t buf[CONFIG_AX25_FRAME_BUF_LEN]; ///< buffer for received chars
	Afsk *afsk;

	uint16_t frm_len;   ///< received frame length.
	uint16_t crc_in;  ///< CRC for current received frame
	uint16_t crc_out; ///< CRC of current sent frame

	ax25_callback_t hook; ///< Hook function to be called when a message is received
	ax25_ch_free_timeout_callback_t timeout_hook;	///< callback hook to be called

	bool raw;
	bool sync;   ///< True if we have received a HDLC flag.
	bool escape; ///< True when we have to escape the following char.
	uint8_t dcd_state;
	bool dcd;

} AX25Ctx;

extern AX25Msg ax25_rxed_frame;
extern char ax25_new_msg_rx_flag;

extern uint32_t ax25_ch_wait_start;

#ifdef __cplusplus
extern "C"
{
#endif
	
/*********************************************************************************************************************/
void ax25_poll(AX25Ctx *ctx);
/*********************************************************************************************************************/

/*********************************************************************************************************************/
void ax25_sendVia(AX25Ctx *ctx, const AX25Call *path, uint16_t path_len, const void *_buf, uint16_t len);
/*********************************************************************************************************************/

/*********************************************************************************************************************/
uint16_t ax25_sendVia_toBuffer(const AX25Call *path, uint16_t path_len, const void *payload, uint16_t payload_len, uint8_t* output_buf, uint16_t output_size);
/*********************************************************************************************************************/

/*********************************************************************************************************************/
void ax25_sendRaw(AX25Ctx *ctx, const void *_buf, uint16_t len);
/*********************************************************************************************************************/

/*********************************************************************************************************************/
void ax25_init(AX25Ctx *ctx, Afsk *afsk, bool raw, ax25_callback_t hook, ax25_ch_free_timeout_callback_t free_timeout);
/*********************************************************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* AX25_H_ */
