/*
 * ntp.c
 *
 *  Created on: Aug 17, 2024
 *      Author: mateusz
 */

#include "ntp.h"

#include "gsm/sim800c_tcpip.h"
#include "tm/tm_stm32_rtc.h"

#include "stm32l4xx_ll_rtc.h"

#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define NTP_PACKET_SIZE 48

#define NTP_TXTM_S_OFFSET (NTP_PACKET_SIZE - sizeof (uint32_t) * 2)

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

typedef struct {

	uint8_t li_vn_mode; // Eight bits. li, vn, and mode.
						// li.   Two bits.   Leap indicator.
						// vn.   Three bits. Version number of the protocol.
						// mode. Three bits. Client will pick mode 3 for client.

	uint8_t stratum;   // Eight bits. Stratum level of the local clock.
	uint8_t poll;	   // Eight bits. Maximum interval between successive messages.
	uint8_t precision; // Eight bits. Precision of the local clock.

	uint32_t rootDelay;		 // 32 bits. Total round trip delay time.
	uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
	uint32_t refId;			 // 32 bits. Reference clock identifier.

	uint32_t refTm_s; // 32 bits. Reference time-stamp seconds.
	uint32_t refTm_f; // 32 bits. Reference time-stamp fraction of a second.

	uint32_t origTm_s; // 32 bits. Originate time-stamp seconds.
	uint32_t origTm_f; // 32 bits. Originate time-stamp fraction of a second.

	uint32_t rxTm_s; // 32 bits. Received time-stamp seconds.
	uint32_t rxTm_f; // 32 bits. Received time-stamp fraction of a second.

	uint32_t txTm_s; // 32 bits and the most important field the client cares about. Transmit
					 // time-stamp seconds.
	uint32_t txTm_f; // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet_t; // Total: 384 bits or 48 bytes.

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

//!< Context of a serial port used to communicate with GSM module
static srl_context_t *ntp_serial_context;

//!< Current GSM module status
static gsm_sim800_state_t *ntp_gsm_modem_state;

//!< Content of a NTP packet received or sent to a time server
static ntp_packet_t ntp_packet;

//!< Pointer to a NTP packet used to operate as a byte array
static uint8_t *ntp_packet_buffer;

//!< NTP timestamp decoded to local date and time
static TM_RTC_t ntp_decoded_timedate = {0};

static LL_RTC_TimeTypeDef ntp_rtc_time;

static LL_RTC_DateTypeDef ntp_rtc_date;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

//!< Set to one after a updated time and date has been received from a NTP server
uint8_t ntp_done = 0;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

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

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Initialize NTP client
 * @param context
 * @param gsm_modem_state
 */
void ntp_init (srl_context_t *context, gsm_sim800_state_t *gsm_modem_state)
{
	ntp_serial_context = context;

	ntp_gsm_modem_state = gsm_modem_state;

	ntp_packet_buffer = (uint8_t *)&ntp_packet;


}

/**
 * Synchronously connect to NTP server, get current UTC timestamp and set RTC
 * to it.
 */
void ntp_get_sync (void)
{
	memset (ntp_packet_buffer, 0, sizeof (ntp_packet_t));

	// Initialize values needed to form NTP request
	ntp_packet_buffer[0] = 0xE3; // 0b11100011;   // LI, Version, Mode
	ntp_packet_buffer[1] = 0x00; // Stratum, or type of clock
	ntp_packet_buffer[2] = 0x06; // Polling Interval
	ntp_packet_buffer[3] = 0xEC; // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	ntp_packet_buffer[12] = 49;
	ntp_packet_buffer[13] = 0x4E;
	ntp_packet_buffer[14] = 49;
	ntp_packet_buffer[15] = 52;

	const sim800_return_t connect_status =
		gsm_sim800_tcpip_connect ("tempus1.gum.gov.pl", strlen ("tempus1.gum.gov.pl"), "123", 3,
								  ntp_serial_context, ntp_gsm_modem_state, 1);

	if (SIM800_OK == connect_status) {
		const sim800_return_t write_result = gsm_sim800_tcpip_write (
			ntp_packet_buffer, NTP_PACKET_SIZE, ntp_serial_context, ntp_gsm_modem_state);

		if (SIM800_OK == write_result) {
			memset (ntp_packet_buffer, 0x00, NTP_PACKET_SIZE);

			const sim800_return_t receive_result =
				gsm_sim800_tcpip_receive (ntp_packet_buffer, NTP_PACKET_SIZE, ntp_serial_context,
										  ntp_gsm_modem_state, ntp_rx_done_callback, 2048);

			gsm_sim800_tcpip_close (ntp_serial_context, ntp_gsm_modem_state, 0);

			if (receive_result == SIM800_OK) {
				// NTP server returns data in big-endian format, so it is needed t convert it to le
				const uint32_t le_ntp_timestamp = ntp_packet_buffer[NTP_TXTM_S_OFFSET] << 24 |
												  ntp_packet_buffer[NTP_TXTM_S_OFFSET + 1] << 16 |
												  ntp_packet_buffer[NTP_TXTM_S_OFFSET + 2] << 8  |
												  ntp_packet_buffer[NTP_TXTM_S_OFFSET + 3];

				const uint32_t epoch = (le_ntp_timestamp - 2208988800UL);

				(void)TM_RTC_GetDateTimeFromUnix (&ntp_decoded_timedate, epoch);

				ntp_rtc_time.TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24;
				ntp_rtc_time.Hours = ntp_decoded_timedate.Hours;
				ntp_rtc_time.Minutes = ntp_decoded_timedate.Minutes;
				ntp_rtc_time.Seconds = ntp_decoded_timedate.Seconds;

				ntp_rtc_date.Day = ntp_decoded_timedate.Day;
				ntp_rtc_date.Month = ntp_decoded_timedate.Month;
				ntp_rtc_date.Year = ntp_decoded_timedate.Year;
				ntp_rtc_date.WeekDay = ntp_decoded_timedate.WeekDay;

				// enable access to backup domain
				PWR->CR1 |= PWR_CR1_DBP;

				LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &ntp_rtc_date);
				LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &ntp_rtc_time);

				PWR->CR1 &= (0xFFFFFFFFu ^ PWR_CR1_DBP);

				ntp_done = 1;
			}
		}
	}
}
