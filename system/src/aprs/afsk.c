
#include "afsk.h"

#include <string.h> /* memset */
#include <math.h>
#include <stdlib.h>

#include <dac.h>
#include <ax25.h>


#define 	BIT_DIFFER(bitline1, bitline2) (((bitline1) ^ (bitline2)) & 0x01)
#define 	EDGE_FOUND(bitline)	BIT_DIFFER((bitline), (bitline) >> 1)
#define 	SWITCH_TONE(inc)  (((inc) == MARK_INC) ? SPACE_INC : MARK_INC)

#define M_PI 3.1415

static int16_t data[SAMPLEPERBIT];
static int16_t corr_mark_i[SAMPLEPERBIT]; /* sin 1200 Hz */
static int16_t corr_mark_q[SAMPLEPERBIT]; /* cos 1200 Hz */
static int16_t corr_space_i[SAMPLEPERBIT]; /* sin 2200 Hz */
static int16_t corr_space_q[SAMPLEPERBIT]; /* cos 2200 Hz */


static uint8_t ptr=0;

char PersistRand;
char DrawCounter;

extern unsigned short tx10m;

extern uint32_t  	rte_main_tx_total;


/**
 * Sine table for the first quarter of wave.
 * The rest of the wave is computed from this first quarter.
 * This table is used to generate the modulated data.
 */
static const uint8_t sin_table[] = {

	128, 129, 131, 132, 134, 135, 137, 138, 140, 142, 143, 145, 146, 148, 149, 151,
	152, 154, 155, 157, 158, 160, 162, 163, 165, 166, 167, 169, 170, 172, 173, 175,
	176, 178, 179, 181, 182, 183, 185, 186, 188, 189, 190, 192, 193, 194, 196, 197,
	198, 200, 201, 202, 203, 205, 206, 207, 208, 210, 211, 212, 213, 214, 215, 217,
	218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
	234, 234, 235, 236, 237, 238, 238, 239, 240, 241, 241, 242, 243, 243, 244, 245,
	245, 246, 246, 247, 248, 248, 249, 249, 250, 250, 250, 251, 251, 252, 252, 252,
	253, 253, 253, 253, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255,

};

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
static void fifo_flush(FIFOBuffer *fb) {
/*********************************************************************************************************************/

	fb->head = fb->tail;

}


/*********************************************************************************************************************/
static void fifo_init(FIFOBuffer *fb, uint8_t *buf, uint16_t size) {
/*********************************************************************************************************************/

	fb->head = fb->tail = fb->begin = buf;
	fb->end = buf + size - 1;

}

/*********************************************************************************************************************/
static uint8_t sin_sample(uint16_t idx) {
/*********************************************************************************************************************/

	uint8_t data;
	uint16_t new_idx = idx % (SIN_LEN / 2);
	new_idx = (new_idx >= (SIN_LEN / 4)) ? (SIN_LEN / 2 - new_idx - 1) : new_idx;

	data = pgm_read8(&sin_table[new_idx]);

	return (idx >= (SIN_LEN / 2)) ? (255 - data) : data;

}


