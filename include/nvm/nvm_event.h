#ifndef B9059D46_61C3_45A2_A688_7297F71FC356
#define B9059D46_61C3_45A2_A688_7297F71FC356

#include "event_log.h"
#include "nvm_t.h"


/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void nvm_event_log_init(void);

/**
 *
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_find_first_oldest_newest(event_log_t **oldest, event_log_t **newest, void *area_start, void *area_end, int16_t page_size, uint16_t* area_percentage_use);


/**
 * @param event
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_push_new_event(event_log_t* event);

/**
 * THis function walks through non volatile events storage area and returns no more than max_num_events
 * latest events, with severity level equal or greater than min_severity_lvl
 * @param output_arr
 * @param max_num_events
 * @param min_severity_lvl
 * @return
 */
nvm_event_result_stats_t nvm_event_get_last_events_in_exposed(event_log_exposed_t * output_arr, uint16_t max_num_events, event_log_severity_t min_severity_lvl);

#endif /* B9059D46_61C3_45A2_A688_7297F71FC356 */
