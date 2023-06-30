/*
 * umb_pool.h
 *
 *  Created on: 21.03.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_MASTER_UMB_CHANNEL_POOL_H_
#define INCLUDE_UMB_MASTER_UMB_CHANNEL_POOL_H_

#include <stored_configuration_nvm/config_data.h>
#include "umb_context_t.h"
#include "umb_frame_t.h"


void umb_channel_pool(umb_frame_t *frame, umb_context_t *ctx, const config_data_umb_t * const config_umb);

#endif /* INCLUDE_UMB_MASTER_UMB_CHANNEL_POOL_H_ */
