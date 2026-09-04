/*
 * event_log_to_string.h
 *
 *  Created on: Sep 4, 2026
 *      Author: mateusz
 */

#ifndef EVENT_LOG_TO_STRING_H_
#define EVENT_LOG_TO_STRING_H_

#include "event_log_t.h"

const char *event_log_severity_to_str (event_log_severity_t severity);

/**
 * Returns a pointer to a string representing event source
 * @param src
 * @return
 */
const char *event_log_source_to_str (event_log_source_t src);

/**
 *
 * @param source
 * @param event_id
 * @return
 */
const char *event_id_to_str (event_log_source_t source, uint8_t event_id);

/**
 * Generates string representation of given event log in exposed form
 * @param exposed pointer to an event to be converted
 * @param output char buffer to place a string into
 * @param output_ln maximum length of output string
 * @return length of assembled string
 */
uint16_t event_exposed_to_string (const event_log_exposed_t *exposed, char *output,
								  uint16_t output_ln);



#endif /* EVENT_LOG_TO_STRING_H_ */
