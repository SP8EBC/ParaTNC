/*
 * pwr_save.h
 *
 *  Created on: Aug 22, 2021
 *      Author: mateusz
 */

#ifndef PWR_SAVE_H_
#define PWR_SAVE_H_

#include "station_config_target_hw.h"

#if defined(STM32L471xx)

void pwr_save_init(void);
void pwr_save_enter_stop2(void);

#endif


#endif /* PWR_SAVE_H_ */
