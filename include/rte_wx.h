/*
 * rte_wx.h
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include "drivers/_dht22.h"
#include "drivers/dallas.h"
#include "drivers/ms5611.h"
#include "../umb_master/umb_master.h"


#ifndef RTE_WX_H_
#define RTE_WX_H_

#define WIND_AVERAGE_LEN 18

#define PRESSURE_AVERAGE_LN 4

extern float rte_wx_temperature_dallas, rte_wx_temperature_dallas_valid;
extern float rte_wx_temperature_dalls_slew_rate;
extern float rte_wx_temperature_average_dallas_valid;
extern float rte_wx_temperature_min_dallas_valid, rte_wx_temperature_max_dallas_valid;
extern float rte_wx_temperature_ms, rte_wx_temperature_ms_valid;
extern float rte_wx_pressure, rte_wx_pressure_valid;
extern float rte_wx_pressure_history[PRESSURE_AVERAGE_LN];
extern uint8_t rte_wx_pressure_it;

extern uint16_t rte_wx_windspeed_pulses;
extern uint16_t rte_wx_windspeed[WIND_AVERAGE_LEN];
extern uint8_t rte_wx_windspeed_it;
extern uint16_t rte_wx_winddirection[WIND_AVERAGE_LEN];
extern uint8_t rte_wx_winddirection_it;
extern uint16_t rte_wx_winddirection_last;
extern uint16_t rte_wx_average_windspeed;
extern uint16_t rte_wx_max_windspeed;
extern int16_t rte_wx_average_winddirection;

extern uint8_t rte_wx_tx20_excessive_slew_rate;

extern dht22Values rte_wx_dht, rte_wx_dht_valid;

extern DallasQF rte_wx_current_dallas_qf, rte_wx_error_dallas_qf;
extern DallasAverage_t rte_wx_dallas_average;
extern ms5611_qf_t rte_wx_ms5611_qf;

#ifdef _UMB_MASTER

extern umb_frame_t rte_wx_umb;
extern umb_context_t rte_wx_umb_context;
extern uint8_t rte_wx_last_status;

#endif

#ifdef __cplusplus
extern "C"
{
#endif

void rte_wx_init(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_WX_H_ */