/*********************************************************************************************************************/
static bool hdlc_parse(Hdlc *hdlc, bool bit, FIFOBuffer *fifo) {
/*********************************************************************************************************************/

	bool ret = true;

	hdlc->demod_bits <<= 1;
	hdlc->demod_bits |= bit ? 1 : 0;

	/* HDLC Flag */
	if (hdlc->demod_bits == HDLC_FLAG)
	{
		if (!fifo_isfull(fifo))
		{
			/* modification by sp8ebc */
			if(hdlc->s == false && hdlc->raw_dcd == true)
			hdlc->raw_dcd = false; 
			/***************************/ 
			fifo_push(fifo, HDLC_FLAG);
			hdlc->rxstart = true;
		}
		else
		{
			ret = false;
			hdlc->rxstart = false;
			fifo_flush(fifo);
		}

		hdlc->currchar = 0;
		hdlc->bit_idx = 0;
		hdlc->s = true;	  //
		return ret;
	}

	/* Reset */
	if ((hdlc->demod_bits & HDLC_RESET) == HDLC_RESET)
	{
		hdlc->rxstart = false;
		hdlc->s = false;	//
		return ret;
	}

	if (!hdlc->rxstart) return ret;

	/* Stuffed bit */
	if ((hdlc->demod_bits & 0x3f) == 0x3e) return ret;

	if (hdlc->demod_bits & 0x01) hdlc->currchar |= 0x80;

	if (++hdlc->bit_idx >= 8)
	{
		if ((hdlc->currchar == HDLC_FLAG || hdlc->currchar == HDLC_RESET || hdlc->currchar == AX25_ESC))
		{
			if (!fifo_isfull(fifo))
			{
				fifo_push(fifo, AX25_ESC);
			}
			else
			{
				hdlc->rxstart = false;
				ret = false;
				fifo_flush(fifo);
			}
		}

		if (!fifo_isfull(fifo))
		{
			fifo_push(fifo, hdlc->currchar);
		}
		else
		{
			hdlc->rxstart = false;
			ret = false;
			fifo_flush(fifo);
		}
		hdlc->currchar = 0;
		hdlc->bit_idx = 0;
		/*******************************************/
		if (hdlc->s == true && hdlc->raw_dcd == false)	   //
		hdlc->raw_dcd = true;	   //
		hdlc->s = false;	  //
		/*******************************************/
	}
	else
	{
		hdlc->currchar >>= 1;
	}

	return ret;

}


/*********************************************************************************************************************/
static int afsk_demod(int16_t curr_sample) {
/*********************************************************************************************************************/

	uint8_t i;
	int16_t d;
	int32_t out_mark_i=0,out_mark_q=0,out_space_i=0,out_space_q=0;

	data[ptr]=curr_sample;

	ptr = (ptr+1)%SAMPLEPERBIT; /* % : Modulo */

	for(i=0;i<SAMPLEPERBIT;i++)
	{

		d = data[(ptr+i)%SAMPLEPERBIT];
		out_mark_i += d*corr_mark_i[i];
		out_mark_q += d*corr_mark_q[i];
		out_space_i += d*corr_space_i[i];
		out_space_q += d*corr_space_q[i];
	}
	
	
	 
	return  (out_space_i>>12)*(out_space_i>>12)+
			(out_space_q>>12)*(out_space_q>>12)-
			(out_mark_i>>12)*(out_mark_i>>12)-
			(out_mark_q>>12)*(out_mark_q>>12);			  /// wszedzie bylo 11

}


/*********************************************************************************************************************/
void AFSK_ADC_ISR(Afsk *afsk, int16_t curr_sample) {
/*********************************************************************************************************************/

	uint8_t bits;
	afsk->sampled_bits <<= 1;
	afsk->sampled_bits |=  (afsk_demod(curr_sample) > 0);

	if (EDGE_FOUND(afsk->sampled_bits))
	{
		if (afsk->curr_phase < PHASE_THRES) afsk->curr_phase += PHASE_INC;
		else afsk->curr_phase -= PHASE_INC;
	}

	afsk->curr_phase += PHASE_BIT;

	if (afsk->curr_phase >= PHASE_MAX)
	{
		afsk->curr_phase %= PHASE_MAX;

		afsk->found_bits <<= 1;

		bits = afsk->sampled_bits & 0x07;

		if (bits == 0x07 // 111, 3 bits set to 1
		 || bits == 0x06 // 110, 2 bits
		 || bits == 0x05 // 101, 2 bits
		 || bits == 0x03 // 011, 2 bits
		)

		afsk->found_bits |= 1;
		if (!hdlc_parse(&afsk->hdlc, !EDGE_FOUND(afsk->found_bits), &afsk->rx_fifo)) afsk->status |= AFSK_RXFIFO_OVERRUN;

	}

}

