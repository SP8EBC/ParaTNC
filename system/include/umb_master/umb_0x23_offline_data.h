/*
 * umb_0x23_offline_data.h
 *
 *  Created on: 20.03.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_MASTER_UMB_0X23_OFFLINE_DATA_H_
#define INCLUDE_UMB_MASTER_UMB_0X23_OFFLINE_DATA_H_

#include <stored_configuration_nvm/config_data.h>
#include "./umb_master/umb_retval_t.h"
#include "./umb_master/umb_frame_t.h"
#include "./umb_master/umb_context_t.h"


umb_retval_t umb_0x23_offline_data_request(umb_frame_t* frame, umb_context_t* ctx, uint16_t channel_number,  const config_data_umb_t * const config_umb);
umb_retval_t umb_0x23_offline_data_callback(umb_frame_t* frame, umb_context_t* ctx);

#endif /* INCLUDE_UMB_MASTER_UMB_0X23_OFFLINE_DATA_H_ */
