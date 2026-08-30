#ifndef B9059D46_61C3_45A2_A688_7297F71FC356
#define B9059D46_61C3_45A2_A688_7297F71FC356

#include "event_log.h"
#include "nvm_t.h"

#include "drivers/serial.h"

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

// clang-format off

#define NVM_EVENT_GET_PAGENUM_OFFSET(event_address, area_start, page_size)							\
		((void*) event_address - (void*)area_start)	/ (uint32_t)page_size

// clang-format on
/// ==================================================================================================
///	GLOBAL TYPES
/// ==================================================================================================

/**
 * FIFO queue of log entries to be transmitted over UART to host PC. Worker function periodically
 * check if *tail, last element sent in previous transaction is different than  **newest
 * pointer, the last element added into the log.  If there is a difference it means that
 * at least one new entry has been logged in between consecutive call to the worker.
 * Then *head is set to element next to *tail and *tail is set to **newest.
 * Everything what is in-between head and tail is transmitted via UART
 */
typedef struct nvm_event_log_fifo_t {
	event_log_t *start;	  //!< First (lower address) log entry in the area
	event_log_t *end;	  //!< Last (highest address) log entry in the area
	event_log_t *head;	  //!< First element send via UART in previous transaction
	event_log_t *tail;	  //!< Last element send via UART in previous transaction
	event_log_t **oldest; //!< Pointer to pointer to Oldest log entry in the area
	event_log_t **newest; //!< Pointer to pointer to Newest log entry in the area. If this is
						  //!< different than tail
} nvm_event_log_fifo_t;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Initializes everything logging related. Fill array of FIFOs with start (lowest addr) and
 * end of (highest addr) of every configured log storage area
 * @param fifo_arr pointer to an array of FIFOs
 * @param fifo_arr_capacity how many fifos is pointed by
 */
void nvm_event_log_init (nvm_event_log_fifo_t *fifo_arr, uint8_t fifo_arr_capacity);

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
														   event_log_t **newest, void *area_start,
														   void *area_end, int16_t page_size,
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

/**
 * Pushes a packet of event logs entries via USART port to host PC basing
 * on FIFO queue set on a storage.
 * @param fifo of events
 * @param serial_port to be used for transmission
 */
void nvm_event_log_send_via_usart (nvm_event_log_fifo_t *fifo, srl_context_t *serial_port);

#endif /* B9059D46_61C3_45A2_A688_7297F71FC356 */
