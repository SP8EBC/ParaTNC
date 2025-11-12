/*
 * integers.h
 *
 *  Created on: Nov 12, 2025
 *      Author: mateusz
 */

#ifndef INCLUDE_INTEGERS_H_
#define INCLUDE_INTEGERS_H_

#include <stdint.h>

inline static uint32_t integers_dword_from_arr(uint8_t * arr, uint16_t offset)
{
    uint32_t out = 0;

    const void* const ptr = (arr + offset);

    out = *(uint32_t*) ptr;

    return out;
}


inline static uint16_t integers_word_from_arr(uint8_t * arr, uint16_t offset)
{
    uint16_t out = 0;

    const void* const ptr = (arr + offset);

    out = *(uint16_t*) ptr;

    return out;
}

#endif /* INCLUDE_INTEGERS_H_ */
