
#include "ax25.h"

#include <string.h> /* memset */
#include <ctype.h>
#include <stdio.h> /* printf */

#include <afsk.h>
#include <cfifo.h>
#include <crc.h>
#include <kiss_communication/kiss_communication.h>
#include "station_config.h"


AX25Msg ax25_rxed_frame;
char ax25_new_msg_rx_flag;

uint32_t ax25_ch_wait_start = 0;


/*********************************************************************************************************************/
static bool fifo_isempty(const FIFOBuffer *fb) {
/*********************************************************************************************************************/

	return fb->head == fb->tail;

}


/*********************************************************************************************************************/
static bool fifo_isfull(const FIFOBuffer *fb) {
/*********************************************************************************************************************/

	return ((fb->head == fb->begin) && (fb->tail == fb->end)) || (fb->tail == fb->head - 1);

}


/*********************************************************************************************************************/
static void fifo_push(FIFOBuffer *fb, uint8_t c) {
/*********************************************************************************************************************/

	/* Write at tail position */
	*(fb->tail) = c;

	if (fb->tail == fb->end)
	{
		/* wrap tail around */
		fb->tail = fb->begin;
	}
	else
	{
		/* Move tail forward */
		fb->tail++;
	}

}


/*********************************************************************************************************************/
static uint8_t fifo_pop(FIFOBuffer *fb) {
/*********************************************************************************************************************/

	if (fb->head == fb->end)
	{
		/* wrap head around */
		fb->head = fb->begin;
		return *(fb->end);
	}
	else
	{
		/* move head forward */
		return *(fb->head++);
	}

}


/*********************************************************************************************************************/
static int16_t fifo_getc(FIFOBuffer *fb) {
/*********************************************************************************************************************/

	if (!fifo_isempty(fb)) return fifo_pop(fb);
	else return -1;

}

/*********************************************************************************************************************/
static void fifo_putc(uint8_t c, FIFOBuffer *fb) {
/*********************************************************************************************************************/

	if (!fifo_isfull(fb)) fifo_push(fb, c);

}

/*********************************************************************************************************************/
static void ax25_decode(AX25Ctx *ctx) {
/*********************************************************************************************************************/

	uint16_t i;

	uint8_t *buf = ctx->buf;
//
	for (i = 0; i < ctx->frm_len ;i++)
		*(ax25_rxed_frame.raw_data + i) = *(ctx->buf + i);
	*(ax25_rxed_frame.raw_data + i) = '\0';
//
	ax25_rxed_frame.raw_msg_len = i;

	for (i = 0; i < sizeof(ax25_rxed_frame.dst.call);i++)
	{
		uint8_t c = (*(buf)++ >> 1);
		(ax25_rxed_frame.dst.call)[i] = (c == ' ') ? '\x0' : c;
	}
	ax25_rxed_frame.dst.ssid = (*buf++ >> 1) & 0x0F;


	for (i = 0; i < sizeof(ax25_rxed_frame.src.call);i++)
	{
		uint8_t c = (*(buf)++ >> 1);
		(ax25_rxed_frame.src.call)[i] = (c == ' ') ? '\x0' : c;
	}
	ax25_rxed_frame.src.ssid = (*buf >> 1) & 0x0F;


	for (ax25_rxed_frame.rpt_cnt = 0; !(*buf++ & 0x01) && (ax25_rxed_frame.rpt_cnt < (sizeof(ax25_rxed_frame.rpt_lst) / sizeof(*(ax25_rxed_frame.rpt_lst)))); ax25_rxed_frame.rpt_cnt++)
	{

		for (i = 0; i < sizeof(ax25_rxed_frame.rpt_lst[ax25_rxed_frame.rpt_cnt].call);i++)
		{
			uint8_t c = (*(buf)++ >> 1);
			(ax25_rxed_frame.rpt_lst[ax25_rxed_frame.rpt_cnt].call)[i] = (c == ' ') ? '\x0' : c;
		}
		ax25_rxed_frame.rpt_lst[ax25_rxed_frame.rpt_cnt].ssid = (*buf >> 1) & 0x0F;

		if ((*buf & 0x80)) (&ax25_rxed_frame)->rpt_flags |= BV(ax25_rxed_frame.rpt_cnt);
		else (&ax25_rxed_frame)->rpt_flags &= ~BV(ax25_rxed_frame.rpt_cnt);

	}

	ax25_rxed_frame.ctrl = *buf++;
	if (ax25_rxed_frame.ctrl != AX25_CTRL_UI) return;

	ax25_rxed_frame.pid = *buf++;
	if (ax25_rxed_frame.pid != AX25_PID_NOLAYER3) return;

	ax25_rxed_frame.len = ctx->frm_len - 2 - (buf - ctx->buf);
	ax25_rxed_frame.info = buf;


	if (ctx->hook) {
		ax25_new_msg_rx_flag = 1;
//		ctx->dcd = false;
	 	ctx->hook(&ax25_rxed_frame);
	}
	/*
		insert your code here
	*/
	
	/*
		end
	*/

}


