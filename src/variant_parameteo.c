/*
 * variant.c
 *
 *  Created on: Oct 24, 2023
 *      Author: mateusz
 */

#include "variant.h"

int variant_validate_is_within_ram(uint32_t address) {
	if (address != 0x00) {
		return 1;
	}
	else {
		return 0;
	}
}
