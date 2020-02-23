/*
 * umb_frame_t.h
 *
 *  Created on: 22.02.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_MASTER_UMB_FRAME_T_H_
#define INCLUDE_UMB_MASTER_UMB_FRAME_T_H_

#define UMB_FRAME_MAX_PAYLOAD_LN 40

#include <stdint.h>

typedef struct umb_frame_t {

	uint8_t protocol_version;

	uint8_t slave_id;

	uint8_t slave_class;

	uint8_t lenght;

	uint8_t command_id;

	uint8_t payload[UMB_FRAME_MAX_PAYLOAD_LN];

	uint8_t calculated_checksum_lsb;

	uint8_t calculated_checksum_msb;

} umb_frame_t;

#endif /* INCLUDE_UMB_MASTER_UMB_FRAME_T_H_ */
