#ifndef E365347D_02F3_4DCB_A254_BF10A5E57B60
#define E365347D_02F3_4DCB_A254_BF10A5E57B60

// clang-format off
/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

#define NVM_EVENT_CREATE_ENUM_FOR_TARGETS(_name, _non_ptr_based_write_function, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity, _page_size, pointer_based_access)					\
	typedef enum nvm_event_target_ereas_t {							\
		NVM_EVENT_TARGET_AREA_##_name,								\
		NVM_EVENT_TARGET_AREA_COUNT									\
	}nvm_event_target_ereas_t;																\

/**
 * Macro used for creating work pointers to all event logger targets,
 * which is configured as directly mapped into i/o area
 *
 * @param _name preconfigured name of a event logger target
*/
#define NVM_EVENT_CREATE_POINTERS_FOR_TARGET(_name, _non_ptr_based_write_function, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity, _page_size, pointer_based_access) \
		event_log_t* nvm_event_oldest##_name;							\
		event_log_t* nvm_event_newest##_name;							\
		uint32_t nvm_event_counter_id_for_last_##_name					\

/**
 *	Macro performing pre-insertion pointer arithmetics for all directly i/o
 *	mapped areas. This usually involves checking if an end of a buffer area
 *	is reached, and incrementing or resetting pointers to oldest and newest
 *	entry
 *
 * @param _name preconfigured name of a event logger target
 * @param _area_start_addr the lowest address of an area for this target
 * @param _area_end_addr the highest address of an area for this target
 * @param _erase_fn a function (a pointer to) which performs flash memory erasure.
 * 					It should implement an interface FLASH_Status(*erase_fn)(uint32_t), where a parameter is
 * 					an address of flash memory page of size @link{_page_size} to be erased. 
 * 					If the target is configured to store in RAM memory, or anything else which 
 * 					does not require explicit erase, the function might be simply nop
 * @param _severity level of an event
 * @param _page_size
 */