/*********************************************************************************************************************/
void ax25_poll(AX25Ctx *ctx) {
/*********************************************************************************************************************/

	int16_t c;		// TODO: put an assert here!!!!

while ((c = fifo_getc(&ctx->afsk->rx_fifo)) != -1)
	{

		if (!ctx->escape && c == HDLC_FLAG)
		{

			if (ctx->frm_len >= AX25_MIN_FRAME_LEN)
			{
				if (ctx->crc_in == AX25_CRC_CORRECT)
				{

					if (ctx->raw)
					{
						if (ctx->hook)
						{
							ctx->hook(NULL);
						}
					}
					else
					{
						ax25_decode(ctx);
					}

				}
			}

			ctx->sync = true;
			ctx->crc_in = CRC_CCITT_INIT_VAL;
			ctx->frm_len = 0;

			ctx->dcd_state = 0;
			ctx->dcd = false;	 ///// bylo false
			continue;

		}

		if (!ctx->escape && c == HDLC_RESET)
		{
			ctx->sync = false;
			ctx->dcd = false;
			continue;
		}

		if (!ctx->escape && c == AX25_ESC)
		{
			ctx->escape = true;
			continue;
		}

		if (ctx->sync)
		{
			if (ctx->frm_len < CONFIG_AX25_FRAME_BUF_LEN)
			{
				ctx->buf[ctx->frm_len++] = c;
				ctx->crc_in = updcrc_ccitt(c, ctx->crc_in);
				/* Begin of Destination-ADDR based DCD*/ 
				if (ctx->buf[6] == 0x60 && ctx->buf[0] == 0x82) {
					ctx->dcd = true;
				}
				/* End */ 
				if (ctx->dcd_state == 1 && c == AX25_PID_NOLAYER3) {
					ctx->dcd_state ++;
					ctx->dcd = true;
				}

				if (ctx->dcd_state == 0 && c == AX25_CTRL_UI) {
					ctx->dcd_state ++;
				}
			}
			else
			{
				ctx->sync = false;
				ctx->dcd = false;
			}
		}

		ctx->escape = false;

	}

}

/*********************************************************************************************************************/
void ax25_putchar(AX25Ctx *ctx, uint8_t c) {
/*********************************************************************************************************************/

	if (c == HDLC_FLAG || c == HDLC_RESET || c == AX25_ESC)
	{
		fifo_putc(AX25_ESC, &ctx->afsk->tx_fifo);
	}

	ctx->crc_out = updcrc_ccitt(c, ctx->crc_out);
	fifo_putc(c, &ctx->afsk->tx_fifo);

}


