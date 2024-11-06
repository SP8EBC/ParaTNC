#include "./nvm/nvm_event.h"
#include "./nvm/nvm_internals.h"
#include "backup_registers.h"
#include "memory_map.h"
#include "nvm_configuration.h"
#include "crc_.h"

#include <string.h>		// for memset

#ifdef UNIT_TEST
#define STATIC
#else
#define STATIC static
#endif

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/**
 *
 */
typedef enum nvm_event_next_t {
	NVM_EVENT_NEXT_ERASED,	  ///!< There is no next element, the next one is just erased memory
	NVM_EVENT_NEXT_END,		  ///!< There is no next element, because the current one is the last
	NVM_EVENT_NEXT_OLDER,	  ///!< Next event is older that the current one
	NVM_EVENT_NEXT_NEWER,	  ///!< Next event is newer (later timestamp)
	NVM_EVENT_NEXT_TIMESTAMP, ///!< Next element is a timestamp event
	NVM_EVENT_NEXT_UNKNOWN/**< NVM_EVENT_NEXT_UNKNOWN */
} nvm_event_next_t;


/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================
/**
 * Variable stores a result of last flash operation
 */
static nvm_state_after_last_oper_t nvm_general_state = NVM_UNINITIALIZED;

/**
 * Amount of CRC errors after last call to @link{nvm_event_log_find_first_oldest_newest}
 */
static uint16_t nvm_event_crc_errors = 0;

NVM_EVENT_LOGGING_TARGETS (NVM_EVENT_CREATE_ENUM_FOR_TARGETS);

/**
 * Definition of all pointers, two of them per event logging target area, used
 * to find oldest and newest event
 */
NVM_EVENT_LOGGING_TARGETS (NVM_EVENT_CREATE_POINTERS_FOR_TARGET);

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

STATIC void nvm_event_erase_all(void *area_start, void *area_end, int16_t page_size) {
#if defined(STM32L471xx)

	const uint32_t area_size = (uint32_t) area_end - (uint32_t) area_start;

	const int16_t pages = area_size / page_size;

	FLASH_Unlock();

	for (int i = 0; i < pages; i++) {
		FLASH_ErasePage((uint32_t)area_start + (i * page_size));
	}

	FLASH_Lock();
#endif
}

/**
 * This function goes through whole event log area and finds the highest 
 * event ID used. Please note that it ignores any CRC errors (it doesn't
 * calculate CRC at all) and non consecutive id's outside page borders.
 * It is used only when a backward value of event_counter_id has been
 * found everywhere outside a boundary between two consecutive flash
 * areas. 
 */
STATIC uint32_t nvm_get_highest_event_id(void *area_start, void *area_end)
{
	uint32_t out = 0;
	// size of single log entry
	const uint8_t log_entry_size = sizeof (event_log_t);

	// how any events could be stored in NVM flash memory
	const uint32_t log_entries = (area_end - area_start + 1) / log_entry_size;

	// iterate through all event log flash area
	for (int i = 0; i < log_entries; i++) {
		// temp pointer to currently processed event
		const event_log_t *const current = ((const event_log_t *)area_start) + i;

		// skip all erased and non used entries
		if (current->event_counter_id != 0x0 && current->event_counter_id != 0xFFFFFFFF) {
			if (current->event_counter_id > out) {
				out = current->event_counter_id;
			}
		}
	}
}

/**
 * Function which solve a problem, when two consecutive event log entries
 * inside a flash memory page have descending counter_id, what normally
 * is not possible. What this function does is:
 * 	1. Go through all area and finds the highest value of event_counter_id
 *  2. Increment this highest value by 255 (better safe than sorry)
 *  3. Erase memory page next to one in which broken entry is stored
 *  4. Put EVENT_MARKER_BAD as the first entry in this erased flash memory page
 * 
 * @note: if the broken entry is stored in the last flash memory page in the
 * area. This memory page will be erased completely and no EVENT_MARKER_BAD
 * will be put. 
 * 
 * New event will be put just at the next location after EVENT_MARKER_BAD. This 
 * marker has a value of event_counter_id from 2nd point. It's role is to 
 * exclude previous page (this, where faulty entry is) from any other operation,
 * not to retrigger the same error once again. Eventually it will be erased anyway,
 * when the area will wrap up.
 * 
 * @note: This function assumes that broken entry has been written recently, probably
 * as the last event log entry persisted. Neither as a result of a software defect, 
 * and writing malformed event_id with correct CRC, or flash memory corruption which
 * somehow lead to corrupted entry, with the same CRC (LSB of CRC-32). It will not 
 * work well if flash memory starts to fail and some 
 * 
 * @param oldest pointer to the oldest entry found to the point of a fault
 * @param newest pointer to the newest entry, usually this should be the one before current_broken
 * @param area_start a pointer to first byte of an event log area
 * @param area_end a pointer to last byte (not byte after the last one!!!) of event log area
 * @param current_broken pointer to broken entry, which has descending counter_event_id value
 * @param erase_fn function used to erase flash memory page
 * @param page_size
 * @return
 */
