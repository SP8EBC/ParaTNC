/*
 * ntp.c
 *
 *  Created on: Aug 17, 2024
 *      Author: mateusz
 */

#include "gsm/sim800c_tcpip.h"
#include "ntp.h"

#include <string.h>

static srl_context_t *ntp_serial_context;
static gsm_sim800_state_t *ntp_gsm_modem_state;

#define NTP_PACKET_SIZE 48

typedef struct
{

  uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                           // li.   Two bits.   Leap indicator.
                           // vn.   Three bits. Version number of the protocol.
                           // mode. Three bits. Client will pick mode 3 for client.

  uint8_t stratum;         // Eight bits. Stratum level of the local clock.
  uint8_t poll;            // Eight bits. Maximum interval between successive messages.
  uint8_t precision;       // Eight bits. Precision of the local clock.

  uint32_t rootDelay;      // 32 bits. Total round trip delay time.
  uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
  uint32_t refId;          // 32 bits. Reference clock identifier.

  uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
  uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

  uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
  uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

  uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
  uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

  uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
  uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total: 384 bits or 48 bytes.

ntp_packet packet;

uint8_t * packetBuffer;

uint8_t ntp_done = 0;

static uint8_t ntp_rx_done_callback (uint8_t current_data,
									 const uint8_t *const rx_buffer,
									 uint16_t rx_bytes_counter)
{
	if (rx_bytes_counter == NTP_PACKET_SIZE) {
		return 1;
	}
	else {
		return 0;
	}
}

/**
 *
 * @param context
 * @param gsm_modem_state
 */
void ntp_init (srl_context_t *context, gsm_sim800_state_t *gsm_modem_state)
{
	ntp_serial_context = context;

	ntp_gsm_modem_state = gsm_modem_state;

	packetBuffer = (uint8_t *)&packet;
}

/**
 *
 */
void ntp_get_sync (void)
{
	memset (packetBuffer, 0, sizeof(ntp_packet));

	// Initialize values needed to form NTP request
	packetBuffer[0] = 0xE3; // 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0x00; // Stratum, or type of clock
	packetBuffer[2] = 0x06; // Polling Interval
	packetBuffer[3] = 0xEC; // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	const sim800_return_t connect_status =
		gsm_sim800_tcpip_connect ("tempus1.gum.gov.pl", strlen ("tempus1.gum.gov.pl"), "123", 3,
								  ntp_serial_context, ntp_gsm_modem_state, 1);

	if (SIM800_OK == connect_status) {
		const sim800_return_t write_result = gsm_sim800_tcpip_write (
			packetBuffer, NTP_PACKET_SIZE, ntp_serial_context, ntp_gsm_modem_state);

		if (SIM800_OK == write_result) {
			memset (packetBuffer, 0x00, NTP_PACKET_SIZE);

			const sim800_return_t receive_result =
				gsm_sim800_tcpip_receive (packetBuffer, NTP_PACKET_SIZE, ntp_serial_context,
										  ntp_gsm_modem_state, ntp_rx_done_callback, 2048);

			gsm_sim800_tcpip_close (ntp_serial_context, ntp_gsm_modem_state, 0);

			if (receive_result == SIM800_OK) {
				ntp_done = 1;
			}
		}
	}
}
