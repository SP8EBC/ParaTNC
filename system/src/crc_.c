/*
 * crc_.c
 *
 *  Created on: Aug 28, 2022
 *      Author: mateusz
 */


#include "crc_.h"

uint8_t reflect8(uint8_t val)
{
    uint8_t resVal = 0;

    for(int i = 0; i < 8; i++)
    {
        if ((val & (1 << i)) != 0)
        {
            resVal |= (uint8_t )(1 << (7 - i));
        }
    }

    return resVal;
}

uint32_t reflect32(uint32_t val)
{
    uint32_t resVal = 0;

    for(int i = 0; i < 32; i++)
    {
        if ((val & (1 << i)) != 0)
        {
            resVal |= (uint32_t )(1 << (31 - i));
        }
    }

    return resVal;
}

uint32_t calcCRC32stm(void *data, uint32_t len,
   uint32_t poly, uint32_t seed, uint32_t initCRC, uint32_t inR, uint32_t outR)
{
    const uint8_t *buffer = (const uint8_t*) data;
    uint32_t crc = seed;
    uint8_t byte;

    while( len-- )
    {
        byte = *buffer++;
        if(inR) {
            byte = reflect8(byte);
        }
        crc = crc ^ (byte << 24);
        for( int bit = 0; bit < 8; bit++ )
        {
            if( crc & (1L << 31)) crc = (crc << 1) ^ poly;
            else                  crc = (crc << 1);
        }
    }
    if(outR) {
        crc = reflect32(crc);
    }
    if(initCRC == 1)    crc = ~crc;
    return crc;
}

uint32_t calcCRC32std(void *data, uint32_t len,
   uint32_t poly, uint32_t seed, uint32_t initCRC, uint32_t inR, uint32_t outR)
{
    uint32_t crc;
    uint8_t* current = (uint8_t*) data;
    uint8_t byte;

    crc = seed;

    while (len--) {
        byte = *current++;
        if(inR) {
            byte = reflect8(byte);
        }
        crc ^= byte;
        for (unsigned int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ poly;
            else
                crc = crc >> 1;
        }

    }
    if(outR) {
        crc = reflect32(crc);
    }
    if(initCRC == 1)    crc = ~crc;
    return crc;
}