STATIC uint32_t nvm_fix_broken_event_ids (event_log_t **oldest, event_log_t **newest, void *area_start,
									  void *area_end, event_log_t * current_broken,
									  FLASH_Status (*erase_fn) (uint32_t), int16_t page_size,
									  nvm_event_target_ereas_t target_area)
{
	uint32_t out = 0;

	// size of single log entry
	const uint8_t log_entry_size = sizeof (event_log_t);

	// how many entries single memory page fits
	const uint8_t one_page_capacity = page_size / log_entry_size;

	void* raw_ptr_to_broken = (void*) current_broken; 

	// check if broken entry is in the last flash memory page associated to entry log
	if (raw_ptr_to_broken + page_size >= area_end) {

		// erase last page of memory
		erase_fn(raw_ptr_to_broken);

		// set the pointer to virtual & non existent entry after the last
		// real entry in last real memory page
		// WARNING! This shall not be dereferenced, as it might even be located
		// outside existing memory space
		event_log_t* first_after_end = (event_log_t*)(area_end + 1);

		// set the newest location to 
		*newest = first_after_end - one_page_capacity;
	}
	else {
		// calculate offset measured by number of pages from start of the event log area
		uint16_t page_number_offset = NVM_EVENT_GET_PAGENUM_OFFSET(current_broken, area_start, page_size);

		// calculate pointer to first log entry in this memory page
		const event_log_t *const first_in_faulty_area =
			(const event_log_t *)(area_start + one_page_capacity * page_number_offset * sizeof(event_log_t));

		// calculate pointer to first entry in next area.
		const event_log_t * const first_in_next_area = first_in_faulty_area + one_page_capacity;

		// erase next area
		erase_fn(first_in_next_area);

		out = nvm_get_highest_event_id(area_start, area_end) + 255;
	}

	return out;
}

/**
 * Perform pointer arithmetics just before storing new event in target area. It always move 
 * newest pointer to a location at which this new event will be persisted. It also may change 
 * oldest pointer if an end of area is reached, or the area has rounder already, and old logs 
 * are overwritten. Some flash is erased in second and third cases
 * @param oldest
 * @param newest
 * @param area_start a pointer to first byte of an event log area
 * @param area_end a pointer to last byte (not byte after the last one!!!) of event log area
 * @param erase_fn
 */
