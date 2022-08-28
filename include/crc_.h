#ifndef CRC___H_
#define CRC___H_

#include "stdint.h"

uint8_t reflect8(uint8_t val);
uint32_t reflect32(uint32_t val);

uint32_t calcCRC32stm(void *data, uint32_t len,
   uint32_t poly, uint32_t seed, uint32_t initCRC, uint32_t inR, uint32_t outR);

uint32_t calcCRC32std(void *data, uint32_t len,
   uint32_t poly, uint32_t seed, uint32_t initCRC, uint32_t inR, uint32_t outR);

#endif