/*********************************************************************************************************************/
static void ax25_sendCall(AX25Ctx *ctx, const AX25Call *addr, bool last) {
/*********************************************************************************************************************/

//#ifdef _DBG_TRACE
//				trace_printf("ax25_sendCall:call=%s\r\n", addr->call);
//#endif

	uint16_t i;
	uint8_t ssid;
	uint16_t len = MIN(sizeof(addr->call), strlen(addr->call));


	for (i = 0; i < len; i++)
	{
		uint8_t c = addr->call[i];
//		c = toupper(c);	  /////////////////////////////////////////////
		ax25_putchar(ctx, c << 1);
	}

	if (len < sizeof(addr->call))
	{
		for (i = 0; i < sizeof(addr->call) - len; i++)
		{
			ax25_putchar(ctx, ' ' << 1);
		}
	}

	ssid = 0x60 | (addr->ssid << 1) | (last ? 0x01 : 0);
	ax25_putchar(ctx, ssid);

}

/*********************************************************************************************************************/
void ax25_sendVia(AX25Ctx *ctx, const AX25Call *path, uint16_t path_len, const void *_buf, uint16_t len) {
/*********************************************************************************************************************/

	uint16_t i;
	uint8_t crcl,crch;
	const uint8_t *buf = (const uint8_t *)_buf;

	ctx->crc_out = CRC_CCITT_INIT_VAL;

	fifo_putc(HDLC_FLAG, &ctx->afsk->tx_fifo);

	for (i = 0; i < path_len; i++)
	{
		ax25_sendCall(ctx, &path[i], (i == path_len - 1));
	}

	ax25_putchar(ctx, AX25_CTRL_UI);
	ax25_putchar(ctx, AX25_PID_NOLAYER3);

	while (len--)
	{
		ax25_putchar(ctx, *buf++);
	}

	crcl = (ctx->crc_out & 0xff) ^ 0xff;
	crch = (ctx->crc_out >> 8) ^ 0xff;
	ax25_putchar(ctx, crcl);
	ax25_putchar(ctx, crch);

	fifo_putc(HDLC_FLAG, &ctx->afsk->tx_fifo);

}

uint16_t ax25_sendVia_toBuffer(const AX25Call *path, uint16_t path_len, const void *payload, uint16_t payload_len, uint8_t* output_buf, uint16_t output_size) {

	uint16_t i;
	uint16_t crc = CRC_CCITT_INIT_VAL;
	const uint8_t *buf = (const uint8_t *)payload;

	uint16_t return_val = 0;

	kiss_reset_buffer(output_buf, output_size, &return_val);

	for (i = 0; i < path_len; i++)
	{
		kiss_put_call(&path[i], (i == path_len - 1), output_buf, output_size, &return_val, &crc);
	}

	kiss_put_char(AX25_CTRL_UI, output_buf, output_size, &return_val, &crc);
	kiss_put_char(AX25_PID_NOLAYER3, output_buf, output_size, &return_val, &crc);

	while (payload_len--)
	{
		kiss_put_char(*buf++, output_buf, output_size, &return_val, &crc);
	}

	kiss_finalize_buffer(output_buf, output_size, &return_val);

	return return_val;
}

/*********************************************************************************************************************/
void ax25_sendRaw(AX25Ctx *ctx, const void *_buf, uint16_t len) {
/*********************************************************************************************************************/

	const uint8_t *buf = (const uint8_t *)_buf;
	uint8_t crcl,crch;
	ctx->crc_out = CRC_CCITT_INIT_VAL;
	fifo_putc(HDLC_FLAG, &ctx->afsk->tx_fifo);

	while (len--) ax25_putchar(ctx, *buf++);

	crcl = (ctx->crc_out & 0xff) ^ 0xff;
	crch = (ctx->crc_out >> 8) ^ 0xff;
	ax25_putchar(ctx, crcl);
	ax25_putchar(ctx, crch);

	fifo_putc(HDLC_FLAG, &ctx->afsk->tx_fifo);

}

/*********************************************************************************************************************/
void ax25_init(AX25Ctx *ctx, Afsk *afsk, bool raw, ax25_callback_t hook, ax25_ch_free_timeout_callback_t free_timeout) {
/*********************************************************************************************************************/

	memset(ctx, 0, sizeof(*ctx));

	ctx->afsk = afsk;
	ctx->raw = raw;
	ctx->hook = hook;
	ctx->crc_in = ctx->crc_out = CRC_CCITT_INIT_VAL;
	ctx->timeout_hook = free_timeout;


}

