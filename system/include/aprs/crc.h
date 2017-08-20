#ifndef CRC_H_
#define CRC_H_

#include "stm32f10x_conf.h"

#include <macros.h>

extern const uint16_t crc_ccitt_tab[256];
#ifdef __cplusplus
extern "C"
{
#endif
/*********************************************************************************************************************/
static uint16_t updcrc_ccitt(uint8_t c, uint16_t oldcrc) {
/*********************************************************************************************************************/

	return (oldcrc >> 8) ^ pgm_read16(&crc_ccitt_tab[(oldcrc ^ c) & 0xff]);
}


/** CRC-CCITT init value */
#define CRC_CCITT_INIT_VAL ((uint16_t)0xFFFF)


/*********************************************************************************************************************/
extern uint16_t crc_ccitt(uint16_t crc, const void *buffer, uint16_t len);
/*********************************************************************************************************************/
#ifdef __cplusplus
}
#endif	


#endif /* CRC_H_ */
