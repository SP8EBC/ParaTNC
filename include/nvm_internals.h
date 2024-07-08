#ifndef E365347D_02F3_4DCB_A254_BF10A5E57B60
#define E365347D_02F3_4DCB_A254_BF10A5E57B60

typedef enum nvm_state_result_t {
	NVM_UNINITIALIZED,
	NVM_OK,
	NVM_NO_SPACE_LEFT,
	NVM_INIT_ERROR,
	NVM_PGM_ERROR
}nvm_state_result_t;

typedef enum nvm_event_result_t {
	NVM_EVENT_OK,
	NVM_EVENT_OVERRUN_NO_TS,
	NVM_EVENT_OVERRUN,
	NVM_EVENT_SINGLE_TS,
	NVM_EVENT_EMPTY,
	NVM_EVENT_AREA_ERROR
}nvm_event_result_t;

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



#if (!defined STM32F10X_MD_VL) && (!defined STM32L471xx)

#endif

#if (defined UNIT_TEST)
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
