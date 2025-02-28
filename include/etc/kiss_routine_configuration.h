#ifndef CDEE69E2_4B5D_43BD_9E95_2C41507617E4
#define CDEE69E2_4B5D_43BD_9E95_2C41507617E4

#if defined(PARAMETEO)

#define KISS_ROUTINES(ENTRY)                                                                                \
    ENTRY(0x5254, routine_5254_start, 0x0, routine_5254_get_result, KISS_ROUTINE_CONTROL_SYNCHRO)           \
    ENTRY(0x6001, routine_6001_start, 0x0, routine_6001_get_result, KISS_ROUTINE_CONTROL_SYNCHRO)			\

#endif

#endif /* CDEE69E2_4B5D_43BD_9E95_2C41507617E4 */
