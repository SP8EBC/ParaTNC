/*
 * api.c
 *
 *  Created on: Apr 24, 2022
 *      Author: mateusz
 *
 *
 *      19 maja, Krszystof Binek, miedy 12 a 13
 */

#include <api/api_content.h>
#include "etc/api_configuration.h"

#include "http_client/http_client.h"

#include "stdint.h"

/**
 * Buffers for generating JSON and URL
 */
#define API_BUFFER_LN	512
char api_buffer[API_BUFFER_LN];
#define URL_BUFFER_LN	96
char api_url_buffer[URL_BUFFER_LN];

/**
 * Index to move around a buffer with request body
 */
uint32_t api_buffer_idx = 0;

/**
 * Value returned from http client
 */
uint8_t api_retval = 0xFF;

/**
 * Cycle counter to control the frequency of api calls
 */
int8_t api_cycle_counter = 0;

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
	PARAMETEO_WX
} api_endpoint_t;

#define	LN	api_buffer_idx
#define OUT api_buffer

static void api_construct_url_status(api_endpoint_t endpoint) {

	if (api_base_url != 0x00 && api_station_name != 0x00) {
		memset(api_url_buffer, 0x00, URL_BUFFER_LN);

		switch (endpoint) {
		case PARAMETEO_STATUS:
			snprintf(api_url_buffer, URL_BUFFER_LN - 1, "%s/parameteo/%s/status", api_base_url, api_station_name);
			break;
		case PARAMETEO_WX:
			snprintf(api_url_buffer, URL_BUFFER_LN - 1, "%s/parameteo/%s/measurements", api_base_url, api_station_name);
			break;
		}
	}

}

static void api_callback(uint16_t http_code, char * content, uint16_t content_lenght) {


}

void api_init(const char * api_base, const char * station_name) {
	api_base_url = api_base;
	api_station_name = station_name;
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


