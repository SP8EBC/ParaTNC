/*
 * sds011.c
 *
 *  Created on: Sep 16, 2022
 *      Author: mateusz
 */

#include "dust_sensor/sds011.h"

int sds011_get_pms(uint8_t * data, uint16_t data_ln, uint16_t * pm_10, uint16_t * pm_2_5) {

	if (pm_10 == 0 || pm_2_5 == 0) {
		return -1;
	}

	uint16_t local_checksum = 0;

	// values received from sensor
	uint8_t head = 			*(data + 0);
	uint8_t command_id = 	*(data + 1);
	uint8_t checksum = 		*(data + 8);

	if (command_id == 0xC0) {
		// calculate checksum
		for (int i = 2; i < 8; i++) {
			local_checksum += *(data + i);
		}

		if ((local_checksum & 0xFF) != checksum) {
			return -2;
		}

		local_checksum = *(data + 2) | (*(data + 3) << 8);
		*pm_2_5 = local_checksum;

		local_checksum = *(data + 4) | (*(data + 5) << 8);
		*pm_10 = local_checksum;

		return 0;
	}
	else {
		return -3;
	}

}