/*********************************************************************************************************************/
void afsk_txStart(Afsk *af) {
/*********************************************************************************************************************/

	if (!af->sending)
	{
		tx10m++;
		#ifdef STM32L471xx
		rte_main_tx_total++;
		#endif
		
		af->phase_inc = MARK_INC;
		af->phase_acc = 0;
		af->stuff_cnt = 0;
		af->sending = true;
		af->preamble_len = DIV_ROUND(CONFIG_AFSK_PREAMBLE_LEN * BITRATE, 8000);
		DA_Start();
	}
//	ATOMIC(af->trailer_len  = DIV_ROUND(CONFIG_AFSK_TRAILER_LEN  * BITRATE, 8000));
	af->trailer_len  = DIV_ROUND(CONFIG_AFSK_TRAILER_LEN  * BITRATE, 8000);

}

/*********************************************************************************************************************/
uint8_t AFSK_DAC_ISR(Afsk *afsk) {
/*********************************************************************************************************************/


	if (afsk->sample_count == 0)
	{
		if (afsk->tx_bit == 0)
		{
			if (fifo_isempty(&afsk->tx_fifo) && afsk->trailer_len == 0)
			{
				DA_Stop();
				afsk->sending = false;
				return 0;
			}
			else
			{
				if (!afsk->bit_stuff) afsk->stuff_cnt = 0;

				afsk->bit_stuff = true;

				if (afsk->preamble_len == 0)
				{

					if (fifo_isempty(&afsk->tx_fifo))
					{
						afsk->trailer_len--;
						afsk->curr_out = HDLC_FLAG;
					}
					else
					{
						afsk->curr_out = fifo_pop(&afsk->tx_fifo);
					}
				}
				else
				{
					afsk->preamble_len--;
					afsk->curr_out = HDLC_FLAG;
				}

				if (afsk->curr_out == AX25_ESC)
				{
					if (fifo_isempty(&afsk->tx_fifo))
					{
						DA_Stop();
						afsk->sending = false;
						return 0;
					}
					else
					{
						afsk->curr_out = fifo_pop(&afsk->tx_fifo);
					}
				}
				else
				{
					if (afsk->curr_out == HDLC_FLAG || afsk->curr_out == HDLC_RESET)
					{
						afsk->bit_stuff = false;
					}
				}
			}

			afsk->tx_bit = 0x01;
		}

		if (afsk->bit_stuff && afsk->stuff_cnt >= BIT_STUFF_LEN)
		{
			afsk->stuff_cnt = 0;
			afsk->phase_inc = SWITCH_TONE(afsk->phase_inc);
		}
		else
		{
			if (afsk->curr_out & afsk->tx_bit)
			{
				afsk->stuff_cnt++;
			}
			else
			{
				afsk->stuff_cnt = 0;
				afsk->phase_inc = SWITCH_TONE(afsk->phase_inc);
			}

			afsk->tx_bit <<= 1;
		}

		afsk->sample_count = SAMPLEPERBIT;
	}

	afsk->phase_acc += afsk->phase_inc;
	afsk->phase_acc %= SIN_LEN;

	afsk->sample_count--;

	return sin_sample(afsk->phase_acc);

}


/*********************************************************************************************************************/
void AFSK_Init(Afsk *afsk) {
/*********************************************************************************************************************/


	uint8_t i;

	memset(afsk, 0, sizeof(*afsk));

	fifo_init(&afsk->rx_fifo, afsk->rx_buf, sizeof(afsk->rx_buf));
	fifo_init(&afsk->tx_fifo, afsk->tx_buf, sizeof(afsk->tx_buf));

	for (i=0;i<SAMPLEPERBIT;i++)
	{
		corr_mark_i[i] = 4095*cos(2*M_PI*i/SAMPLEPERBIT*FREQ_MARK/BITRATE);
		corr_mark_q[i] = 4095*sin(2*M_PI*i/SAMPLEPERBIT*FREQ_MARK/BITRATE);
		corr_space_i[i] = 4095*cos(2*M_PI*i/SAMPLEPERBIT*FREQ_SPACE/BITRATE);
		corr_space_q[i] = 4095*sin(2*M_PI*i/SAMPLEPERBIT*FREQ_SPACE/BITRATE);		 /// 2047 wszedzie bylo
	}

	afsk->phase_inc = MARK_INC;

}

	
