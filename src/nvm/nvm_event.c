#include "./nvm/nvm_event.h"
#include "./nvm/nvm_internals.h"
#include "nvm_configuration.h"
#include "memory_map.h"
#include "backup_registers.h"



static nvm_state_result_t nvm_general_state = NVM_UNINITIALIZED;


NVM_EVENT_LOGGING_TARGETS(NVM_EVENT_CREATE_POINTERS_FOR_TARGET);

static void nvm_event_log_perform_pointer_arithmetics(
								event_log_t** oldest, 
								event_log_t** newest, 
								void * area_start, 
								void * area_end,
								FLASH_Status(*erase_fn)(uint32_t)) {

	// pointers
	const event_log_t* oldest_init_ptr 			= *oldest;
	const event_log_t* next_newest_init_ptr 	= *newest + 1;

	/* check if we reach boundary between two flash memory pages */
	/* and the newest entry is just before the oldest pne  */
	if (next_newest_init_ptr == oldest_init_ptr) {
		/* erase next flash memory page to make a room for next events   */
		const FLASH_Status flash_status = erase_fn (*oldest);

		/* check operation result */
		if (flash_status != FLASH_COMPLETE) {
			nvm_general_state = NVM_PGM_ERROR;
		}

		/* rescan for oldest and newest event one more time  */
		nvm_event_log_find_first_oldest_newest(oldest, newest, (void*)area_start, (void*)area_end);

		const uint8_t old_new_events_spacing = *oldest - *newest;

		/* oldest - newest should be located NVM_PAGE_SIZE bytes apart  */
		/* please note, that pointers points to the beginning of each  */
		/* entry, hence this minus one  */
		if ((old_new_events_spacing - 1) * sizeof(event_log_t) != NVM_PAGE_SIZE) {
			backup_assert(BACKUP_REG_ASSERT_ERASE_FAIL_WHILE_STORING_EVENT);
		}

		/* move pointer to newest, to point to a place where  */
		/* newly inserted event will be located  */
		*newest = *(newest) + 1;
	}
	else if ((void*)next_newest_init_ptr >= (void*)area_end) {
		/* we have reached an end of the event area in flash  */

		/* erase first memory page  */
		(void)erase_fn (area_start);

		/* set pointers accordingly  */
		event_log_t* new_newest = (event_log_t*)area_start;
		event_log_t* new_oldest = (event_log_t*)(area_end - sizeof(event_log_t));

		*newest = new_newest;
		*oldest = new_oldest;
	}
	else {
		*newest = *(newest) + 1;
	}									
}

/**
 *
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_find_first_oldest_newest(event_log_t** oldest, event_log_t** newest, void * area_start, void * area_end) {

	nvm_event_result_t res = NVM_EVENT_OK;

	// pointer to last, non null and non TIMESYNC entry
	event_log_t* last_non_ts = 0x0;

	// pointer to the oldest non TIMESYNC event log entry
	event_log_t* oldest_non_ts = 0x0;

	// size of single log entry
	const uint8_t log_entry_size = sizeof(event_log_t);

	// how any events could be stored in NVM flash memory
	const uint16_t log_entries = (area_end - area_start) / log_entry_size;

	// lowest date found within events in NVM
	uint32_t lowest_date = 0xFFFFFFFFu;

	uint32_t lowest_time = 0xFFFFFFFFu;

	// sanity check if everything is set correctly
	if ((area_end - area_start) % log_entry_size != 0 ) {
		return NVM_EVENT_AREA_ERROR;
	}

	last_non_ts = 		(event_log_t *)area_start;

	// iterate through all event log flash area
	for (int i = 0; i < log_entries; i++) {

		// set pointer to currently checked event
		const event_log_t* const current = (area_start + (log_entry_size) * i);

		//event_log_severity_t severity = (current->severity_and_source & 0xF0) >> 4;
		//event_log_source_t source = (current->severity_and_source & 0xF);

		// skip erased memory
		if (current->event_id == 0xFFU && current->event_master_time == 0xFFFFFFFFU) {
			oldest_non_ts = 0x00;
			continue;
		}

		// look for timesync event created at bootup
		if (current->event_id == EVENT_TIMESYNC && current->wparam == EVENT_LOG_TIMESYNC_BOOTUP_WPARAM) {

			// check if this timestamp is before the oldest found before
			if (lowest_date > current->lparam && lowest_time > current->lparam2) {

				// set this as the oldest
				lowest_date = current->lparam;
				lowest_time = current->lparam2;

				// timestamp are always created after the first one after power up, so that
				// with oldest RTC date and time will be the oldest in general
				*oldest = (event_log_t*)current;
			}
		}
		else {
			if (current->event_master_time > last_non_ts->event_master_time) {
				// store a pointer to last non-null and non-timesync event
				last_non_ts = (event_log_t*)current;

				// updated output pointer with newest 
				*newest = last_non_ts;
			}
			else {
				// this loop goes forward in memory. if consecutive non timesync event
				// has decreasing master_time value it means, that nvm events area 
				// has overruned at least one time 
				res = NVM_EVENT_OVERRUN;

				if (oldest_non_ts == 0x0) {
					oldest_non_ts = (event_log_t*)current;
				}
			}
		}
	}

	// check if any non-timesync event has been found at all
	if (last_non_ts == 0x0) {
		// no, NVM log contains only single timesync event
		res = NVM_EVENT_SINGLE_TS;
	}

	// check if any timesync event has been found
	if (lowest_date == 0xFFFFFFFFu && lowest_time == 0xFFFFFFFF) {
		if (last_non_ts == (event_log_t *)area_start) {
			res = NVM_EVENT_EMPTY;	// nvm event area is empty
		}
		else {
			*oldest = oldest_non_ts;
			res = NVM_EVENT_OVERRUN_NO_TS;
		}
	}

	return res;

}

/**
 * @param event
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_push_new_event(event_log_t* event) {
	nvm_event_result_t out = NVM_EVENT_OK;


	NVM_EVENT_LOGGING_TARGETS(NVM_EVENT_EXPAND_POINTER_BASE_ACCESS);


	return out;
}
