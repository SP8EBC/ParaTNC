/*
 * pwr_switch.h
 *
 *  Created on: Aug 31, 2021
 *      Author: mateusz
 */

#ifndef PWR_SWITCH_H_
#define PWR_SWITCH_H_


typedef enum wx_pwr_state_t {
	WX_PWR_OFF,
	WX_PWR_ON,
	WX_PWR_UNDER_RESET,
	WX_PWR_DISABLED
}wx_pwr_state_t;

extern wx_pwr_state_t wx_pwr_state;

void wx_pwr_switch_init(void);
void wx_pwr_switch_periodic_handle(void);

#endif /* PWR_SWITCH_H_ */
