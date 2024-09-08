/*
 * api.c
 *
 *  Created on: Apr 24, 2022
 *      Author: mateusz
 *
 *
 *      19 maja, Krszystof Binek, miedy 12 a 13
 */

#include <api/api.h>
#include <api/api_content.h>
#include "etc/api_configuration.h"

#include "http_client/http_client.h"

#include "stdint.h"
#include "aes.h"

/**
 * Buffers for generating JSON and URL
 */
#define API_BUFFER_LN	640
char api_buffer[API_BUFFER_LN];
#define URL_BUFFER_LN	96
char api_url_buffer[URL_BUFFER_LN];

/**
 * Index to move around a buffer with request body
 */
uint32_t api_buffer_idx = 0;

/**
 * Last value returned from http client
 */
uint8_t api_retval = 0xFF;

/**
 * Cycle counter to control the frequency of api calls
 */
int8_t api_cycle_counter = 0;

/**
 * Message authentication code encrypted by AES128 and converted to hex string
 */
char api_mac[33];

/**
 * Buffer to perform encryption of the MAC
 */
uint8_t api_aes_mac_buffer[16];

struct AES_ctx api_aes_context;

const uint8_t api_shared_secret[] = API_SHARED_SECRET;

const uint8_t api_iv[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/**
 * This is used to retrigger specific api communication in case of any request
 * appears on the same call to `api_pooler` function. For code simplicity only one
 * API request is possible in the single 'api_pooler' cycle
 */
#define API_TRIGGER_STATUS			(1 << 1)
#define API_TRIGGER_MEASUREMENTS	(1 << 2)
int8_t api_retrigger_api_call = 0;

/**
 * Pointers to base url and station name (stored within flash)
 */
const char * api_base_url;
const char * api_station_name;

typedef enum api_endpoint{

	PARAMETEO_STATUS,
	PARAMETEO_WX,
	PARAMETEO_EVENT
} api_endpoint_t;

#define	LN	api_buffer_idx
#define OUT api_buffer

static void api_construct_url_status(api_endpoint_t endpoint) {

	if (api_base_url != 0x00 && api_station_name != 0x00) {
		memset(api_url_buffer, 0x00, URL_BUFFER_LN);

		switch (endpoint) {
		case PARAMETEO_STATUS:
			snprintf(api_url_buffer, URL_BUFFER_LN - 1, "%s/parameteo/%s/status/v1", api_base_url, api_station_name);
			break;
		case PARAMETEO_WX:
			snprintf(api_url_buffer, URL_BUFFER_LN - 1, "%s/parameteo/%s/measurements/v1", api_base_url, api_station_name);
			break;
		case PARAMETEO_EVENT:
			snprintf(api_url_buffer, URL_BUFFER_LN - 1, "%s/parameteo/%s/event/v1", api_base_url, api_station_name);
			break;
		}
	}

}

static void api_callback(uint16_t http_code, char * content, uint16_t content_lenght) {

	http_client_close();


}

void api_init(const char * api_base, const char * station_name) {
	api_base_url = api_base;
	api_station_name = station_name;
	AES_init_ctx_iv(&api_aes_context, api_shared_secret, api_iv);
	memset(api_mac, 0x00, 33);
	memset(api_mac, '0', 32);
}


void api_calculate_mac(void) {

	// iterators used during conversion to hex string after encryption
	int i = 0, j = 0;

	memset(api_aes_mac_buffer, 0x00, 16);
	memset(api_mac, 0x00, 33);

	api_aes_mac_buffer[0] = (uint8_t)((master_time & 0xFF));
	api_aes_mac_buffer[1] = (uint8_t)((master_time & 0xFF00) >> 8);
	api_aes_mac_buffer[2] = (uint8_t)((master_time & 0xFF0000) >> 16);
	api_aes_mac_buffer[3] = (uint8_t)((master_time & 0xFF000000) >> 24);
	api_aes_mac_buffer[4] = (uint8_t)((rte_main_average_battery_voltage & 0xFF));
	api_aes_mac_buffer[5] = (uint8_t)((rte_main_average_battery_voltage & 0xFF00) >> 8);
	api_aes_mac_buffer[6] = (uint8_t)((rte_wx_average_winddirection & 0xFF));
	api_aes_mac_buffer[7] = (uint8_t)((rte_wx_average_winddirection & 0xFF00) >> 8);
	api_aes_mac_buffer[8] = (uint8_t)((rte_wx_average_windspeed & 0xFF));
	api_aes_mac_buffer[9] = (uint8_t)((rte_wx_average_windspeed & 0xFF00) >> 8);
	api_aes_mac_buffer[10] = (uint8_t)((rte_wx_max_windspeed & 0xFF));
	api_aes_mac_buffer[11] = (uint8_t)((rte_wx_max_windspeed & 0xFF00) >> 8);
	api_aes_mac_buffer[12] = (uint8_t)((rte_wx_pm2_5 & 0xFF));
	api_aes_mac_buffer[13] = (uint8_t)((rte_wx_pm2_5 & 0xFF00) >> 8);
	api_aes_mac_buffer[14] = (uint8_t)((rte_wx_temperature_average_dallas & 0xFF));
	api_aes_mac_buffer[15] = (uint8_t)((rte_wx_temperature_average_dallas & 0xFF00) >> 8);

	AES_CBC_encrypt_buffer(&api_aes_context, api_aes_mac_buffer, 16);

	for (i = 0; i < 16; i++) {
		snprintf(api_mac + j, 3, "%02X", api_aes_mac_buffer[i]);
		j += 2;
	}

}

void api_send_json_status(void) {
	BEGIN
	PRINT_ALL_STATUS
	END

	if (api_buffer_idx < API_BUFFER_LN) {
		api_construct_url_status(PARAMETEO_STATUS);

		api_retval = http_client_async_post(api_url_buffer, strlen(api_url_buffer), OUT, strlen(OUT), 0, api_callback);
	}
}

void api_send_json_measuremenets(void) {
	BEGIN
	PRINT_ALL_MEASUREMENTS
	END

	if (api_buffer_idx < API_BUFFER_LN) {
		api_construct_url_status(PARAMETEO_WX);

		api_retval = http_client_async_post(api_url_buffer, strlen(api_url_buffer), OUT, strlen(OUT), 0, api_callback);
	}
}

uint8_t api_send_json_event(const event_log_exposed_t * event) {
	BEGIN
	PRINT_STRING(api_station_name, station_name);
	PRINT_32INT(event->event_master_time, event_master_time);
	PRINT_32INT(event->event_counter_id, event_counter_id);
	PRINT_STRING(event->severity_str, severity);
	PRINT_STRING(event->source_str_name, source);
	PRINT_STRING(event->event_str_name, event);
	PRINT_16INT(event->param, param);
	PRINT_16INT(event->param2, param2);
	PRINT_16INT(event->wparam, wparam);
	PRINT_16INT(event->wparam2, wparam2);
	PRINT_32INT(event->lparam, lparam);
	PRINT_32INT(event->lparam2, lparam2);
	END

	if (api_buffer_idx < API_BUFFER_LN) {
		api_construct_url_status(PARAMETEO_EVENT);

		api_retval = http_client_async_post(api_url_buffer, strlen(api_url_buffer), OUT, strlen(OUT), 0, api_callback);
	}

	return api_retval;
}


