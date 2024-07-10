#ifndef E365347D_02F3_4DCB_A254_BF10A5E57B60
#define E365347D_02F3_4DCB_A254_BF10A5E57B60

/**
 * 
*/
#define NVM_EVENT_CREATE_POINTERS_FOR_TARGET(_name, _non_ptr_based_write_function, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity, pointer_based_access) \
		event_log_t* nvm_event_oldest##_name;							\
		event_log_t* nvm_event_newest##_name;							\
																		\

/**
 *
 */
#define NVM_EVENT_PUSH_POINTERS_ARITM(_name, _area_start_addr, _area_end_addr, _erase_fn, _severity)	\
if (EVENT_LOG_GET_SEVERITY(event->severity_and_source) >= _severity )	{						\
		nvm_event_log_perform_pointer_arithmetics(&nvm_event_oldest##_name, &nvm_event_newest##_name, _area_start_addr, _area_end_addr, _erase_fn);	\
}																								\

/**
 *
 */
#define NVM_EVENT_PUSH_POINTERS_FLASH_OPER(_name, _event_to_insert, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity)	\
if (EVENT_LOG_GET_SEVERITY(event->severity_and_source) >= _severity )	{																		\
	/* programming 32 bits at once */																											\
	uint32_t * ptr_event_to_insert = (uint32_t*)_event_to_insert;																				\
	uint32_t * ptr_place_for_new_event = (uint32_t*)nvm_event_newest##_name;																					\
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
 *
 */
#define NVM_EVENT_PUSH_POINTERS_ARITM_SEC(_name, _area_start_addr, _area_end_addr)														\
	/* rescan for oldest and newest event one more time */																		\
	nvm_event_log_find_first_oldest_newest(&nvm_event_oldest##_name, &nvm_event_newest##_name, (void*)_area_start_addr, (void*)_area_end_addr);						\
																																\

/**
 *
 */
#define NVM_EVENT_EXPAND_POINTER_BASE_ACCESS_true(_name, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity)		\
	NVM_EVENT_PUSH_POINTERS_ARITM(_name, _area_start_addr, _area_end_addr, _erase_fn, _severity);																			\
	NVM_EVENT_PUSH_POINTERS_FLASH_OPER(_name, event, _area_end_addr,  _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity);							\
	NVM_EVENT_PUSH_POINTERS_ARITM_SEC(_name, _area_start_addr, _area_end_addr);																							\

/**
 *
 */
#define NVM_EVENT_EXPAND_POINTER_BASE_ACCESS(_name, _non_ptr_based_write_function, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity, pointer_based_access)	\
	NVM_EVENT_EXPAND_POINTER_BASE_ACCESS_##pointer_based_access(_name, _area_start_addr, _area_end_addr, _erase_fn, _enable_pgm_fn, _wait_for_pgm_fn, _disable_pgm_fn, _severity);				\

#define NVM_EVENT_PERFORM_INIT_true(_name, _area_start_addr, _area_end_addr)			\
		NVM_EVENT_PUSH_POINTERS_ARITM_SEC(_name, _area_start_addr, _area_end_addr)		\

#define NVM_EVENT_PERFORM_INIT(_name, _area_start_addr, _area_end_addr, pointer_based_access)	\
		NVM_EVENT_PERFORM_INIT_##pointer_based_access(_name, _area_start_addr, _area_end_addr)	\

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