static void nvm_event_log_perform_pointer_arithmetics (event_log_t **oldest,
													   event_log_t **newest,
													   void *area_start,
													   void *area_end,
													   uint32_t *next_event_counter_id,
													   FLASH_Status (*erase_fn) (uint32_t),
													   int16_t page_size,
													   uint16_t* area_percentage_use)
{

	// if memory is initialized but area is empty
	if (nvm_general_state == NVM_OK_AND_EMPTY) {

		// check if this area is actually empty (another, like RAM might be empty, while FLASH isn't)
		if ((*oldest == 0x0) && (*newest == 0x0)) {

			// set a pointer one element before area start. It will be moved forward in this function
			*newest = ((event_log_t*)area_start) - 1;
		}
	}

	*next_event_counter_id = (*newest)->event_counter_id + 1;

	if (*next_event_counter_id == 0xFFFFFFFFU || *next_event_counter_id == 0x0U) {
		*next_event_counter_id = 0x1U;
	}

	// pointers
	const event_log_t *oldest_init_ptr = *oldest;
	const event_log_t *next_newest_init_ptr = *newest + 1;


	/* check if we reach boundary between two flash memory pages */
	/* and the newest entry is just before the oldest pne  */
	if (next_newest_init_ptr == oldest_init_ptr) {
		/* erase next flash memory page to make a room for next events   */
		const FLASH_Status flash_status = erase_fn ((intptr_t)*oldest);

		/* check operation result */
		if (flash_status != FLASH_COMPLETE) {
			nvm_general_state = NVM_PGM_ERROR;
		}

		/* rescan for oldest and newest event one more time  */
		nvm_event_log_find_first_oldest_newest (oldest, newest, (void *)area_start,
												(void *)area_end, page_size, area_percentage_use);


		const uint8_t old_new_events_spacing = *oldest - *newest;

		/* oldest - newest should be located NVM_PAGE_SIZE bytes apart  */
		/* please note, that pointers points to the beginning of each  */
		/* entry, hence this minus one  */
		if ((old_new_events_spacing - 1) * sizeof (event_log_t) != NVM_PAGE_SIZE) {
			nvm_event_erase_all(area_start, area_end, page_size);
			backup_assert (BACKUP_REG_ASSERT_ERASE_FAIL_WHILE_STORING_EVENT);

		}

		/* move pointer to newest, to point to a place where  */
		/* newly inserted event will be located  */
		*newest = *(newest) + 1;
	}
	else if ((void *)next_newest_init_ptr >= (void *)area_end) {
		/* we have reached an end of the event area in flash  */

		/* erase first memory page  */
		(void)erase_fn ((intptr_t)area_start);

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
 */
void nvm_event_log_init(void)
{
	nvm_general_state = NVM_OK;

	NVM_EVENT_LOGGING_TARGETS(NVM_EVENT_PERFORM_INIT);

	if (nvm_general_state == NVM_GENERAL_ERROR) {
		nvm_event_erase_all((void*)MEMORY_MAP_EVENT_LOG_START, (void*)MEMORY_MAP_EVENT_LOG_END, NVM_PAGE_SIZE);
	}
}

/**
 * Gets current value of @link{nvm_event_crc_errors}
 */
uint16_t nvm_event_get_crc_errors(void)
{
	return nvm_event_crc_errors;
}

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
nvm_event_result_t nvm_event_log_find_first_oldest_newest (
	event_log_t **oldest, event_log_t **newest, void *area_start, void *area_end, int16_t page_size, uint16_t* area_percentage_use)
{

	nvm_event_result_t res = NVM_EVENT_OK;

	uint8_t sequence_number_error = 0u;

	uint32_t log_entries_counter = 0u;

	// size of single log entry
	const uint8_t log_entry_size = sizeof (event_log_t);

	// how any events could be stored in NVM flash memory
	const uint32_t log_entries = (area_end - area_start + 1) / log_entry_size;

#ifndef NVM_EVENT_PAGE_SIZE_CHECK_WITH_ADDRESS_PTR
	const int8_t page_size_in_events = page_size / log_entry_size;
#endif

	*area_percentage_use = 0;
	nvm_event_crc_errors = 0;

	// lowest date found within events in NVM
	uint32_t lowest_counter_id = 0xFFFFFFFFu;

	// highest counter id found within events in NVM
	uint32_t highest_counter_id = 0x0u;

	// as name suggest this is a counter value of last
	// event found
	uint32_t previous_counter_id = 0u;

	// sanity check if everything is set correctly
	if ((area_end - area_start + 1) % log_entry_size != 0) {
		return NVM_EVENT_AREA_ERROR;
	}

	// check if a page size if a multiplicity of event size 
	if (page_size % log_entry_size != 0) {
		return NVM_EVENT_AREA_ERROR;
	}	

	// iterate through all event log flash area
	for (int i = 0; i < log_entries; i++) {
		// continue only if there is no critical error
		if (res != NVM_EVENT_AREA_ERROR) 
		{
			// temp pointer to currently processed event
			const event_log_t *const current = ((const event_log_t *)area_start) + i;

			// do not go through erased flash memory on uninitialized RAM. The first valid counter
			// value is 1 and the last valid is 0xFFFFFFFE
			if (current->event_counter_id != 0x0u && current->event_counter_id != 0xFFFFFFFFu) {

				// calculate crc checksum for this entry
				const uint32_t crc = calcCRC32std(current, sizeof(event_log_t) - 1, 0x04C11DB7, 0xFFFFFFFF, 0, 0, 0);

				// skip entries with bad crc
				if ((uint8_t)(crc & 0xFF) != current->crc_checksum) 
				{
					// increase number of CRC errors
					nvm_event_crc_errors++;

					// checks if this is an entry just after newest one
					if ((*newest) + 1 == current) {
						// push the newest pointer forward to fix issues, when this 
						// corrupted entry will be the last one before erased 
						// space, which goes up to the end of flash event storage area.
						// while storing an event *newest pointer will be incremented
						// in @link{nvm_event_log_perform_pointer_arithmetics}
						*newest = (event_log_t*)current;
					}

					//continue;
				}
				else 
				{
					log_entries_counter++;

					// check if this event counter id value is lower than a previous one
					if (current->event_counter_id < previous_counter_id) {

						// address of next event element
						//const intptr_t address_of_current = (intptr_t)((void *)current);
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
							sequence_number_error = 1;

							res = NVM_EVENT_FIX_IDS;
						}
					}
					else {
						sequence_number_error = 0;
					}
				}
				// continue only if no critical error has been detected
				if (res != NVM_EVENT_AREA_ERROR && sequence_number_error == 0u)
				{
					previous_counter_id = current->event_counter_id;

					if (current->event_counter_id < lowest_counter_id) {
						lowest_counter_id = current->event_counter_id;

						// the smallest counter id is, the older this event is
						*oldest = (event_log_t*)current;
					}

					if (current->event_counter_id > highest_counter_id) {
						highest_counter_id = current->event_counter_id;

						// the higher counter id is, the newer this event is
						*newest = (event_log_t*)current;
					}
				}
			}
			else {
				// check if another fields are also in initiazed state or not
				if ((current->event_id != 0x0 && current->event_id != 0xFFu) ||
					(current->severity != 0x0 && current->severity != 0xFFu)) {
						// NVM event area is screwed very badly and cannot be recovered at all
						// it must be formatted and reinitialized from scratch
						res = NVM_EVENT_AREA_ERROR;
				}
			}
		}
	}

	// if these values have not been updated, the memory is in erased state
	if ((lowest_counter_id == 0xFFFFFFFFu) && (highest_counter_id == 0x0u)) {
		res = NVM_EVENT_EMPTY;
	}

	const float usage = (float)log_entries_counter / (float)log_entries;

	*area_percentage_use = (uint16_t)(usage * 100.0f);

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

	if (nvm_general_state == NVM_PGM_ERROR) {
		out = NVM_EVENT_ERROR;
	}
	else if (nvm_general_state == NVM_GENERAL_ERROR) {
		out = NVM_EVENT_ERROR;
	}

	return out;
}

/**
 * THis function walks through non volatile events storage area and returns no more than max_num_events
 * latest events, with severity level equal or greater than min_severity_lvl
 * @param output_arr a place where exported events will be placed
 * @param max_num_events
 * @param min_severity_lvl minimum severity level (inclusive) to be included in an export
 * @return
 */
nvm_event_result_stats_t nvm_event_get_last_events_in_exposed(event_log_exposed_t * output_arr, uint16_t max_num_events, event_log_severity_t min_severity_lvl)
{
	nvm_event_result_stats_t out = {0u};

	uint8_t current_lowest_severity = 0xFFu;
	
	event_log_t* newest = 0;
	event_log_t* area_start = 0;

	uint16_t output_arr_iterator = 0;

	NVM_EVENT_LOGGING_TARGETS(NVM_EVENT_FIND_LOWEST_SEVERITY);

	// check if any area has been found. this check may fail
	// only because of areas misconfiguration
	if (newest != 0 && area_start != 0) 
	{
		// make a room for exported events
		memset(output_arr, 0x00, sizeof(event_log_exposed_t) * max_num_events);

		// loop through selected area
		while (newest >= area_start) {

			// get severity level of currently processed event
			const event_log_severity_t severity = (event_log_severity_t)EVENT_LOG_GET_SEVERITY(newest->severity);

			// check if this event has appropriate severity level
			if (severity >= min_severity_lvl)
			{
				// if yes save it in output array
				output_arr[output_arr_iterator].event_counter_id = newest->event_counter_id;
				output_arr[output_arr_iterator].event_id = newest->event_id;
				output_arr[output_arr_iterator].event_master_time = newest->event_master_time;
				output_arr[output_arr_iterator].severity = severity;
				output_arr[output_arr_iterator].source = (event_log_source_t)EVENT_LOG_GET_SOURCE(newest->source);
				output_arr[output_arr_iterator].lparam = newest->lparam;
				output_arr[output_arr_iterator].lparam2 = newest->lparam2;
				output_arr[output_arr_iterator].wparam = newest->wparam;
				output_arr[output_arr_iterator].wparam2 = newest->wparam2;
				output_arr[output_arr_iterator].param = newest->param;
				output_arr[output_arr_iterator].param2 = newest->param2;


				// increment counter of all events processed
				out.zz_total++;

				// increment per severity counter
				switch (severity) {
					case EVENT_TIMESYNC: out.timesyncs++; break;
					case EVENT_ASSERT: out.asserts++; break;
					case EVENT_BOOTUP: out.bootups++; break;
					case EVENT_ERROR: out.errors++; break;
					case EVENT_WARNING: out.warnings++; break;
					case EVENT_INFO_CYCLIC: out.info_cyclic++; break;
					case EVENT_INFO: out.info++; break;
					default: break;
				}

				output_arr[output_arr_iterator].source_str_name = event_log_source_to_str(output_arr[output_arr_iterator].source);

				output_arr[output_arr_iterator].event_str_name = event_id_to_str(output_arr[output_arr_iterator].source, output_arr[output_arr_iterator].event_id);

				output_arr[output_arr_iterator].severity_str = event_log_severity_to_str(output_arr[output_arr_iterator].severity);

				// increment the iterator
				output_arr_iterator++;


			}
			// check if there is room left for more events to be stored in output array
			if (output_arr_iterator >= max_num_events)
			{
				break;		// exit the loop if there is no more room in output array
			}
			else
			{
				newest--;	// decrement pointer towards the beginning of area
			}

		}
	}

	return out;
}
