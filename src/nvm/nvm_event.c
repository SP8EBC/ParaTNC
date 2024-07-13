#include "./nvm/nvm_event.h"
#include "./nvm/nvm_internals.h"
#include "backup_registers.h"
#include "memory_map.h"
#include "nvm_configuration.h"

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

typedef enum nvm_event_next_t {
	NVM_EVENT_NEXT_ERASED,	  ///!< There is no next element, the next one is just erased memory
	NVM_EVENT_NEXT_END,		  ///!< There is no next element, because the current one is the last
	NVM_EVENT_NEXT_OLDER,	  ///!< Next event is older that the current one
	NVM_EVENT_NEXT_NEWER,	  ///!< Next event is newer (later timestamp)
	NVM_EVENT_NEXT_TIMESTAMP, ///!< Next element is a timestamp event
	NVM_EVENT_NEXT_UNKNOWN
} nvm_event_next_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================
/**
 * Variable stores a result of last flash operation
 */
static nvm_state_result_t nvm_general_state = NVM_UNINITIALIZED;

NVM_EVENT_LOGGING_TARGETS (NVM_EVENT_CREATE_ENUM_FOR_TARGETS);

/**
 * Definition of all pointers, two of them per event logging target area, used
 * to find oldest and newest event
 */
NVM_EVENT_LOGGING_TARGETS (NVM_EVENT_CREATE_POINTERS_FOR_TARGET);

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param oldest
 * @param newest
 * @param area_start
 * @param area_end
 * @param erase_fn
 */
static void nvm_event_log_perform_pointer_arithmetics (event_log_t **oldest,
													   event_log_t **newest,
													   void *area_start,
													   void *area_end,
													   uint32_t *next_event_counter_id,
													   FLASH_Status (*erase_fn) (uint32_t),
													   int16_t page_size)
{

	*next_event_counter_id = (*newest)->event_counter_id + 1;

	if (*next_event_counter_id == 0xFFFFFFFFU) {
		*next_event_counter_id = 0x1U;
	}

	// pointers
	const event_log_t *oldest_init_ptr = *oldest;
	const event_log_t *next_newest_init_ptr = *newest + 1;

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
		nvm_event_log_find_first_oldest_newest (oldest, newest, (void *)area_start,
												(void *)area_end, page_size);

		const uint8_t old_new_events_spacing = *oldest - *newest;

		/* oldest - newest should be located NVM_PAGE_SIZE bytes apart  */
		/* please note, that pointers points to the beginning of each  */
		/* entry, hence this minus one  */
		if ((old_new_events_spacing - 1) * sizeof (event_log_t) != NVM_PAGE_SIZE) {
			backup_assert (BACKUP_REG_ASSERT_ERASE_FAIL_WHILE_STORING_EVENT);
		}

		/* move pointer to newest, to point to a place where  */
		/* newly inserted event will be located  */
		*newest = *(newest) + 1;
	}
	else if ((void *)next_newest_init_ptr >= (void *)area_end) {
		/* we have reached an end of the event area in flash  */

		/* erase first memory page  */
		(void)erase_fn (area_start);

		/* set pointers accordingly  */
		event_log_t *new_newest = (event_log_t *)area_start;
		event_log_t *new_oldest = (event_log_t *)(area_end - sizeof (event_log_t));

		*newest = new_newest;
		*oldest = new_oldest;
	}
	else {
		*newest = *(newest) + 1;
	}
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_find_first_oldest_newest (
	event_log_t **oldest, event_log_t **newest, void *area_start, void *area_end, int16_t page_size)
{

	nvm_event_result_t res = NVM_EVENT_OK;

	// size of single log entry
	const uint8_t log_entry_size = sizeof (event_log_t);

	// how any events could be stored in NVM flash memory
	const uint16_t log_entries = (area_end - area_start) / log_entry_size;

#ifndef NVM_EVENT_PAGE_SIZE_CHECK_WITH_ADDRESS_PTR
	const int8_t page_size_in_events = page_size / log_entry_size;
#endif

	// lowest date found within events in NVM
	uint32_t lowest_counter_id = 0xFFFFFFFFu;

	// highest counter id found within events in NVM
	uint32_t highest_counter_id = 0x0u;

	// as name suggest this is a counter value of last
	// event found
	uint32_t previous_counter_id = 0u;

	// sanity check if everything is set correctly
	if ((area_end - area_start) % log_entry_size != 0) {
		return NVM_EVENT_AREA_ERROR;
	}

	// iterate through all event log flash area
	for (int i = 0; i < log_entries; i++) {
		// temp pointer to currently processed event
		const event_log_t *const current = ((const event_log_t *)area_start) + i;

		// do not go through erased flash memory on uninitialized RAM. The first valid counter
		// value is 1 and the last valid is 0xFFFFFFFE
		if (current->event_counter_id != 0x0u && current->event_counter_id != 0xFFFFFFFFu) {

			// check if this event counter id value is lower than a previous one
			if (current->event_counter_id < previous_counter_id) {

				// address of next event element
				const intptr_t address_of_current = (intptr_t)((void *)current);
#ifndef NVM_EVENT_PAGE_SIZE_CHECK_WITH_ADDRESS_PTR
				// check if this is a boundary of two flash areas
				if (page_size == 0 || (i % page_size_in_events) == 0) {
#else
				// check if this is a boundary of two flash areas
				if (page_size == 0 || (address_of_current % page_size) == 0) {
#endif
					// we are fine. event counter for consecutive events
					// can be decreasing on an edge of memory pages.
					// this check is also enabled if page_size is set to
					// zero, what is a case for RAM, freely accessible
					// for read and write in single byte granularity.
					// no explicit erasure is required
					res = NVM_EVENT_OVERRUN;
				}
				else {
					// NVM event area is screwed very badly and cannot be recovered at all
					// it must be formatted and reinitialized from scratch
					return NVM_EVENT_AREA_ERROR;
				}
			}

			previous_counter_id = current->event_counter_id;

			if (current->event_counter_id < lowest_counter_id) {
				lowest_counter_id = current->event_counter_id;

				// the smallest counter id is, the older this event is
				*oldest = current;
			}

			if (current->event_counter_id > highest_counter_id) {
				highest_counter_id = current->event_counter_id;

				// the higher counter id is, the newer this event is
				*newest = current;
			}
		}
	}

	// if these values have not been updated, the memory is in erased state
	if ((lowest_counter_id == 0xFFFFFFFFu) && (highest_counter_id == 0x0u)) {
		res = NVM_EVENT_EMPTY;
	}

	return res;
}

/**
 * @param event
 * @param oldest
 * @param newest
 */
nvm_event_result_t nvm_event_log_push_new_event (event_log_t *event)
{
	nvm_event_result_t out = NVM_EVENT_OK;

	NVM_EVENT_LOGGING_TARGETS (NVM_EVENT_EXPAND_POINTER_BASE_ACCESS);

	return out;
}
