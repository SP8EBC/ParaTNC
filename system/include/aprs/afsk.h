#ifndef AFSK_H_
#define AFSK_H_

//
//#ifndef CCC
//#include "stm32f10x_conf.h"
//#endif

#include <ax25_config.h>
#include <stdbool.h>

#include "cfifo.h"
#include "macros.h"



#define FREQ_MARK  1200
#define FREQ_SPACE 2200

#define SAMPLERATE 9600
#define BITRATE    1200

#define PHASE_BIT    8
#define PHASE_INC    1


#define SAMPLEPERBIT (SAMPLERATE / BITRATE)

#define PHASE_MAX    (SAMPLEPERBIT * PHASE_BIT)
#define PHASE_THRES  (PHASE_MAX / 2)

// Modulator constants
#define SIN_LEN 512 ///< Full wave length
#define	BIT_STUFF_LEN 5



#define MARK_INC   (uint16_t)(DIV_ROUND(SIN_LEN * (uint32_t)FREQ_MARK, SAMPLERATE))
#define SPACE_INC  (uint16_t)(DIV_ROUND(SIN_LEN * (uint32_t)FREQ_SPACE, SAMPLERATE))



/**
 * RX FIFO buffer full error.
 */
#define AFSK_RXFIFO_OVERRUN BV(0)



/**
 * HDLC (High-Level Data Link Control) context.
 * Maybe to be moved in a separate HDLC module one day.
 */
typedef struct Hdlc
{

	uint8_t demod_bits; ///< Bitstream from the demodulator.
	uint8_t bit_idx;    ///< Current received bit.
	uint8_t currchar;   ///< Current received character.
	bool rxstart;       ///< True if an HDLC_FLAG char has been found in the bitstream.
	bool raw_dcd;
	bool s;
	bool raw_mode;

} Hdlc;

typedef struct Afsk
{


	uint16_t max_value;

	/** Current sample of bit for output data. */
	uint8_t sample_count;

	/** Current character to be modulated */
	uint8_t curr_out;

	/** Mask of current modulated bit */
	uint8_t tx_bit;

	/** True if bit stuff is allowed, false otherwise */
	bool bit_stuff;

	/** Counter for bit stuffing */
	uint8_t stuff_cnt;

	/**
	 * DDS phase accumulator for generating modulated data.
	 */
	uint16_t phase_acc;

	/** Current phase increment for current modulated bit */
	uint16_t phase_inc;

	/**
	 * Bits sampled by the demodulator are here.
	 * Since ADC samplerate is higher than the bitrate, the bits here are
	 * SAMPLEPERBIT times the bitrate.
	 */
	uint8_t sampled_bits;

	/**
	 * Current phase, needed to know when the bitstream at ADC speed
	 * should be sampled.
	 */
	int8_t curr_phase;

	/** Bits found by the demodulator at the correct bitrate speed. */
	uint8_t found_bits;

	/** FIFO for received data */
	FIFOBuffer rx_fifo;

	/** FIFO rx buffer */
	uint8_t rx_buf[CONFIG_AFSK_RX_BUFLEN];

	/** FIFO for transmitted data */
	FIFOBuffer tx_fifo;

	/** FIFO tx buffer */
	uint8_t tx_buf[CONFIG_AFSK_TX_BUFLEN];

	/** True while modem sends data */
	volatile bool sending;

	/**
	 * AFSK modem status.
	 * If 0 all is ok, otherwise errors are present.
	 */
	volatile int status;

	/** Hdlc context */
	Hdlc hdlc;

	/**
	 * Preamble length.
	 * When the AFSK modem wants to send data, before sending the actual data,
	 * shifts out preamble_len HDLC_FLAG characters.
	 * This helps to synchronize the demodulator filters on the receiver side.
	 */
	uint16_t preamble_len;

	/**
	 * Trailer length.
	 * After sending the actual data, the AFSK shifts out
	 * trailer_len HDLC_FLAG characters.
	 * This helps to synchronize the demodulator filters on the receiver side.
	 */
	uint16_t trailer_len;


} Afsk;
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************************************************************/
void AFSK_ADC_ISR(Afsk *afsk, int16_t curr_sample);
/*********************************************************************************************************************/

void afsk_txStart(Afsk *af);

/*********************************************************************************************************************/
uint8_t AFSK_DAC_ISR(Afsk *afsk);
/*********************************************************************************************************************/

/*********************************************************************************************************************/
void AFSK_Init(Afsk *afsk);
/*********************************************************************************************************************/
#ifdef __cplusplus
}
#endif	

#endif /* AFSK_H_ */
