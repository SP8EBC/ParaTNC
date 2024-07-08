#ifndef B9059D46_61C3_45A2_A688_7297F71FC356
#define B9059D46_61C3_45A2_A688_7297F71FC356

#include "nvm_internals.h"
#include "event_log.h"

/**
 *
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_find_first_oldest_newest(event_log_t** oldest, event_log_t** newest);

/**
 * @param event
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_push_new_event(event_log_t* event, event_log_t** oldest, event_log_t** newest);


#endif /* B9059D46_61C3_45A2_A688_7297F71FC356 */
