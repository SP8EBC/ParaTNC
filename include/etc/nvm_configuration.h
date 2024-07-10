#ifndef A829659A_5A6C_450A_B678_DF44F9CAFB9B
#define A829659A_5A6C_450A_B678_DF44F9CAFB9B

#include "./drivers/l4/flash_stm32l4x.h"

/**
 * Configuration of all event logger targets
 *
 * @param TargetName 					preconfigured name of a event logger target
 * @param NonPtrBasedWriteFunction 		pointer used if an event target is configured as NOT memory i/o mapped, like serial port or a file
 * @param AreaStartAddress 				the lowest address of an area for this target
 * @param AreaEndAddress 				the highest address of an area for this target
 * @param EraseFunction 				a function (a pointer to) which performs flash memory erasure.
 * @param PgmingEnableFunction 			macro which should expand to function call or a piece of code to enable programming
 * @param WaitPgmCompleteFunction		macro which should expand to function call or a piece of code to wait for flash memory programming to finish
 * @param PgmingDisableFunction 		macro which should expand to function call or a piece of code to disable programming
 * @param MinimumSeverityLevel			level of an event
 * @param PointerBasedAccess			set to true if this event logger target is memory mapped i/o
 */
#define NVM_EVENT_LOGGING_TARGETS(ENTRY)														\
			/* TargetName,	NonPtrBasedWriteFunction,	AreaStartAddress,			AreaEndAddress,				EraseFunction,		PgmingEnableFunction,	WaitPgmCompleteFunction,	PgmingDisableFunction,	MinimumSeverityLevel,	PointerBasedAccess */	\
	ENTRY(	Flash,			NULL,						MEMORY_MAP_EVENT_LOG_START,	MEMORY_MAP_EVENT_LOG_END,	FLASH_ErasePage,	NVM_CONFIG_ENABLE_PGM,	WAIT_FOR_PGM_COMPLETION,	NVM_CONFIG_DISABLE_PGM,	1,						true)					\

/**
 * Macro for waiting for flash programming to finish
 */
#define WAIT_FOR_PGM_COMPLETION			\
	while (1) {\
		const FLASH_Status flash_status = FLASH_GetBank1Status();				\
															\
		if (flash_status == FLASH_BUSY) {					\
			;												\
		}													\
		else if (flash_status == FLASH_ERROR_PG) {			\
			nvm_general_state = NVM_PGM_ERROR;				\
			break;											\
		}													\
		else {												\
			break;											\
		}													\
	}														\

#define NVM_CONFIG_ENABLE_PGM                               \
	FLASH_Unlock();                                         \
	FLASH->CR |= FLASH_CR_PG;                               \


#define NVM_CONFIG_DISABLE_PGM                              \
	FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);                \
	FLASH_Lock();                                           

#endif /* A829659A_5A6C_450A_B678_DF44F9CAFB9B */