#define NVM_EVENT_PUSH_POINTERS_ARITM(_name, _area_start_addr, _area_end_addr, _erase_fn, _severity, _page_size)	\
if (EVENT_LOG_GET_SEVERITY(event->severity_and_source) >= _severity )	{						\
		nvm_event_log_perform_pointer_arithmetics(&nvm_event_oldest##_name, &nvm_event_newest##_name, _area_start_addr, _area_end_addr, &nvm_event_counter_id_for_last_##_name, _erase_fn, _page_size);	\
}																								\

/**
 *	Macro performing flash/RAM operation itself
 *
 * @param _name preconfigured name of a event logger target
 * @param _event_to_insert pointer to an event to be inserted
 * @param _area_end_addr the highest address of an area for this target
 * @param _erase_fn a function (a pointer to) which performs flash memory erasure.
 * @param _enable_pgm_fn macro which should expand to function call or a piece of code to enable programming
 * @param _wait_for_pgm_fn macro which should expand to function call or a piece of code to wait for flash memory programming to finish
 * @param _disable_pgm_fn macro which should expand to function call or a piece of code to disable programming
 * @param _severity level of an event
 */
#define NVM_EVENT_PUSH_POINTERS_FLASH_OPER(_name, _event_to_insert, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity)	\
if (EVENT_LOG_GET_SEVERITY(event->severity_and_source) >= _severity )	{																		\
	/* Set event counter id, if it hasn't been set so far  */																					\
	if (event->event_counter_id == 0) {																											\
		event->event_counter_id = nvm_event_counter_id_for_last_##_name;																		\
	}																																			\
	/* programming 32 bits at once */																											\
	uint32_t * ptr_event_to_insert = (uint32_t*)_event_to_insert;																				\
	uint32_t * ptr_place_for_new_event = (uint32_t*)nvm_event_newest##_name;																	\
																																				\
	_enable_pgm_fn																																\
																																				\
	*((uint32_t*)(ptr_place_for_new_event)) = *ptr_event_to_insert;																				\
	_wait_for_pgm_fn																															\
																																				\
	*((uint32_t*)(ptr_place_for_new_event)+ 1) = *(ptr_event_to_insert + 1);																	\
	_wait_for_pgm_fn																															\
																																				\
	*((uint32_t*)(ptr_place_for_new_event)+ 2) = *(ptr_event_to_insert + 2);																	\
	_wait_for_pgm_fn																															\
																																				\
	*((uint32_t*)(ptr_place_for_new_event)+ 3) = *(ptr_event_to_insert + 3);																	\
	_wait_for_pgm_fn																															\
																																				\
	_disable_pgm_fn																																\
																																				\
}																																				\

/**
 * 	Macro to perform pointer arithmetics after an event is stored in target area. It
 * 	updates pointers to oldest and newest pointer
 */
#define NVM_EVENT_PUSH_POINTERS_ARITM_SEC(_name, _area_start_addr, _area_end_addr, _page_size)														\
	/* rescan for oldest and newest event one more time */																		\
	nvm_event_log_find_first_oldest_newest(&nvm_event_oldest##_name, &nvm_event_newest##_name, (void*)_area_start_addr, (void*)_area_end_addr, _page_size);						\
																																\

/**
 *
 */
#define NVM_EVENT_EXPAND_POINTER_BASE_ACCESS_true(_name, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity, _page_size)		\
	NVM_EVENT_PUSH_POINTERS_ARITM(_name, _area_start_addr, _area_end_addr, _erase_fn, _severity, _page_size);																			\
	NVM_EVENT_PUSH_POINTERS_FLASH_OPER(_name, event, _area_end_addr,  _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity);							\
	NVM_EVENT_PUSH_POINTERS_ARITM_SEC(_name, _area_start_addr, _area_end_addr, _page_size);																							\

/**
 * 	Main macro used in @link{nvm_event_log_push_new_event} to expand all event i/o operations for all target configured
 *
 * @param _name preconfigured name of a event logger target
 * @param _non_ptr_based_write_function pointer used if an event target is configured as NOT memory i/o mapped, like serial port or a file
 * @param _area_start_addr the lowest address of an area for this target
 * @param _area_end_addr the highest address of an area for this target
 * @param _erase_fn a function (a pointer to) which performs flash memory erasure.
 * @param _enable_pgm_fn macro which should expand to function call or a piece of code to enable programming
 * @param _wait_for_pgm_fn macro which should expand to function call or a piece of code to wait for flash memory programming to finish
 * @param _disable_pgm_fn macro which should expand to function call or a piece of code to disable programming
 * @param _severity level of an event
 * @param _page_size
 * @param pointer_based_access set to true if this event logger target is memory mapped i/o
 */
#define NVM_EVENT_EXPAND_POINTER_BASE_ACCESS(_name, _non_ptr_based_write_function, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity, _page_size, pointer_based_access)	\
	NVM_EVENT_EXPAND_POINTER_BASE_ACCESS_##pointer_based_access(_name, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity, _page_size);				\

/**
 *
 */
#define NVM_EVENT_PERFORM_INIT_true(_name, _area_start_addr, _area_end_addr)			\
		NVM_EVENT_PUSH_POINTERS_ARITM_SEC(_name, _area_start_addr, _area_end_addr)		\

/**
 *
 */
#define NVM_EVENT_PERFORM_INIT(_name, _area_start_addr, _area_end_addr, pointer_based_access)	\
		NVM_EVENT_PERFORM_INIT_##pointer_based_access(_name, _area_start_addr, _area_end_addr)	\

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================
#define NVM_EVENT_GET_SEVERITY(x)	(x->severity_and_source & 0xF0) >> 4

#define NVM_EVENT_GET_SOURCE(x)		(x->severity_and_source & 0xF)

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#ifdef STM32L471xx

//!< Size of single flash memory page
#define NVM_PAGE_SIZE				2048

//!< How flash program operation are aligned, how many bytes must be programmed at once
#define NVM_WRITE_BYTE_ALIGN		8

#endif

#ifdef STM32F10X_MD_VL
/*
 * NVM logger currently not implemented for this platform
 */
#endif


#if (defined UNIT_TEST)
//!< Size of single flash memory page
#define NVM_PAGE_SIZE				2048

//!< How flash program operation are aligned, how many bytes must be programmed at once
#define NVM_WRITE_BYTE_ALIGN		8


/// ==================================================================================================
///	GLOBAL TYPES
/// ==================================================================================================

// currently defined here for unit tests
typedef enum
{
  FLASH_BUSY = 1, /**< FLASH_BUSY */
  FLASH_ERROR_PG, /**< FLASH_ERROR_PG */
  FLASH_ERROR_WRP,/**< FLASH_ERROR_WRP */
  FLASH_COMPLETE, /**< FLASH_COMPLETE */
  FLASH_TIMEOUT   /**< FLASH_TIMEOUT */
}FLASH_Status;
#endif

#endif /* E365347D_02F3_4DCB_A254_BF10A5E57B60 */
