
#include "ax25.h"

#include <string.h> /* memset */
#include <ctype.h>
#include <stdio.h> /* printf */

#include <afsk.h>
#include <cfifo.h>
#include <crc.h>

#include "station_config.h"

AX25Msg msg;
char new_msg_rx;

/*********************************************************************************************************************/
static void ax25_decode(AX25Ctx *ctx) {
/*********************************************************************************************************************/

	uint16_t i;

	uint8_t *buf = ctx->buf;
//
	for (i = 0; i < ctx->frm_len ;i++)
		*(msg.raw_data + i) = *(ctx->buf + i);
	*(msg.raw_data + i) = '\0';
//
	msg.raw_msg_len = i;

	for (i = 0; i < sizeof(msg.dst.call);i++)
	{
		uint8_t c = (*(buf)++ >> 1);
		(msg.dst.call)[i] = (c == ' ') ? '\x0' : c;
	}
	msg.dst.ssid = (*buf++ >> 1) & 0x0F;


	for (i = 0; i < sizeof(msg.src.call);i++)
	{
		uint8_t c = (*(buf)++ >> 1);
		(msg.src.call)[i] = (c == ' ') ? '\x0' : c;
	}
	msg.src.ssid = (*buf >> 1) & 0x0F;


	for (msg.rpt_cnt = 0; !(*buf++ & 0x01) && (msg.rpt_cnt < (sizeof(msg.rpt_lst) / sizeof(*(msg.rpt_lst)))); msg.rpt_cnt++)
	{

		for (i = 0; i < sizeof(msg.rpt_lst[msg.rpt_cnt].call);i++)
		{
			uint8_t c = (*(buf)++ >> 1);
			(msg.rpt_lst[msg.rpt_cnt].call)[i] = (c == ' ') ? '\x0' : c;
		}
		msg.rpt_lst[msg.rpt_cnt].ssid = (*buf >> 1) & 0x0F;

		if ((*buf & 0x80)) (&msg)->rpt_flags |= BV(msg.rpt_cnt);
		else (&msg)->rpt_flags &= ~BV(msg.rpt_cnt);

	}

	msg.ctrl = *buf++;
	if (msg.ctrl != AX25_CTRL_UI) return;

	msg.pid = *buf++;
	if (msg.pid != AX25_PID_NOLAYER3) return;

	msg.len = ctx->frm_len - 2 - (buf - ctx->buf);
	msg.info = buf;


	if (ctx->hook) {
		new_msg_rx = 1;
//		ctx->dcd = false;
	 	ctx->hook(&msg);
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

	int16_t c;

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
void ax25_init(AX25Ctx *ctx, Afsk *afsk, bool raw, ax25_callback_t hook) {
/*********************************************************************************************************************/

	memset(ctx, 0, sizeof(*ctx));

	ctx->afsk = afsk;
	ctx->raw = raw;
	ctx->hook = hook;
	ctx->crc_in = ctx->crc_out = CRC_CCITT_INIT_VAL;


}

