#ifndef A829659A_5A6C_450A_B678_DF44F9CAFB9B
#define A829659A_5A6C_450A_B678_DF44F9CAFB9B

#define WAIT_FOR_PGM_COMPLETION			\
	while (1) {\
		flash_status = FLASH_GetBank1Status();				\
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
