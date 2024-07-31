#ifndef D693DD34_021C_482B_A532_AB5B31D18C64
#define D693DD34_021C_482B_A532_AB5B31D18C64

typedef enum nvm_state_after_last_oper_t {
	NVM_UNINITIALIZED,		///!< Default state before @link{nvm_event_log_init} is called
	NVM_OK,					///!< Nvm event logger is initialized and last event has been stored successfully
	NVM_OK_AND_EMPTY,		///!< Nvm event logger has been initialized correctly and it was detected that no event is stored
	NVM_NO_SPACE_LEFT,
	NVM_INIT_ERROR,			///!< Used only by @link{nvm_measurement_init} where there size of available flash memory can't be obrained
	NVM_PGM_ERROR,			///!< Error during erasing flash memory page or programming flash memory with new event
	NVM_GENERAL_ERROR		///!< Event logger target area (FLASH or RAM) contains complete garbage, so it must be reinitialized
}nvm_state_after_last_oper_t;

typedef enum nvm_event_result_t {
	NVM_EVENT_OK,
	NVM_EVENT_OVERRUN,		///!<
	NVM_EVENT_SINGLE,
	NVM_EVENT_EMPTY,		///!< NVM event logger memory is in erased state
	NVM_EVENT_AREA_ERROR,	///!< NVM event area is screwed very badly and cannot be recovered at all it must be formatted and reinitialized from scratch
	NVM_EVENT_ERROR
}nvm_event_result_t;

typedef struct nvm_event_result_stats_t {
	uint16_t timesyncs;
	uint16_t zz_total;
	uint16_t bootups;
	uint16_t asserts;
	uint16_t errors;
	uint16_t warnings;
	uint16_t info_cyclic;
	uint16_t info;
}nvm_event_result_stats_t;

#endif /* D693DD34_021C_482B_A532_AB5B31D18C64 */
