/*
 * variant.h
 *
 *  Created on: Oct 24, 2023
 *      Author: mateusz
 */

#ifndef VARIANT_H_
#define VARIANT_H_

#include "stdint.h"

int variant_validate_is_within_ram(void * address);
int variant_validate_is_within_flash(void * address);


#endif /* VARIANT_H_ */
