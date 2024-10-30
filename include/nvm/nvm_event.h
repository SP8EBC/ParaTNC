#ifndef B9059D46_61C3_45A2_A688_7297F71FC356
#define B9059D46_61C3_45A2_A688_7297F71FC356

#include "event_log.h"
#include "nvm_t.h"

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

#define NVM_EVENT_GET_PAGENUM_OFFSET(event_address, area_start, page_size)							\
		((void*) event_address - (void*)area_start)	/ (uint32_t)page_size										\


/// ==================================================================================================
///	GLOBAL TYPES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 */
void nvm_event_log_init (void);

/**
 * Gets current value of @link{nvm_event_crc_errors}
 */
uint16_t nvm_event_get_crc_errors (void);

/**
 * Scans event log area to find the oldest and the newest entry
 * @param oldest a pointer to a pointer in which an address of oldest entry will be stored
 * @param newest a pointer to a pointer in which an address of newest entry will be stored
 * @param area_start a pointer to first byte of an event log area
 * @param area_end a pointer to last byte (not byte after the last one!!!) of event log area
 * @param page_size page size
 * @param area_percentage_usage a pointer to uint16_t variable where this function will put
 * percentage usage
 */
nvm_event_result_t nvm_event_log_find_first_oldest_newest (event_log_t **oldest,
														   event_log_t **newest, 
                                                           void *area_start,
														   void *area_end, 
                                                           int16_t page_size,
														   uint16_t *area_percentage_use);

/**
 * @param event
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_push_new_event (event_log_t *event);

/**
 * THis function walks through non volatile events storage area and returns no more than
 * max_num_events latest events, with severity level equal or greater than min_severity_lvl
 * @param output_arr
 * @param max_num_events
 * @param min_severity_lvl
 * @return
 */
nvm_event_result_stats_t
nvm_event_get_last_events_in_exposed (event_log_exposed_t *output_arr, uint16_t max_num_events,
									  event_log_severity_t min_severity_lvl);

#endif /* B9059D46_61C3_45A2_A688_7297F71FC356 */
